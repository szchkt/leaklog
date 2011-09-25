/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2011 Matus & Michal Tomlein

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

#include "edit_inspection_dialogue.h"

#include "global.h"
#include "records.h"
#include "input_widgets.h"
#include "edit_dialogue_table.h"
#include "edit_inspection_dialogue_compressors.h"
#include "edit_inspection_dialogue_assembly_record_tab.h"
#include "edit_dialogue_layout.h"
#include "variables.h"

#include <QMessageBox>

EditInspectionDialogue::EditInspectionDialogue(DBRecord * record, QWidget * parent)
    : TabbedEditDialogue(record, parent),
      compressors(NULL)
{
    main_tabw->setTabText(0, tr("Inspection"));

    MDAbstractInputWidget * rmds = inputWidget("rmds");
    md_grid_main->addWidget(rmds->label()->widget(), md_grid_main->rowCount(), 0);
    md_grid_main->addWidget(rmds->widget(), md_grid_main->rowCount() - 1, 1, 1, md_grid_main->columnCount() - 1);

    if (!(((Inspection *) record)->scope() & Variable::Compressor)) {
        compressors = new EditInspectionDialogueCompressors(md_record->parent("customer"),
                                                              md_record->parent("circuit"),
                                                              md_record->id(),
                                                              this);
        md_grid_main->addWidget(compressors, md_grid_main->rowCount(), 0, 1, md_grid_main->columnCount());
        tabs.append(compressors);
    }

    addTab(new EditInspectionDialogueAssemblyRecordTab(0, (MDLineEdit *) inputWidget("arno"), (MDComboBox *) inputWidget("ar_type"), md_record->parent("customer"), md_record->parent("circuit")));
    addTab(new EditInspectionDialogueImagesTab(md_record->parent("customer"), md_record->parent("circuit"), idFieldValue().toString()));
}

const QVariant EditInspectionDialogue::idFieldValue()
{
    MDAbstractInputWidget * iw = inputWidget("date");
    if (iw)
        return iw->variantValue();
    else
        return QVariant();
}

EditInspectionDialogueImagesTab::EditInspectionDialogueImagesTab(const QString & customer_id, const QString & circuit_id, const QString & inspection_id)
{
    this->customer_id = customer_id;
    this->circuit_id = circuit_id;

    setName(tr("Images"));

    init(inspection_id);
}

void EditInspectionDialogueImagesTab::init(const QString & inspection_id)
{
    original_inspection_id = inspection_id;

    QVBoxLayout * layout = new QVBoxLayout(this);

    QList<EditDialogueTableCell *> cells;
    EditDialogueTableCell * cell = new EditDialogueTableCell(tr("Description"), Global::Text);
    cell->setId("description");
    cells.append(cell);
    cell = new EditDialogueTableCell(tr("Image"), Global::File);
    cell->setId("file_id");
    cells.append(cell);
    table = new EditDialogueBasicTable(tr("Images"), cells, this);
    layout->addWidget(table);

    loadItemInputWidgets(inspection_id);
}

void EditInspectionDialogueImagesTab::loadItemInputWidgets(const QString & inspection_id)
{
    InspectionImage images_record(customer_id, circuit_id, inspection_id);
    ListOfVariantMaps images = images_record.listAll();

    QMap<QString, EditDialogueTableCell *> image_data;
    EditDialogueTableCell * cell;

    for (int i = 0; i < images.count(); ++i) {
        cell = new EditDialogueTableCell(images.at(i).value("description"), Global::Text);
        cell->setId("description");
        image_data.insert("description", cell);
        cell = new EditDialogueTableCell(images.at(i).value("file_id"), Global::File);
        cell->setId("file_id");
        image_data.insert("file_id", cell);
        table->addRow(image_data);
    }

    if (!images.count()) table->addNewRow();
}

void EditInspectionDialogueImagesTab::save(const QVariant & inspection_id)
{
    QVariantMap map;

    QList<MTDictionary> dicts = table->allValues();
    QList<int> undeleted_files;

    InspectionImage images_record(customer_id, circuit_id, original_inspection_id);

    ListOfVariantMaps images = images_record.listAll("file_id");

    for (int i = 0; i < images.count(); ++i) {
        int file_id = images.at(i).value("file_id").toInt();
        if (!undeleted_files.contains(file_id))
            undeleted_files.append(file_id);
    }

    images_record.remove();

    images_record = InspectionImage(customer_id, circuit_id, inspection_id.toString());

    for (int i = 0; i < dicts.count(); ++i) {
        int file_id = dicts.at(i).value("file_id").toInt();
        if (file_id <= 0)
            continue;
        undeleted_files.removeAll(file_id);

        map.insert("description", dicts.at(i).value("description"));
        map.insert("file_id", file_id);
        images_record.update(map);
    }

    for (int i = 0; i < undeleted_files.count(); ++i)
        DBFile(undeleted_files.at(i)).remove();
}
