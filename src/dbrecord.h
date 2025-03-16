/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2025 Matus & Michal Tomlein

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

#ifndef DBRECORD_H
#define DBRECORD_H

#include "mtquery.h"
#include "mtrecord.h"

#include <functional>

#include <QSet>

class EditDialogueWidgets;

class Column
{
public:
    Column(const QString &name, const QString &type);

    QString name() const;
    QString type() const;

    QString toString() const;

private:
    QString _name;
    QString _type;
};

class ColumnList : public QList<Column>
{
public:
    QString toString() const;
    QStringList toStringList(std::function<QString(const Column &)> f) const;
    QStringList columnNameList() const;
    QSet<QString> columnNameSet() const;
};

class Modifiable
{
public:
    virtual ~Modifiable() {}

    virtual void initEditDialogue(EditDialogueWidgets *) = 0;
    virtual bool checkValues(QWidget * = 0) { return true; }
};

class DBRecord : public QObject, public MTRecord, public Modifiable
{
    Q_OBJECT

public:
    DBRecord();
    DBRecord(const QString &type, const QString &uuid, const QVariantMap &savedValues = QVariantMap());
    DBRecord(const DBRecord &other): MTRecord(other) {}
    DBRecord &operator=(const DBRecord &other) { MTRecord::operator=(other); return *this; };

    QString dateUpdated();
    QString updatedBy();

protected:
    bool showErrorMessage(QWidget *parent, const QString &title, const QString &text, const QString &informativeText = QString()) const;
};

#endif // DBRECORD_H
