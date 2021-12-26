/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2021 Matus & Michal Tomlein

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

#include "companyidvalidator.h"

#include "global.h"

using namespace Global;

CompanyIDValidator::CompanyIDValidator(QObject *parent):
QValidator(parent)
{
    _format = companyIDFormat();
}

QValidator::State CompanyIDValidator::validate(QString &input, int &) const
{
    if (input.isEmpty())
        return QValidator::Acceptable;

    if (input.contains(QRegularExpression("[^0-9]")))
        return QValidator::Invalid;

    if (_format == Global::CompanyIDFormatNIP) {
        if (input.length() < 10)
            return QValidator::Intermediate;

        if (input.length() > 10)
            return QValidator::Invalid;

        return formatCompanyID(input) == input ? QValidator::Acceptable : QValidator::Invalid;
    } else {
        if (input.length() < 8)
            return QValidator::Intermediate;

        if (input.length() > 8)
            return QValidator::Invalid;
    }

    return QValidator::Acceptable;
}

void CompanyIDValidator::fixup(QString &input) const
{
    if (!input.isEmpty()) {
        input.remove(QRegularExpression("[^0-9]+"));
        input = formatCompanyID(input);
    }
}
