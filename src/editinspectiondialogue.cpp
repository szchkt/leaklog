/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2016 Matus & Michal Tomlein

 Leaklog is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence
 as published by the Free Software Foundation; either version 2
 of the Licence, or (at your option) any later version.

 Leaklog is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with Leaklog; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
********************************************************************/

#include "editinspectiondialogue.h"

#include "global.h"
#include "records.h"
#include "inputwidgets.h"
#include "editdialoguetable.h"
#include "editinspectiondialoguecompressors.h"
#include "editinspectiondialogueassemblyrecordtab.h"
#include "editinspectiondialoguelayout.h"
#include "editinspectiondialogueaccess.h"
#include "variables.h"

#include <QMessageBox>
#include <QSplitter>
#include <QSettings>

EditInspectionDialogue::EditInspectionDialogue(Inspection *record, UndoStack *undo_stack, QWidget *parent, const QString &duplicate_from)
    : TabbedEditDialogue(record, undo_stack, parent, false),
      compressors(NULL)
{
    md_grid_main->setHorizontalSpacing(9);
    md_grid_main->setVerticalSpacing(6);
    md_grid_main->setContentsMargins(0, 0, 0, 0);

    main_tabw->setTabText(0, tr("Inspection"));

    splitter = new QSplitter(Qt::Vertical, this);
    splitter->setContentsMargins(0, 0, 0, 0);
    md_grid_main->addWidget(splitter, 0, 0);

    QWidget *widget_trees = new QWidget(this);
    splitter->addWidget(widget_trees);

    QGridLayout *grid_trees = new QGridLayout(widget_trees);
    grid_trees->setContentsMargins(6, 6, 6, 6);
    EditInspectionDialogueLayout(&md_inputwidgets, &md_groups, grid_trees).layout();

    QWidget *widget_rmds = new QWidget(this);
    splitter->addWidget(widget_rmds);

    QGridLayout *gl_rmds = new QGridLayout(widget_rmds);
    gl_rmds->setContentsMargins(9, 9, 9, 9);

    MDAbstractInputWidget *risks = inputWidget("risks");
    gl_rmds->addWidget(risks->label()->widget(), 0, 0);
    gl_rmds->addWidget(risks->widget(), 0, 1);
    gl_rmds->setRowStretch(0, 1);

    MDAbstractInputWidget *rmds = inputWidget("rmds");
    gl_rmds->addWidget(rmds->label()->widget(), 1, 0);
    gl_rmds->addWidget(rmds->widget(), 1, 1);
    gl_rmds->setRowStretch(1, 2);

    if (!(((Inspection *) record)->scope() & Variable::Compressor)) {
        QString id = duplicate_from.isEmpty() ? md_record->id() : duplicate_from;
        compressors = new EditInspectionDialogueCompressors(record->customerUUID(), record->circuitUUID(), id, this);
        compressors->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        splitter->addWidget(compressors);
        tabs.append(compressors);
    }

    addTab(new EditInspectionDialogueAssemblyRecordTab(0, (MDLineEdit *) inputWidget("arno"),
                                                       (MDComboBox *) inputWidget("ar_type_uuid"),
                                                       new EditInspectionDialogueAccess(this),
                                                       record->customerUUID(),
                                                       record->circuitUUID()));
    addTab(new EditInspectionDialogueImagesTab(record->id()));

    splitter->setSizes(QList<int>() << 1000 << 1 << 200);

    QSettings settings("SZCHKT", "Leaklog");
    resize(settings.value("inspection_dialogue/size", QSize(900, 550)).toSize() * Global::scaleFactor());
    QString splitter_state_key = QString("inspection_dialogue/splitter_state%1").arg(compressors ? "" : "_no_compressors");
    splitter->restoreState(settings.value(splitter_state_key).toByteArray());
}

EditInspectionDialogue::~EditInspectionDialogue()
{
    QSettings settings("SZCHKT", "Leaklog");
    settings.setValue("inspection_dialogue/size", size() / Global::scaleFactor());
    QString splitter_state_key = QString("inspection_dialogue/splitter_state%1").arg(compressors ? "" : "_no_compressors");
    settings.setValue(splitter_state_key, splitter->saveState());
}

EditInspectionDialogueImagesTab::EditInspectionDialogueImagesTab(const QString &inspection_uuid)
{
    setName(tr("Images"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    QList<EditDialogueTableCell *> cells;
    EditDialogueTableCell *cell = new EditDialogueTableCell(tr("Description"), Global::Text);
    cell->setId("description");
    cells.append(cell);
    cell = new EditDialogueTableCell(tr("Image"), Global::File);
    cell->setId("file_uuid");
    cells.append(cell);
    table = new EditDialogueBasicTable(tr("Images"), cells, this);
    layout->addWidget(table);

    loadItemInputWidgets(inspection_uuid);
}

void EditInspectionDialogueImagesTab::loadItemInputWidgets(const QString &inspection_uuid)
{
    if (inspection_uuid.isEmpty()) {
        table->addNewRow();
        return;
    }

    auto images = Inspection(inspection_uuid).images().all();

    QMap<QString, EditDialogueTableCell *> image_data;
    EditDialogueTableCell *cell;

    foreach (auto image, images) {
        cell = new EditDialogueTableCell(image.id(), Global::File);
        cell->setId("uuid");
        image_data.insert("uuid", cell);
        cell = new EditDialogueTableCell(image.description(), Global::Text);
        cell->setId("description");
        image_data.insert("description", cell);
        cell = new EditDialogueTableCell(image.fileUUID(), Global::File);
        cell->setId("file_uuid");
        image_data.insert("file_uuid", cell);
        table->addRow(image_data);
    }

    if (!images.count()) table->addNewRow();
}

void EditInspectionDialogueImagesTab::save(const QString &inspection_uuid)
{
    QList<MTDictionary> dicts = table->allValues();
    auto images = Inspection(inspection_uuid).images().map("uuid");

    for (int i = 0; i < dicts.count(); ++i) {
        QString file_uuid = dicts.at(i).value("file_uuid");
        if (file_uuid.isEmpty())
            continue;

        InspectionImage image;
        QString uuid = dicts.at(i).value("uuid");

        if (!uuid.isEmpty() && images.contains(uuid)) {
            image = images.value(uuid);
            images.remove(uuid);
        }

        image.setInspectionUUID(inspection_uuid);
        image.setFileUUID(file_uuid);
        image.setDescription(dicts.at(i).value("description"));
        image.save();
    }

    QMapIterator<QVariant, InspectionImage> i(images);
    while (i.hasNext()) { i.next();
        i.value().remove();
    }
}
