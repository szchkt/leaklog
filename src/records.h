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

#ifndef RECORDS_H
#define RECORDS_H

#include "mtrecord.h"

class ModifyDialogue;

class Modifiable
{
public:
    virtual ~Modifiable() {}

    virtual void initModifyDialogue(ModifyDialogue *) = 0;
    virtual bool checkValues(const QVariantMap &, QWidget * = 0) { return true; }
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
    bool checkValues(const QVariantMap &, QWidget * = 0);

    static const MTDictionary & attributes();
    static int numBasicAttributes();
};

class Inspection : public DBRecord
{
    Q_OBJECT

public:
    Inspection(const QString &, const QString &, const QString &);
    Inspection(const QString &, const QString &, const QString &, const MTDictionary &);

    void initModifyDialogue(ModifyDialogue *);
};

class InspectionByInspector : public Inspection
{
    Q_OBJECT

public:
    InspectionByInspector(const QString &);
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

class AssemblyRecordType : public DBRecord
{
    Q_OBJECT

public:
    enum DisplayOptions {
        ShowServiceCompany = 1,
        ShowCustomer = 2,
        ShowCustomerContactPersons = 4,
        ShowCircuit = 8,
        ShowCircuitUnits = 16
            };

    AssemblyRecordType(const QString &);

    void initModifyDialogue(ModifyDialogue *);

    static const MTDictionary & attributes();
    bool remove();
};

class AssemblyRecordItemType : public DBRecord
{
    Q_OBJECT

public:
    AssemblyRecordItemType(const QString &);

    void initModifyDialogue(ModifyDialogue *);

    static const MTDictionary & attributes();
};

class AssemblyRecordTypeCategory : public MTRecord
{
    Q_OBJECT

public:
    AssemblyRecordTypeCategory(const QString &);

    static const MTDictionary & attributes();
};

class AssemblyRecordItemCategory : public DBRecord
{
    Q_OBJECT

public:
    enum DisplayOptions {
        ShowValue = 1,
        ShowListPrice = 2,
        ShowAcquisitionPrice = 4,
        ShowDiscount = 8,
        ShowTotal = 16
            };
    enum DisplayPosition {
        DisplayAtTop = 0,
        DisplayAtBottom = 1
    };

    AssemblyRecordItemCategory(const QString &);

    void initModifyDialogue(ModifyDialogue *);

    static const MTDictionary & attributes();
};

class AssemblyRecordItem : public MTRecord
{
    Q_OBJECT

public:
    enum Source {
        AssemblyRecordItemTypes = 0,
        CircuitUnitTypes = 1,
        Inspectors = 2
            };

    AssemblyRecordItem(const QString &);
    AssemblyRecordItem(const QString &, const QString &, const QString &, const MTDictionary &);

    static const MTDictionary & attributes();
};

class AssemblyRecordItemByInspector : public AssemblyRecordItem
{
    Q_OBJECT

public:
    AssemblyRecordItemByInspector(const QString &);
};

class File : public MTRecord
{
    Q_OBJECT

public:
    File(const QString &);
};

class Person : public MTRecord
{
    Q_OBJECT

public:
    Person(const QString & = QString(), const QString & = QString());
};

class CircuitUnitType : public DBRecord
{
    Q_OBJECT

public:
    enum Locations {
        External = 0,
        Internal = 1
            };

    CircuitUnitType(const QString &);

    void initModifyDialogue(ModifyDialogue *);
    static const QString locationToString(int);

    static const MTDictionary & attributes();
};

class CircuitUnit : public MTRecord
{
    Q_OBJECT

public:
    CircuitUnit(const QString & = QString(), const MTDictionary & = MTDictionary());
};

class InspectionImage : public MTRecord
{
    Q_OBJECT

public:
    InspectionImage(const QString &, const QString &, const QString &);
};

class Style : public DBRecord
{
    Q_OBJECT

public:
    Style(const QString & = QString());

    void initModifyDialogue(ModifyDialogue *);

    static const MTDictionary & attributes();
};

#endif // RECORDS_H
