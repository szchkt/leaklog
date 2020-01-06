/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2020 Matus & Michal Tomlein

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

#include "partnerwidgets.h"
#include "inputwidgets.h"
#include "mtsqlquery.h"
#include "global.h"

#include <QString>
#include <QLineEdit>

PartnerWidgets::PartnerWidgets(const QString &partner_name_str, const QString &partner_id_str, QWidget *md)
{
    partner_name_le = new MDLineEdit("partner", tr("Business partner:"), md, partner_name_str);
    partner_id_le = new MDCompanyIDEdit("partner_id", tr("Business partner (ID):"), md, partner_id_str);
    partner_id_le->setNullValue(QVariant(QVariant::Int));

    MTDictionary partners_dict("", "");

    MTSqlQuery query("SELECT partner, partner_id FROM refrigerant_management WHERE partner IS NOT NULL GROUP BY partner, partner_id");
    while (query.next()) {
        partners_dict.insert(QString("%1_%2").arg(query.value(1).toString()).arg(query.value(0).toString()),
                             QString("%1 (%2)").arg(query.value(0).toString()).arg(Global::formatCompanyID(query.value(1))));
    }

    partners_cb = new MDComboBox("partners", QObject::tr("Partners:"), md, "", partners_dict);
    partners_cb->setSkipSave(true);
    partners_cb->setMaximumWidth(300);
    QObject::connect(partners_cb, SIGNAL(currentIndexChanged(int)), this, SLOT(partnerChanged(int)));
}

void PartnerWidgets::partnerChanged(int)
{
    QStringList split = partners_cb->variantValue().toString().split("_");
    partner_id_le->setText(Global::formatCompanyID(split.takeFirst()));
    partner_name_le->setText(split.join("_"));
}
