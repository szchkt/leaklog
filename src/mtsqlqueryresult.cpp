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

#include "mtsqlqueryresult.h"

void MTSqlQueryResult::saveResult()
{
    int n = _query->record().count();
    _pos = -1;
    _result.clear();
    QMap<int, QVariant> row;
    while (_query->next()) {
        row.clear();
        for (int i = 0; i < n; ++i) {
            row.insert(i, _query->value(i));
        }
        _result << row;
    }
}
