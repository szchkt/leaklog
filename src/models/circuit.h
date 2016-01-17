/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2016 Matus & Michal Tomlein

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

#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "dbrecord.h"

class Circuit : public DBRecord
{
    Q_OBJECT

public:
    enum State {
        ExcludedFromAgenda = -1,
        Commissioned = 0,
        Decommissioned = 1
    };

    Circuit();
    Circuit(const QString &, const QString &);

    void initEditDialogue(EditDialogueWidgets *);
    bool checkValues(const QVariantMap &, QWidget * = 0);

    static void cascadeIDChange(int customer_id, int old_id, int new_id, int new_customer_id = -1, bool compressors_and_units = false);

    static QString tableName();
    static const ColumnList &columns();
    static const MTDictionary &attributes();
    static int numBasicAttributes();
};

#endif // CIRCUIT_H
