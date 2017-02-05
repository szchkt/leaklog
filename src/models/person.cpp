/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2017 Matus & Michal Tomlein

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

#include "person.h"

#include <QApplication>

Person::Person(const QString &person_id):
    MTRecord(tableName(), "id", person_id, MTDictionary())
{}

Person::Person(const QString &person_id, const QString &customer_id):
    MTRecord(tableName(), "id", person_id, MTDictionary("company_id", customer_id))
{}

QString Person::tableName()
{
    return "persons";
}

class PersonColumns
{
public:
    PersonColumns() {
        columns << Column("id", "BIGINT PRIMARY KEY");
        columns << Column("company_id", "INTEGER");
        columns << Column("name", "TEXT");
        columns << Column("mail", "TEXT");
        columns << Column("phone", "TEXT");
        columns << Column("hidden", "INTEGER DEFAULT 0 NOT NULL");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &Person::columns()
{
    static PersonColumns columns;
    return columns.columns;
}

class PersonAttributes
{
public:
    PersonAttributes() {
        dict.insert("id", QApplication::translate("Person", "ID"));
        dict.insert("company_id", QApplication::translate("Person", "Company ID"));
        dict.insert("name", QApplication::translate("Person", "Name"));
        dict.insert("mail", QApplication::translate("Person", "E-mail"));
        dict.insert("phone", QApplication::translate("Person", "Phone"));
        dict.insert("hidden", QApplication::translate("Person", "Hidden"));
    }

    MTDictionary dict;
};

const MTDictionary &Person::attributes()
{
    static PersonAttributes dict;
    return dict.dict;
}
