/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2018 Matus & Michal Tomlein

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

#include "mtvariant.h"
#include "global.h"
#include "mtaddress.h"
#include "mtdictionary.h"

MTVariant::MTVariant(const QVariant &v, const QString &t):
    v_value(v)
{
    if (t.endsWith("address"))
        v_type = Address;
    else if (t.endsWith("country"))
        v_type = Country;
    else
        v_type = Default;
}

QString MTVariant::toString() const {
    switch (v_type) {
        case Address: return MTAddress(v_value.toString()).toPlainText(); break;
        case Country: return Global::countries().value(v_value.toString()); break;
        case Default: break;
    }
    return v_value.toString();
}

QString MTVariant::toHtml() const {
    switch (v_type) {
        case Address: return MTAddress(v_value.toString()).toHtml(); break;
        case Country: return Global::countries().value(v_value.toString()); break;
        case Default: break;
    }
    return Global::escapeString(v_value.toString());
}
