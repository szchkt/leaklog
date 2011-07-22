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

#include "link_parser.h"

LinkParser::LinkParser()
{
    root_entity = new LinkEntity(false);

    LinkEntity * customer_entity = root_entity->addRoute("customer", Customer);
    root_entity->addRoute("repair", Repair);
    root_entity->addRoute("inspector", Inspector);
    root_entity->addRoute("inspectorreport", InspectorReport);
    root_entity->addRoute("allcustomers", AllCustomers, false);
    root_entity->addRoute("toggledetailedview", ToggleDetailedView);
    root_entity->addRoute("recordofrefrigerantmanagement", RecordOfRefrigerantManagement);
    root_entity->addRoute("assemblyrecordtype", AssemblyRecordType);
    root_entity->addRoute("assemblyrecorditemtype", AssemblyRecordItemType);
    root_entity->addRoute("assemblyrecorditemcategory", AssemblyRecordCategory);
    root_entity->addRoute("circuitunittype", CircuitUnitType, false);
    root_entity->addRoute("servicecompany", ServiceCompany, false);
    root_entity->addRoute("allassemblyrecords", AllAssemblyRecords, false);

    LinkEntity * circuit_entity = customer_entity->addRoute("circuit", Circuit);

    LinkEntity * inspection_entity = circuit_entity->addRoute("inspection", Inspection);
    circuit_entity->setRoute("repair", inspection_entity);
    circuit_entity->addRoute("assemblyrecord", AssemblyRecord);
    circuit_entity->addRoute("table", TableOfInspections, false);

    inspection_entity->addRoute("assemblyrecord", AssemblyRecord, false);
}

Link * LinkParser::parse(const QString & url)
{
    Link * link = new Link;
    root_entity->parse(url, link);

    return link;
}

LinkEntity::LinkEntity(bool has_id):
    has_id(has_id),
    m_view(-1)
{}

LinkEntity * LinkEntity::addRoute(const QString & name, int view, bool has_id)
{
    if (routes.contains(name)) return routes.value(name);

    LinkEntity * entity = new LinkEntity(has_id);
    entity->setView(view);
    entity->setName(name);
    setRoute(name, entity);

    return entity;
}

void LinkEntity::setRoute(const QString & name, LinkEntity * entity)
{
    routes.insert(name, entity);
}

void LinkEntity::parse(QString url, Link * link)
{
    QStringList split_url = url.split("/");
    QStringList entity_list = split_url.takeFirst().split(":");
    url = split_url.join("/");

    QString route = entity_list.takeFirst();
    QString entity_id = entity_list.join(":");

    if (view() >= 0) link->setView(view());

    if (routes.contains(route)) {
        LinkEntity * next = routes.value(route);

        if (next->hasId()) {
            link->setId(next->name(), entity_id);
        }

        next->parse(url, link);
    } else {
        link->setAction(Link::View);
        if (route == "modify") {
            link->setAction(Link::Modify);
        } else if (route == "order_by") {
            if (!entity_list.empty())
                link->setOrderBy(entity_list.takeFirst());
            if (!entity_list.empty())
                link->setOrderDirection(entity_list.takeFirst() == "asc" ? Qt::AscendingOrder : Qt::DescendingOrder);
        }
    }
}

Link::Link():
    m_action(-1),
    m_order_direction(-1)
{}

void Link::setId(const QString & key, const QString & value)
{
    m_ids.setValue(key, value);
}

const QString & Link::idValue(const QString & key)
{
    return m_ids.value(key);
}

const QString & Link::lastId()
{
    return m_ids.lastValue();
}
