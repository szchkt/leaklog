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

#ifndef GLOBAL_H
#define GLOBAL_H

#include "mtdictionary.h"

#include <QApplication>
#include <QVariant>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

namespace Global {
    QString toString(const QVariant &);
    QString upArrow();
    QString downArrow();
    void copyTable(const QString &, QSqlDatabase *, QSqlDatabase *, const QString & = QString());
    QStringList getTableFieldNames(const QString &, QSqlDatabase *);
}

class MTRecord : public QObject
{
    Q_OBJECT

public:
    MTRecord() {};
    MTRecord(const QString &, const QString &, const MTDictionary &);
    MTRecord(const MTRecord &);
    MTRecord & operator=(const MTRecord &);
    void setType(const QString & type) { r_type = type; };
    inline QString type() { return r_type; };
    inline QString id() { return r_id; };
    inline MTDictionary * parents() { return &r_parents; };
    QSqlQuery select(const QString & = "*");
    QMap<QString, QVariant> list(const QString & = "*");
    QList<QMap<QString, QVariant> > listAll(const QString & = "*");
    bool update(const QMap<QString, QVariant> &, bool = false);
    bool remove();

protected:
    QString tableForRecordType(const QString &);

private:
    QString r_type;
    QString r_id;
    MTDictionary r_parents;
};

#endif // GLOBAL_H
