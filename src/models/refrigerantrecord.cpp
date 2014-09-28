/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2014 Matus & Michal Tomlein

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

#include "refrigerantrecord.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "partnerwidgets.h"
#include "global.h"

#include <QApplication>

using namespace Global;

RefrigerantRecord::RefrigerantRecord(const QString &date):
    DBRecord(tableName(), "date", date, MTDictionary())
{}

void RefrigerantRecord::initEditDialogue(EditDialogueWidgets *md)
{
    MTDictionary refrigerants(listRefrigerantsToString().split(';'));

    md->setWindowTitle(tr("Record of Refrigerant Management"));
    QVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    MDDateTimeEdit *date = new MDDateTimeEdit("date", tr("Date:"), md->widget(), attributes.value("date").toString());
    if (DBInfo::isDatabaseLocked()) {
        date->setMinimumDate(QDate::fromString(DBInfo::lockDate(), DATE_FORMAT));
    }
    md->addInputWidget(date);

    PartnerWidgets *partner_widgets = new PartnerWidgets(attributes.value("partner").toString(), attributes.value("partner_id").toString(), md->widget());
    md->addInputWidget(partner_widgets->partnersWidget());
    md->addInputWidget(partner_widgets->partnerNameWidget());
    md->addInputWidget(partner_widgets->partnerIdWidget());
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md->widget(), attributes.value("refrigerant").toString(), refrigerants));
    md->addInputWidget(new MDDoubleSpinBox("purchased", tr("Purchased (new):"), md->widget(), 0.0, 999999999.9, attributes.value("purchased").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("purchased_reco", tr("Purchased (recovered):"), md->widget(), 0.0, 999999999.9, attributes.value("purchased_reco").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("sold", tr("Sold (new):"), md->widget(), 0.0, 999999999.9, attributes.value("sold").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("sold_reco", tr("Sold (recovered):"), md->widget(), 0.0, 999999999.9, attributes.value("sold_reco").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_rege", tr("Reclaimed:"), md->widget(), 0.0, 999999999.9, attributes.value("refr_rege").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_disp", tr("Disposed of:"), md->widget(), 0.0, 999999999.9, attributes.value("refr_disp").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("leaked", tr("Leaked (new):"), md->widget(), 0.0, 999999999.9, attributes.value("leaked").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("leaked_reco", tr("Leaked (recovered):"), md->widget(), 0.0, 999999999.9, attributes.value("leaked_reco").toDouble(), QApplication::translate("Units", "kg")));
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT date FROM refrigerant_management" + QString(id().isEmpty() ? "" : " WHERE date <> :date"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":date", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

QString RefrigerantRecord::tableName()
{
    return "refrigerant_management";
}

class RefrigerantManagementColumns
{
public:
    RefrigerantManagementColumns() {
        columns << Column("date", "TEXT");
        columns << Column("partner", "TEXT");
        columns << Column("partner_id", "INTEGER");
        columns << Column("refrigerant", "TEXT");
        columns << Column("purchased", "NUMERIC");
        columns << Column("purchased_reco", "NUMERIC");
        columns << Column("sold", "NUMERIC");
        columns << Column("sold_reco", "NUMERIC");
        columns << Column("refr_rege", "NUMERIC");
        columns << Column("refr_disp", "NUMERIC");
        columns << Column("leaked", "NUMERIC");
        columns << Column("leaked_reco", "NUMERIC");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &RefrigerantRecord::columns()
{
    static RefrigerantManagementColumns columns;
    return columns.columns;
}

class RefrigerantRecordAttributes
{
public:
    RefrigerantRecordAttributes() {
        dict.insert("date", QApplication::translate("RefrigerantRecord", "Date"));
        dict.insert("partner", QApplication::translate("RefrigerantRecord", "Business partner"));
        dict.insert("partner_id", QApplication::translate("RefrigerantRecord", "Business partner (ID)"));
        dict.insert("refrigerant", QApplication::translate("RefrigerantRecord", "Refrigerant"));
        dict.insert("purchased", QApplication::translate("RefrigerantRecord", "Purchased (new)"));
        dict.insert("purchased_reco", QApplication::translate("RefrigerantRecord", "Purchased (recovered)"));
        dict.insert("sold", QApplication::translate("RefrigerantRecord", "Sold (new)"));
        dict.insert("sold_reco", QApplication::translate("RefrigerantRecord", "Sold (recovered)"));
        dict.insert("refr_rege", QApplication::translate("RefrigerantRecord", "Reclaimed"));
        dict.insert("refr_disp", QApplication::translate("RefrigerantRecord", "Disposed of"));
        dict.insert("leaked", QApplication::translate("RefrigerantRecord", "Leaked (new)"));
        dict.insert("leaked_reco", QApplication::translate("RefrigerantRecord", "Leaked (recovered)"));
    }

    MTDictionary dict;
};

const MTDictionary &RefrigerantRecord::attributes()
{
    static RefrigerantRecordAttributes dict;
    return dict.dict;
}
