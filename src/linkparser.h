/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2014 Matus & Michal Tomlein

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

#ifndef LINK_PARSER_H
#define LINK_PARSER_H

#include "mtdictionary.h"
#include "view.h"

class LinkEntity;
class Link;
class UrlEntity;

class LinkParser
{
public:
    enum View {
        ToggleDetailsVisible = 1,
        Store = 100,
        ToggleDetailedView = 101,
        RefrigerantManagement = 200,
        RecordOfRefrigerantManagement = 201,
        LeakagesByApplication = 300,
        Agenda = 400,
        AllInspectors = 500,
        Inspector = 501,
        InspectorReport = 600,
        AllCustomers = 700,
        Customer = 800,
        AllRepairs = 801,
        Repair = 802,
        OperatorReport = 900,
        Circuit = 1000,
        Compressor = 1001,
        Inspection = 1100,
        InspectionImages = 1200,
        TableOfInspections = 1300,
        AllAssemblyRecords = 1400,
        AssemblyRecord = 1401,
        AllAssemblyRecordTypes = 1500,
        AssemblyRecordType = 1501,
        AllAssemblyRecordItems = 1600,
        AssemblyRecordItemType = 1601,
        AssemblyRecordCategory = 1602,
        AllCircuitUnitTypes = 1800,
        CircuitUnitType = 1801
    };

    LinkParser();

    Link *parse(const QString &);
    Link *parse(UrlEntity *);

private:
    LinkEntity *root_entity;
};

class LinkEntity
{
public:
    LinkEntity(bool = true);

    LinkEntity *addRoute(const QString &, int = -1, bool = true);
    void setRoute(const QString &, LinkEntity *);

    void setView(int view) { m_view = view; }
    int view() const { return m_view; }

    void setName(const QString &name) { m_name = name; }
    QString name() const { return m_name; }

    void setHasId(bool has_id) { m_has_id = has_id; }
    bool hasId() const { return m_has_id; }

    void parse(UrlEntity *, Link *);

private:
    int m_view;
    bool m_has_id;
    QString m_name;
    QMap<QString, LinkEntity *> routes;
};

class Link
{
public:
    enum {
        MinViewDifference = 50,
        MaxViewBits = 16
    };

    enum Action {
        View,
        Edit
    };

    Link();

    void setId(const QString &, const QString &);

    int compareViews(const Link &other) const;
    int viewAt(int i) const { return m_views.count() > i ? m_views.at(i) : -1; }
    void addView(int view) { m_views << view; }
    int countViews() const { return m_views.count(); }
    quint64 views() const;
    void setViews(quint64 views);
    View::ViewID viewId() const;

    int action() { return m_action; }
    void setAction(int action) { m_action = action; }

    QString idValue(const QString &) const;
    QString lastIdKey() const;
    QString lastIdValue() const;
    int countIds() const { return m_ids.count(); }

    QString suffixParameters() const;

    void setOrderBy(const QString &order_by) { m_order_by = order_by; }
    QString orderBy() const { return m_order_by; }

    void setOrderDirection(int order) { m_order_direction = order; }
    int orderDirection() const { return m_order_direction; }

private:
    int m_action;
    int m_order_direction;
    QList<int> m_views;
    QString m_order_by;
    MTDictionary m_ids;
};

class UrlEntity
{
public:
    UrlEntity() : m_next(NULL) {}
    UrlEntity(const QStringList &attributes) : m_next(NULL)
        { m_attributes << attributes; }
    UrlEntity(const QString &attr1, const QString &attr2 = QString()) : m_next(NULL)
        { m_attributes.append(attr1); if (!attr2.isNull()) m_attributes.append(attr2); }

    ~UrlEntity() { if (m_next) delete m_next; }

    void addAttribute(const QString &attr) { m_attributes.append(attr); }

    UrlEntity *addNext(const QStringList &attributes);
    UrlEntity *addNext(const QString &attr1, const QString &attr2 = QString());
    UrlEntity *next() const { return m_next; }

    int countAttributes() const { return m_attributes.count(); }
    QString name() const { return m_attributes.first(); }
    QString attributeAt(int i) const { return m_attributes.at(i); }

private:
    QStringList m_attributes;
    UrlEntity *m_next;
};

#endif // LINK_PARSER_H
