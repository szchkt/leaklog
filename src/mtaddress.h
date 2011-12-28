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

#ifndef MTADDRESS_H
#define MTADDRESS_H

#include <QFrame>

class QStringList;
class QLabel;
class QLineEdit;

class MTAddress
{
public:
    enum AddressFormat { Singleline, Multiline };

    MTAddress() {}
    MTAddress(const QString & address) { initWithAddress(address); }

    void clear();
    inline void fromString(const QString & address) { initWithAddress(address); }
    QString toString() const;
    QString toPlainText(AddressFormat = Singleline) const;
    QString toHtml(AddressFormat = Singleline) const;

    inline void setStreet(const QString & street) { a_street = street.simplified(); }
    inline QString street() const { return a_street; }
    inline void setCity(const QString & city) { a_city = city.simplified(); }
    inline QString city() const { return a_city; }
    inline void setPostalCode(const QString & postal_code) { a_postal_code = postal_code.simplified(); }
    inline QString postalCode() const { return a_postal_code; }

protected:
    void initWithAddress(const QString &);

private:
    QString a_street;
    QString a_city;
    QString a_postal_code;
};

class MTAddressEdit : public QFrame
{
    Q_OBJECT

protected:
    void init();

public:
    MTAddressEdit(QWidget * parent = 0, Qt::WindowFlags f = 0):
    QFrame(parent, f) { init(); }
    MTAddressEdit(const MTAddress & address, QWidget * parent = 0, Qt::WindowFlags f = 0):
    QFrame(parent, f) { init(); setAddress(address); }

    void setAddress(const MTAddress & address);
    MTAddress address() const;

    static QString addressStringFormat(MTAddress::AddressFormat);

private:
    QLineEdit * ae_street;
    QLineEdit * ae_city;
    QLineEdit * ae_postal_code;
};

#endif // MTADDRESS_H
