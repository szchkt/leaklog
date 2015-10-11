/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2015 Matus & Michal Tomlein

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

#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "dbrecord.h"

class MTAddress;
class Circuit;
class Person;

class Customer : public DBRecord
{
    Q_OBJECT

public:
    enum OperatorType {
        OperatorTypeServiceCompany = -1,
        OperatorTypeCustomer,
        OperatorTypeOther
    };

    Customer(const QString &uuid = QString());

    void initEditDialogue(EditDialogueWidgets *);
    void readOperatorValues();

    QString companyID();
    QString companyName();
    MTAddress address();
    QString mail();
    QString phone();
    OperatorType operatorType();
    QString operatorCompanyID();
    QString operatorCompanyName();
    MTAddress operatorAddress();
    QString operatorMail();
    QString operatorPhone();

    Circuit circuits();
    Person persons();

    static QString tableName();
    static const ColumnList &columns();
    static const MTDictionary &attributes();
    static int numBasicAttributes();
};

#endif // CUSTOMER_H
