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

#ifndef LINK_PARSER_H
#define LINK_PARSER_H

#include "mtdictionary.h"

#include <QMap>

class LinkEntity;
class Link;

class LinkParser
{
public:
    enum View {
        Customer,
        Repair,
        Inspector,
        InspectorReport,
        AllCustomers,
        ToggleDetailedView,
        RecordOfRefrigerantManagement,
        AssemblyRecordType,
        AssemblyRecordItemType,
        AssemblyRecordCategory,
        CircuitUnitType,
        Circuit,
        Inspection,
        AssemblyRecord,
        ServiceCompany,
        TableOfInspections,
        AllAssemblyRecords,
        Compressor
    };

    LinkParser();

    Link * parse(const QString &);

private:
    LinkEntity * root_entity;
};

class LinkEntity
{
public:
    LinkEntity(bool = true);

    LinkEntity * addRoute(const QString &, int = -1, bool = true);
    void setRoute(const QString &, LinkEntity *);

    void setView(int view) { this->m_view = view; }
    int view() { return m_view; }

    void setName(const QString & name) { this->m_name = name; }
    const QString & name() { return m_name; }

    void setHasId(bool has_id) { this->has_id = has_id; }
    bool hasId() { return has_id; }

    void parse(QString, Link *);

private:
    int m_view;
    bool has_id;
    QString m_name;
    QMap<QString, LinkEntity *> routes;
};

class Link
{
public:
    enum Action {
        View,
        Edit
    };

    Link();

    void setId(const QString &, const QString &);

    int viewAt(int i) { return m_views.count() > i ? m_views.at(i) : -1; }
    void setView(int view) { m_views << view; }
    int countViews() { return m_views.count(); }

    int action() { return m_action; }
    void setAction(int action) { m_action = action; }

    const QString & idValue(const QString &);
    const QString & lastId();

    void setOrderBy(const QString & order_by) { m_order_by = order_by; }
    const QString & orderBy() { return m_order_by; }

    void setOrderDirection(int order) { m_order_direction = order; }
    int orderDirection() { return m_order_direction; }

private:
    int m_action;
    int m_order_direction;
    QList<int> m_views;
    QString m_order_by;
    MTDictionary m_ids;
};

#endif // LINK_PARSER_H
