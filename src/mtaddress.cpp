/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2017 Matus & Michal Tomlein

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

#include "mtaddress.h"

// Qt::escape()
#include <QTextDocument>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>

void MTAddress::clear()
{
    a_street.clear();
    a_city.clear();
    a_postal_code.clear();
}

QString MTAddress::toString() const
{
    QString address;
    address.append("Street: " + a_street + "\n");
    address.append("City: " + a_city + "\n");
    address.append("Postal code: " + a_postal_code);
    return address;
}

QString MTAddress::toPlainText(MTAddress::AddressFormat format) const
{
    if (a_street.isEmpty() && a_city.isEmpty() && a_postal_code.isEmpty())
        return QString();
    return MTAddressEdit::addressStringFormat(format).arg(a_street).arg(a_city).arg(a_postal_code);
}

QString MTAddress::toHtml(MTAddress::AddressFormat format) const
{
    if (a_street.isEmpty() && a_city.isEmpty() && a_postal_code.isEmpty())
        return QString();
    return MTAddressEdit::addressStringFormat(format)
            .replace("\n", "<br>")
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
            .arg(Qt::escape(a_street))
            .arg(Qt::escape(a_city))
            .arg(Qt::escape(a_postal_code));
#else
            .arg(a_street.toHtmlEscaped())
            .arg(a_city.toHtmlEscaped())
            .arg(a_postal_code.toHtmlEscaped());
#endif
}

void MTAddress::initWithAddress(const QString &address) {
    clear();
    QStringList list = address.split('\n', QString::SkipEmptyParts);
    for (QStringList::const_iterator i = list.constBegin(); i != list.constEnd(); ++i) {
        if (i->startsWith("Street:", Qt::CaseInsensitive)) { setStreet(i->mid(7)); }
        else if (i->startsWith("City:", Qt::CaseInsensitive)) { setCity(i->mid(5)); }
        else if (i->startsWith("Postal code:", Qt::CaseInsensitive)) { setPostalCode(i->mid(12)); }
    }
}

QLabel *newLabel(const QString &text, QWidget *parent = NULL)
{
    QLabel *lbl = new QLabel(text, parent);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    return lbl;
}

void MTAddressEdit::init()
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    QGridLayout *grid_layout = new QGridLayout(this);
    grid_layout->setSpacing(6);
    grid_layout->setContentsMargins(6, 6, 6, 6);
    grid_layout->addWidget(newLabel(tr("Street:"), this), 0, 0);
    grid_layout->addWidget(ae_street = new QLineEdit(this), 0, 1);
    ae_street->setStyleSheet("");
    grid_layout->addWidget(newLabel(tr("City:"), this), 1, 0);
    grid_layout->addWidget(ae_city = new QLineEdit(this), 1, 1);
    grid_layout->addWidget(newLabel(tr("Postal code:"), this), 2, 0);
    grid_layout->addWidget(ae_postal_code = new QLineEdit(this), 2, 1);
}

void MTAddressEdit::setAddress(const MTAddress &address)
{
    ae_street->setText(address.street());
    ae_city->setText(address.city());
    ae_postal_code->setText(address.postalCode());
}

MTAddress MTAddressEdit::address() const
{
    MTAddress address;
    address.setStreet(ae_street->text());
    address.setCity(ae_city->text());
    address.setPostalCode(ae_postal_code->text());
    return address;
}

QString MTAddressEdit::addressStringFormat(MTAddress::AddressFormat format)
{
    switch (format) {
        case MTAddress::Singleline:
            //: Singleline address format: %1 = street, %2 = city, %3 = postal code
            return tr("%1, %2 %3");
        default: break;
    }
    //: Multiline address format: %1 = street, %2 = city, %3 = postal code
    return tr("%1\n%2 %3");
}
