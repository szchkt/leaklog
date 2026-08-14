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

#include "partnerwidgets.h"
#include "businesspartner.h"
#include "editdialoguewidgets.h"
#include "inputwidgets.h"

#include <QApplication>
#include <QString>
#include <QLineEdit>

PartnerWidgets::PartnerWidgets(const QString &partner_uuid, const QString &partner_name, const QString &partner_id, QWidget *md)
    : MDGroupedInputWidgets(tr("Business partner:"), md)
{
    setRowSpan(9);

    partner_name_le = new MDLineEdit("partner", QApplication::translate("BusinessPartner", "Name:"), this, partner_name);
    partner_id_le = new MDCompanyIDEdit("partner_id", QApplication::translate("BusinessPartner", "ID:"), this, partner_id);
    partner_id_le->setNullValue(QVariant(QVariant::Int));
    company_vatin_le = new MDLineEdit("company_vatin", QApplication::translate("BusinessPartner", "VAT ID:"), this, QString());
    company_vatin_le->setSkipSave(true);
    address_ae = new MDAddressEdit("address", QApplication::translate("BusinessPartner", "Address:"), this, QString());
    address_ae->setSkipSave(true);
    phone_le = new MDLineEdit("phone", QApplication::translate("BusinessPartner", "Phone:"), this, QString());
    phone_le->setSkipSave(true);
    mail_le = new MDLineEdit("mail", QApplication::translate("BusinessPartner", "E-mail:"), this, QString());
    mail_le->setSkipSave(true);
    website_le = new MDLineEdit("website", QApplication::translate("BusinessPartner", "Website:"), this, QString());
    website_le->setSkipSave(true);
    notes_pte = new MDPlainTextEdit("notes", QApplication::translate("BusinessPartner", "Notes:"), this, QString());
    notes_pte->setSkipSave(true);

    MTDictionary partners_dict(Global::createUUID(), tr("New Partner"));

    BusinessPartner::query().each("name", [&partners_dict](BusinessPartner &partner) {
        partners_dict.insert(partner.uuid(), QString("%1 (%2)").arg(partner.name()).arg(partner.companyID()));
    });

    partners_cb = new MDComboBox("partner_uuid", QObject::tr("Partners:"), this, partner_uuid.isEmpty() ? partners_dict.key(0) : partner_uuid, partners_dict);
    partners_cb->setMaximumWidth(300);
    QObject::connect(partners_cb, SIGNAL(currentIndexChanged(int)), this, SLOT(partnerChanged(int)));

    input_widgets << partners_cb << partner_name_le << partner_id_le << company_vatin_le << address_ae << phone_le << mail_le << website_le << notes_pte;
    foreach (MDAbstractInputWidget *widget, input_widgets) {
        widget->setRowSpan(0);
        addWidget(widget);
    }

    if (!partner_uuid.isEmpty())
        partnerChanged(partners_cb->currentIndex());
}

void PartnerWidgets::addToEditDialogue(EditDialogueWidgets &md)
{
    md.addInputWidget(this);

    foreach (MDAbstractInputWidget *widget, input_widgets)
        md.addInputWidget(widget);
}

void PartnerWidgets::save()
{
    BusinessPartner partner(partners_cb->variantValue().toString());

    for (QList<MDAbstractInputWidget *>::const_iterator i = input_widgets.constBegin(); i != input_widgets.constEnd(); ++i) {
        if (*i == partners_cb)
            continue;
        QString id = (*i)->id();
        if (*i == partner_name_le) {
            id = "name";
        } else if (*i == partner_id_le) {
            id = "company_id";
        }
        QVariant value = (*i)->variantValue();
        partner.setValue(id, value);
    }

    partner.save();
}

void PartnerWidgets::partnerChanged(int)
{
    BusinessPartner partner(partners_cb->variantValue().toString());
    partner_name_le->setText(partner.name());
    partner_id_le->setText(partner.companyID());
    company_vatin_le->setText(partner.companyVATIN());
    address_ae->setVariantValue(partner.address());
    phone_le->setText(partner.phone());
    mail_le->setText(partner.mail());
    website_le->setText(partner.website());
    notes_pte->setVariantValue(partner.notes());
}
