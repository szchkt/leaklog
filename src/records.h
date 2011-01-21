/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2010 Matus & Michal Tomlein

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

#ifndef RECORDS_H
#define RECORDS_H

#include "mtrecord.h"

class ModifyDialogue;

class Modifiable
{
public:
    virtual ~Modifiable() {}

    virtual void initModifyDialogue(ModifyDialogue *) = 0;
};

class DBRecord : public MTRecord, public Modifiable
{
    Q_OBJECT

public:
    DBRecord();
    DBRecord(const QString &, const QString &, const QString &, const MTDictionary &);
};

class Customer : public DBRecord
{
    Q_OBJECT

public:
    Customer(const QString &);

    void initModifyDialogue(ModifyDialogue *);

    static const MTDictionary & attributes();
};

class Circuit : public DBRecord
{
    Q_OBJECT

public:
    Circuit(const QString &, const QString &);

    void initModifyDialogue(ModifyDialogue *);

    static const MTDictionary & attributes();
    static int numBasicAttributes();
};

class Inspection : public DBRecord
{
    Q_OBJECT

public:
    Inspection(const QString &, const QString &, const QString &);

    void initModifyDialogue(ModifyDialogue *);
};

class Repair : public DBRecord
{
    Q_OBJECT

public:
    Repair(const QString &);

    void initModifyDialogue(ModifyDialogue *);

    static const MTDictionary & attributes();
};

class VariableRecord : public DBRecord
{
    Q_OBJECT

public:
    enum Type { VARIABLE = 0, SUBVARIABLE = 1 };

    VariableRecord(Type, const QString &, const QString & = QString());

    void initModifyDialogue(ModifyDialogue *);

private:
    Type v_type;
};

class Table : public DBRecord
{
    Q_OBJECT

public:
    Table(const QString &, const QString & = QString());

    void initModifyDialogue(ModifyDialogue *);
};

class Inspector : public DBRecord
{
    Q_OBJECT

public:
    Inspector(const QString &);

    void initModifyDialogue(ModifyDialogue *);

    static const MTDictionary & attributes();
};

class ServiceCompany : public DBRecord
{
    Q_OBJECT

public:
    ServiceCompany(const QString &);

    void initModifyDialogue(ModifyDialogue *);

    static const MTDictionary & attributes();
};

class RecordOfRefrigerantManagement : public DBRecord
{
    Q_OBJECT

public:
    RecordOfRefrigerantManagement(const QString &);

    void initModifyDialogue(ModifyDialogue *);

    static const MTDictionary & attributes();
};

class WarningRecord : public DBRecord
{
    Q_OBJECT

public:
    WarningRecord(const QString &);

    void initModifyDialogue(ModifyDialogue *);
};

#endif // RECORDS_H
