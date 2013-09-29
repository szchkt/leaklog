/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2013 Matus & Michal Tomlein

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

#ifndef MTVARIANT_H
#define MTVARIANT_H

#include <QVariant>

class MTVariant
{
public:
    enum Type { Default = 0, Address = 128 };
    MTVariant(const QVariant &v = QVariant(), Type t = Default): v_value(v), v_type(t) {}
    MTVariant(const QVariant &v, const QString &t);

    inline void setType(Type t) { v_type = t; }
    inline Type type() const { return v_type; }
    inline QVariant::Type variantType() const { return v_value.type(); }
    inline void setValue(const QVariant &v) { v_value = v; }
    inline QVariant value() const { return v_value; }

    QString toString() const;
    QString toHtml() const;

private:
    QVariant v_value;
    Type v_type;
};

#endif // MTVARIANT_H
