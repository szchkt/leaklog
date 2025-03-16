/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2025 Matus & Michal Tomlein

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

#include "editinspectiondialoguecompressors.h"

#include "records.h"
#include "variables.h"
#include "editinspectiondialoguelayout.h"
#include "inputwidgets.h"

#include <QVBoxLayout>
#include <QTabWidget>
#include <QVariantMap>

EditInspectionDialogueCompressors::EditInspectionDialogueCompressors(const QString &customer_uuid, const QString &circuit_uuid, const QString &inspection_uuid, bool duplicate, QWidget *parent)
    : QWidget(parent),
      customer_uuid(customer_uuid),
      circuit_uuid(circuit_uuid)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    tab_w = new QTabWidget(this);
    layout->addWidget(tab_w);

    loadTabs(inspection_uuid, duplicate);
}

void EditInspectionDialogueCompressors::loadTabs(const QString &inspection_uuid, bool duplicate)
{
    ListOfVariantMaps compressors = Circuit(circuit_uuid).compressors().listAll();

    if (inspection_uuid.isEmpty()) {
        for (int i = 0; i < compressors.count(); ++i) {
            InspectionCompressor inspection_compressor;
            inspection_compressor.setValue("compressor_uuid", compressors.at(i).value("uuid").toString());

            addTab(inspection_compressor, compressors.at(i).value("name").toString());
        }
    } else {
        for (int i = 0; i < compressors.count(); ++i) {
            InspectionCompressor inspection_compressor = InspectionCompressor::query({
                {"inspection_uuid", inspection_uuid},
                {"compressor_uuid", compressors.at(i).value("uuid").toString()}
            }).first();

            if (duplicate) {
                inspection_compressor.duplicate();
                inspection_compressor.resetValue("id");
            }

            addTab(inspection_compressor, compressors.at(i).value("name").toString());
        }
    }
}

InspectionCompressorTab *EditInspectionDialogueCompressors::addTab(const InspectionCompressor &inspection_compressor, const QString &tab_name)
{
    InspectionCompressorTab *tab = new InspectionCompressorTab(inspection_compressor, this);
    tabs.append(tab);

    tab_w->addTab(tab, tab_name);

    return tab;
}

void EditInspectionDialogueCompressors::save(const QString &inspection_uuid)
{
    for (int i = 0; i < tabs.count(); ++i) {
        tabs.at(i)->save(inspection_uuid);
    }
}

InspectionCompressorTab::InspectionCompressorTab(const InspectionCompressor &inspection_compressor, QWidget *parent)
    : QWidget(parent),
      EditDialogueWidgets(),
      inspection_compressor(inspection_compressor)
{
    init(this->inspection_compressor.values());
}

void InspectionCompressorTab::init(const QVariantMap &var_values)
{
    Variables vars(QSqlDatabase(), Variable::Compressor);
    vars.initEditDialogueWidgets(this, var_values);

    QGridLayout *layout = new QGridLayout(this);

    EditInspectionDialogueLayout(&md_inputwidgets, &md_groups, layout).layout();
}

bool InspectionCompressorTab::save(const QString &inspection_uuid)
{
    for (QList<MDAbstractInputWidget *>::const_iterator i = md_inputwidgets.constBegin(); i != md_inputwidgets.constEnd(); ++i) {
        if ((*i)->skipSave())
            continue;
        inspection_compressor.setValue((*i)->id(), (*i)->variantValue());
    }
    inspection_compressor.setValue("inspection_uuid", inspection_uuid);
    return inspection_compressor.save(true);
}
