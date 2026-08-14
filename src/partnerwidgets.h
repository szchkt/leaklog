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

#ifndef PARTNER_WIDGETS_H
#define PARTNER_WIDGETS_H

#include "inputwidgets.h"

class EditDialogueWidgets;
class MDLineEdit;
class MDComboBox;
class MDAddressEdit;
class MDPlainTextEdit;
class QWidget;
class QString;

class PartnerWidgets : public MDGroupedInputWidgets
{
    Q_OBJECT

public:
    PartnerWidgets(const QString &partner_uuid, const QString &partner_name, const QString &partner_id, QWidget *md);

    void addToEditDialogue(EditDialogueWidgets &);
    void save();

private slots:
    void partnerChanged(int);

private:
    QList<MDAbstractInputWidget *> input_widgets;
    MDLineEdit *partner_id_le;
    MDLineEdit *partner_name_le;
    MDLineEdit *company_vatin_le;
    MDAddressEdit *address_ae;
    MDLineEdit *phone_le;
    MDLineEdit *mail_le;
    MDLineEdit *website_le;
    MDPlainTextEdit *notes_pte;
    MDComboBox *partners_cb;
};

#endif // PARTNER_WIDGETS_H
