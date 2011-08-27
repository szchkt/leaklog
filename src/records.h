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

class EditDialogue;
class EditDialogueWidgets;

class Modifiable
{
public:
    virtual ~Modifiable() {}

    virtual void initEditDialogue(EditDialogueWidgets *) = 0;
    virtual bool checkValues(const QVariantMap &, QWidget * = 0) { return true; }
};

class DBRecord : public QObject, public MTRecord, public Modifiable
{
    Q_OBJECT

public:
    DBRecord();
    DBRecord(const QString &, const QString &, const QString &, const MTDictionary &);

    QString parent(const QString & field) const { return MTRecord::parent(field); }
};

class Customer : public DBRecord
{
    Q_OBJECT

public:
    Customer(const QString &);

    void initEditDialogue(EditDialogueWidgets *);

    static const MTDictionary & attributes();
};

class Circuit : public DBRecord
{
    Q_OBJECT

public:
    Circuit(const QString &, const QString &);

    void initEditDialogue(EditDialogueWidgets *);
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

    void initEditDialogue(EditDialogueWidgets *);

    int scope() { return m_scope; }

private:
    int m_scope;
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

    void initEditDialogue(EditDialogueWidgets *);

    static const MTDictionary & attributes();
};

class VariableRecord : public DBRecord
{
    Q_OBJECT

public:
    VariableRecord(const QString &, const QString & = QString());

    void initEditDialogue(EditDialogueWidgets *);
};

class Table : public DBRecord
{
    Q_OBJECT

public:
    Table(const QString &, const QString & = QString(), const MTDictionary & = MTDictionary());

    void initEditDialogue(EditDialogueWidgets *);
};

class Inspector : public DBRecord
{
    Q_OBJECT

public:
    Inspector(const QString &);

    void initEditDialogue(EditDialogueWidgets *);

    static const MTDictionary & attributes();
};

class ServiceCompany : public DBRecord
{
    Q_OBJECT

public:
    ServiceCompany(const QString &);

    void initEditDialogue(EditDialogueWidgets *);

    static const MTDictionary & attributes();
};

class RecordOfRefrigerantManagement : public DBRecord
{
    Q_OBJECT

public:
    RecordOfRefrigerantManagement(const QString &);

    void initEditDialogue(EditDialogueWidgets *);

    static const MTDictionary & attributes();
};

class WarningRecord : public DBRecord
{
    Q_OBJECT

public:
    WarningRecord(const QString &);

    void initEditDialogue(EditDialogueWidgets *);
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
        ShowCompressors = 16,
        ShowCircuitUnits = 32
    };

    AssemblyRecordType(const QString &);

    void initEditDialogue(EditDialogueWidgets *);

    static const MTDictionary & attributes();
    bool remove();
};

class AssemblyRecordItemType : public DBRecord
{
    Q_OBJECT

public:
    AssemblyRecordItemType(const QString &);

    void initEditDialogue(EditDialogueWidgets *);

    static const MTDictionary & attributes();
};

class AssemblyRecordTypeCategory : public MTRecord
{
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

    void initEditDialogue(EditDialogueWidgets *);

    static const MTDictionary & attributes();
};

class AssemblyRecordItem : public MTRecord
{
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
public:
    AssemblyRecordItemByInspector(const QString &);
};

class File : public MTRecord
{
public:
    File(const QString &);
};

class Person : public MTRecord
{
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

    void initEditDialogue(EditDialogueWidgets *);
    static const QString locationToString(int);

    static const MTDictionary & attributes();
};

class CircuitUnit : public MTRecord
{
public:
    CircuitUnit(const QString & = QString(), const MTDictionary & = MTDictionary());
};

class InspectionImage : public MTRecord
{
public:
    InspectionImage(const QString &, const QString &, const QString &);
};

class Style : public DBRecord
{
    Q_OBJECT

public:
    Style(const QString & = QString());

    void initEditDialogue(EditDialogueWidgets *);

    static const MTDictionary & attributes();
};

class Compressor : public MTRecord
{
public:
    Compressor(const QString & = QString(), const MTDictionary & = MTDictionary());
};

class InspectionsCompressor : public MTRecord
{
public:
    InspectionsCompressor(const QString & = QString(), const MTDictionary & = MTDictionary());
};

#endif // RECORDS_H
