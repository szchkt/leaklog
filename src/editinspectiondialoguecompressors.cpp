/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2015 Matus & Michal Tomlein

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

EditInspectionDialogueCompressors::EditInspectionDialogueCompressors(const QString &customer_id, const QString &circuit_id, const QString &inspection_date, QWidget *parent)
    : QWidget(parent),
      customer_id(customer_id),
      circuit_id(circuit_id),
      original_inspection_date(inspection_date)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    tab_w = new QTabWidget(this);
    layout->addWidget(tab_w);

    loadTabs(inspection_date);
}

void EditInspectionDialogueCompressors::loadTabs(const QString &inspection_date)
{
    Compressor compressors_rec(QString(), MTDictionary(QStringList() << "customer_id" << "circuit_id",
                                                       QStringList() << customer_id << circuit_id));
    ListOfVariantMaps compressors = compressors_rec.listAll();

    for (int i = 0; i < compressors.count(); ++i) {
        InspectionCompressorTab *tab = addTab(compressors.at(i).value("id").toInt(), compressors.at(i).value("name").toString());

        if (!inspection_date.isEmpty()) {
            InspectionsCompressor inspection_compressor_rec(QString(), MTDictionary(QStringList() << "customer_id" << "circuit_id" << "date" << "compressor_id",
                QStringList() << customer_id << circuit_id << inspection_date << compressors.at(i).value("id").toString()));
            QVariantMap inspection_compressor = inspection_compressor_rec.list();
            if (!inspection_compressor.isEmpty()) {
                tab->setRecordId(inspection_compressor.value("id").toInt());
                former_ids.append(inspection_compressor.value("id").toInt());
                tab->init(inspection_compressor);
            } else {
                tab->init();
            }
        } else {
            tab->init();
        }
    }
}

InspectionCompressorTab *EditInspectionDialogueCompressors::addTab(int tab_id, const QString &tab_name)
{
    InspectionCompressorTab *tab = new InspectionCompressorTab(tab_id, this);
    tabs.append(tab);

    tab_w->addTab(tab, tab_name);

    return tab;
}

void EditInspectionDialogueCompressors::save(const QVariant &inspection_date)
{
    for (int i = 0; i < tabs.count(); ++i) {
        tabs.at(i)->save(customer_id, circuit_id, original_inspection_date, inspection_date.toString());
        if (tabs.at(i)->recordId() >= 0)
            former_ids.removeAll(tabs.at(i)->recordId());
    }
    for (int i = 0; i < former_ids.count(); ++i)
        InspectionsCompressor(QString::number(former_ids.at(i))).remove();
}

InspectionCompressorTab::InspectionCompressorTab(int id, QWidget *parent)
    : QWidget(parent),
      EditDialogueWidgets(),
      m_id(id),
      m_record_id(-1)
{
}

void InspectionCompressorTab::init(const QVariantMap &var_values)
{
    Variables vars(QSqlDatabase(), Variable::Compressor);
    vars.initEditDialogueWidgets(this, var_values);

    QGridLayout *layout = new QGridLayout(this);

    EditInspectionDialogueLayout(&md_inputwidgets, &md_groups, layout).layout();
}

bool InspectionCompressorTab::save(const QString &customer_id, const QString &circuit_id,
                                   const QString &original_inspection_date, const QString &inspection_date)
{
    MTDictionary parents(QStringList() << "customer_id" << "circuit_id" << "date" << "compressor_id",
                         QStringList() << customer_id << circuit_id << original_inspection_date << QString::number(m_id));
    InspectionsCompressor record(m_record_id >= 0 ?
                                     QString::number(m_record_id)
                                   : QString(),
                                 parents);

    QVariantMap values;
    if (inspection_date != original_inspection_date)
        values.insert("date", inspection_date);
    for (QList<MDAbstractInputWidget *>::const_iterator i = md_inputwidgets.constBegin(); i != md_inputwidgets.constEnd(); ++i) {
        if ((*i)->skipSave())
            continue;
        values.insert((*i)->id(), (*i)->variantValue());
    }
    return record.update(values, true);
}
