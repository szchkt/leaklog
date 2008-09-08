/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008 Matus & Michal Tomlein

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

#include "global.h"

QString Global::toString(const QVariant & v) { return v.toString(); }

QString Global::upArrow() { return QApplication::translate("Global", "\342\206\221", 0, QApplication::UnicodeUTF8); }

QString Global::downArrow() { return QApplication::translate("Global", "\342\206\223", 0, QApplication::UnicodeUTF8); }

void Global::copyTable(const QString & table, QSqlDatabase * from, QSqlDatabase * to, const QString & filter)
{
    QSqlQuery select("SELECT * FROM " + table + QString(filter.isEmpty() ? "" : (" WHERE " + filter)), *from);
    if (select.next() && select.record().count()) {
        QString copy("INSERT INTO " + table + " (");
        QStringList field_names = getTableFieldNames(table, to);
        QString field_name;
        for (int i = 0; i < select.record().count(); ++i) {
            field_name = select.record().fieldName(i);
            copy.append(i == 0 ? "" : ", ");
            copy.append(field_name);
            if (!field_names.contains(field_name)) {
                QSqlQuery add_column("ALTER TABLE " + table + " ADD COLUMN " + field_name + " VARCHAR", *to);
            }
        }
        copy.append(") VALUES (");
        for (int i = 0; i < select.record().count(); ++i) {
            copy.append(i == 0 ? ":" : ", :");
            copy.append(select.record().fieldName(i));
        }
        copy.append(")");
        do {
            QSqlQuery insert(*to);
            insert.prepare(copy);
            for (int i = 0; i < select.record().count(); ++i) {
                insert.bindValue(":" + select.record().fieldName(i), select.value(i));
            }
            insert.exec();
        } while (select.next());
    }
}

QStringList Global::getTableFieldNames(const QString & table, QSqlDatabase * database)
{
    QStringList field_names;
    QSqlQuery query("SELECT * FROM " + table, *database);
    for (int i = 0; i < query.record().count(); ++i) {
        field_names << query.record().fieldName(i);
    }
    return field_names;
}
