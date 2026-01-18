/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2026 Matus & Michal Tomlein

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

#include "editcircuitdialoguecompressorstab.h"
#include "editdialoguetable.h"
#include "records.h"
#include "global.h"

#include <QDateTime>

EditCircuitDialogueCompressorsTab::EditCircuitDialogueCompressorsTab(const QString &circuit_uuid, QWidget *parent)
    : EditDialogueTab(parent)
{
    setName(tr("Compressors"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(9, 9, 9, 9);
    layout->setSpacing(9);

    QList<EditDialogueTableCell *> cells;
    EditDialogueTableCell *cell = new EditDialogueTableCell(tr("Name"), Global::String);
    cell->setId("name");
    cells << cell;
    cell = new EditDialogueTableCell(tr("Manufacturer"), Global::String);
    cell->setId("manufacturer");
    cells << cell;
    cell = new EditDialogueTableCell(tr("Type"), Global::String);
    cell->setId("type");
    cells << cell;
    cell = new EditDialogueTableCell(tr("Serial number"), Global::String);
    cell->setId("sn");
    cells << cell;
    compressors_table = new EditDialogueBasicTable(tr("Compressors"), cells, this);
    layout->addWidget(compressors_table);

    load(circuit_uuid);
}

void EditCircuitDialogueCompressorsTab::save(const QString &circuit_uuid)
{
    QList<QVariantMap> all_values = compressors_table->allValues();

    Compressor compressor;
    for (int i = 0; i < all_values.count(); ++i) {
        if (all_values.at(i).contains("uuid")) {
            QString uuid = all_values.at(i).value("uuid").toString();
            former_ids.removeAll(uuid);
            compressor = Compressor(uuid);
        } else {
            compressor = Compressor();
            compressor.setCircuitUUID(circuit_uuid);
        }

        compressor.setName(all_values.at(i).value("name").toString());
        compressor.setManufacturer(all_values.at(i).value("manufacturer").toString());
        compressor.setType(all_values.at(i).value("type").toString());
        compressor.setSerialNumber(all_values.at(i).value("sn").toString());
        compressor.save();
    }

    for (int i = 0; i < former_ids.count(); ++i)
        Compressor(former_ids.at(i)).remove();
}

void EditCircuitDialogueCompressorsTab::load(const QString &circuit_uuid)
{
    EditDialogueTableCell *cell;
    QMap<QString, EditDialogueTableCell *> compressors_data;
    if (!circuit_uuid.isEmpty() && Circuit(circuit_uuid).exists()) {
        ListOfVariantMaps compressors = Compressor::query({{"circuit_uuid", circuit_uuid}}).listAll();
        for (int i = 0; i < compressors.count(); ++i) {
            former_ids << compressors.at(i).value("uuid").toString();

            cell = new EditDialogueTableCell(compressors.at(i).value("name"), Global::String);
            cell->setId("name");
            compressors_data.insert("name", cell);
            cell = new EditDialogueTableCell(compressors.at(i).value("manufacturer"), Global::String);
            cell->setId("manufacturer");
            compressors_data.insert("manufacturer", cell);
            cell = new EditDialogueTableCell(compressors.at(i).value("type"), Global::String);
            cell->setId("type");
            compressors_data.insert("type", cell);
            cell = new EditDialogueTableCell(compressors.at(i).value("sn"), Global::String);
            cell->setId("sn");
            compressors_data.insert("sn", cell);
            cell = new EditDialogueTableCell(compressors.at(i).value("uuid"), "uuid");
            compressors_data.insert("uuid", cell);
            compressors_table->addRow(compressors_data);
        }
    } else if (!compressors_table->rowsCount()) {
        cell = new EditDialogueTableCell(tr("Compressor 1"), "name", Global::String);
        compressors_data.insert("name", cell);
        cell = new EditDialogueTableCell("", "manufacturer", Global::String);
        compressors_data.insert("manufacturer", cell);
        cell = new EditDialogueTableCell("", "type", Global::String);
        compressors_data.insert("type", cell);
        cell = new EditDialogueTableCell("", "sn", Global::String);
        compressors_data.insert("sn", cell);
        compressors_table->addRow(compressors_data);
    }
}
