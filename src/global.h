/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2019 Matus & Michal Tomlein

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

#include "defs.h"

#include <QSqlDatabase>
#include <QStringList>

template <class T1, class T2>
struct QPair;

class MTDictionary;
class QDir;
class QColor;

namespace Global {
#ifdef Q_OS_MAC
    int macVersion();
#endif
    double scaleFactor(bool refresh = false);
    QString escapeString(const QVariant &, bool = false, bool = false);
    QString escapeString(QString, bool = false, bool = false);
    QString elideRight(const QString &, int);
    QString upArrow();
    QString downArrow();
    QString rightTriangle();
    QString degreeSign();
    QString delta();
    QString longMonthName(int);
    QColor textColourForBaseColour(const QColor &);
    QString createUUID();
    QString createUUIDv5(const QString &ns, const QString &name);
    QString sourceUUID();
    QString sqlStringForDatabaseType(QString, const QSqlDatabase & = QSqlDatabase::database());
    QString variantTypeToSqlType(int);
    QString variableTypeToSqlType(const QString &);
    MTDictionary getTableFieldNames(const QString &, const QSqlDatabase &);
    void copyTable(const QString &, const QSqlDatabase &, const QSqlDatabase &, const QString & = QString());
    void addColumn(const QString &, const QString &, const QSqlDatabase &);
    void renameColumn(const QString &, const QString &, const QString &, const QSqlDatabase &);
    void dropColumn(const QString &, const QString &, const QSqlDatabase &);
    bool journalInsertion(const QString &table_name, const QString &record_uuid, const QSqlDatabase &database = QSqlDatabase::database());
    bool journalInsertion(int table_id, const QString &record_uuid, const QSqlDatabase &database = QSqlDatabase::database());
    bool journalUpdate(const QString &table_name, const QString &record_uuid, const QString &column_name, const QSqlDatabase &database = QSqlDatabase::database());
    bool journalUpdate(int table_id, const QString &record_uuid, int column_id, const QSqlDatabase &database = QSqlDatabase::database());
    bool journalDeletion(const QString &table_name, const QString &record_uuid, const QSqlDatabase &database = QSqlDatabase::database());
    bool journalDeletion(int table_id, const QString &record_uuid, const QSqlDatabase &database = QSqlDatabase::database());
    QPair<bool, QDir> backupDirectoryForDatabasePath(const QString &path);
    bool superuserModeEnabled();
    QString currentUser(const QSqlDatabase & = QSqlDatabase::database());
    bool isDatabaseRemote(const QSqlDatabase & = QSqlDatabase::database());
    bool isOwnerPermissionApplicable(const QString &);
    QString circuitRefrigerantAmountQuery(const QString &return_as = "refrigerant_amount");
    QString compareValues(double, double, double = 0.0, const QString & = QString());
    enum CompanyIDFormat {
        CompanyIDFormatNone = 0,
        CompanyIDFormatDefault = 1,
        CompanyIDFormatNIP = 2
    };
    CompanyIDFormat companyIDFormat();
    QString formatCompanyID(int company_id);
    QString formatCompanyID(const QVariant &company_id);
    QString formatCompanyID(const QString &company_id, CompanyIDFormat format = companyIDFormat());
    enum ToolTipLinkItem {
        ToolTipLinkItemView = 1 << 0,
        ToolTipLinkItemEdit = 1 << 1,
        ToolTipLinkItemRemove = 1 << 2
    };
    QString toolTipLink(const QString &type, const QString &text, const QString &l1, const QString &l2 = QString(), const QString &l3 = QString(), int items = ToolTipLinkItemView | ToolTipLinkItemEdit);
    // Dictionaries
    const MTDictionary &databaseTables();
    const MTDictionary &variableTypes();
    const MTDictionary &variableNames();
    const QString variableType(const QString &, bool * = 0);
    const MTDictionary &fieldsOfApplication();
    int fieldOfApplicationToId(const QString &field);
    QString idToFieldOfApplication(int id);
    const MTDictionary &oils();
    const MTDictionary &attributeValues();
    const MTDictionary &permissions();
    const MTDictionary &countries();
    double refrigerantGWP(const QString &refrigerant);
    double CO2Equivalent(const QString &refrigerant, double refrigerant_amount);
    // List
    QStringList listRefrigerants(bool include_user_refrigerants = true);
    MTDictionary listInspectors();
    MTDictionary listAssemblyRecordItemCategories(bool = false);
    MTDictionary listAssemblyRecordTypes();
    MTDictionary listAllVariables();
    MTDictionary listDataTypes();
    MTDictionary listOperators(const QString &customer_uuid);
    MTDictionary listStyles();
    QStringList listVariableIds(bool = false);
    QStringList listSupportedFunctions();
    // Data types
    enum DataType {
        String = 0,
        Integer = 1,
        Numeric = 2,
        Boolean = 3,
        Text = 4,
        File = 5
    };

    template <typename SourceType, typename ReturnType>
    QList<ReturnType> map(const QList<SourceType> &list, std::function<ReturnType(const SourceType &)> function)
    {
        QList<ReturnType> result;
        result.reserve(list.count());
        foreach (const SourceType &i, list) {
            result << function(i);
        }
        return result;
    }

    template <typename SourceType>
    QStringList map(const QList<SourceType> &list, std::function<QString(const SourceType &)> function)
    {
        QStringList result;
        result.reserve(list.count());
        foreach (const SourceType &i, list) {
            result << function(i);
        }
        return result;
    }

    inline QStringList map(const QStringList &list, std::function<QString(const QString &)> function)
    {
        return map<QString>(list, function);
    }
}

#endif // GLOBAL_H
