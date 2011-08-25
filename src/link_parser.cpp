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

#include <QDebug>

LinkParser::LinkParser()
{
    root_entity = new LinkEntity(false);

    LinkEntity * customer_entity = root_entity->addRoute("customer", Customer);
    root_entity->addRoute("allrepairs", AllRepairs, false);
    root_entity->addRoute("repair", Repair);
    root_entity->addRoute("allinspectors", AllInspectors, false);
    root_entity->addRoute("inspector", Inspector);
    root_entity->addRoute("inspectorreport", InspectorReport);
    root_entity->addRoute("allcustomers", AllCustomers, false);
    root_entity->addRoute("toggledetailedview", ToggleDetailedView);
    root_entity->addRoute("refrigerantmanagement", RefrigerantManagement, false);
    root_entity->addRoute("recordofrefrigerantmanagement", RecordOfRefrigerantManagement);
    root_entity->addRoute("leakagesbyapplication", LeakagesByApplication, false);
    root_entity->addRoute("agenda", Agenda, false);
    root_entity->addRoute("allassemblyrecordtypes", AllAssemblyRecordTypes, false);
    root_entity->addRoute("assemblyrecordtype", AssemblyRecordType);
    root_entity->addRoute("allassemblyrecorditemtypes", AllAssemblyRecordItemTypes, false);
    root_entity->addRoute("assemblyrecorditemtype", AssemblyRecordItemType);
    root_entity->addRoute("allassemblyrecorditemcategories", AllAssemblyRecordItemCategories, false);
    root_entity->addRoute("assemblyrecorditemcategory", AssemblyRecordCategory);
    root_entity->addRoute("allcircuitunittypes", AllCircuitUnitTypes, false);
    root_entity->addRoute("circuitunittype", CircuitUnitType, false);
    root_entity->addRoute("servicecompany", ServiceCompany, false);
    root_entity->addRoute("allassemblyrecords", AllAssemblyRecords, false);

    LinkEntity * circuit_entity = customer_entity->addRoute("circuit", Circuit);
    customer_entity->addRoute("allassemblyrecords", AllAssemblyRecords, false);
    customer_entity->addRoute("operatorreport", OperatorReport, false);
    customer_entity->addRoute("allrepairs", AllRepairs, false);

    LinkEntity * inspection_entity = circuit_entity->addRoute("inspection", Inspection);
    circuit_entity->setRoute("repair", inspection_entity);
    circuit_entity->addRoute("allassemblyrecords", AllAssemblyRecords, false);
    circuit_entity->addRoute("assemblyrecord", AssemblyRecord);
    circuit_entity->addRoute("table", TableOfInspections, false);

    LinkEntity * compressor_entity = circuit_entity->addRoute("compressor", Compressor);
    compressor_entity->addRoute("table", TableOfInspections, false);

    inspection_entity->addRoute("assemblyrecord", AssemblyRecord, false);
    inspection_entity->addRoute("images", InspectionImages, false);
}

Link * LinkParser::parse(const QString & url)
{
    if (url == "qrc:/html/")
        return NULL;

    QStringList split_url = url.split("/");
    UrlEntity * url_entity = new UrlEntity;
    UrlEntity * entity = url_entity;
    for (int i = 0; i < split_url.count(); ++i) {
        QStringList args = split_url.at(i).split(":");
        QString name = args.takeFirst();
        entity = entity->addNext(name, args.join(":"));
    }

    return parse(url_entity);
}

Link * LinkParser::parse(UrlEntity * url_entity)
{
    if (!url_entity) return NULL;

    Link * link = new Link;
    root_entity->parse(url_entity, link);

    delete url_entity;

    return link;
}

LinkEntity::LinkEntity(bool has_id):
    m_view(-1),
    has_id(has_id)
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

void LinkEntity::parse(UrlEntity * url, Link * link)
{
    if (view() >= 0) link->setView(view());

    if (!url || !url->countAttributes()) {
        link->setAction(Link::View);
        return;
    }

    if (routes.contains(url->name())) {
        LinkEntity * next = routes.value(url->name());

        if (next->hasId() && url->countAttributes() > 1) {
            link->setId(next->name(), url->attributeAt(1));
        }

        next->parse(url->next(), link);
    } else {
        link->setAction(Link::View);
        if (url->name() == "edit") {
            link->setAction(Link::Edit);
        } else if (url->name() == "order_by" && url->countAttributes() > 1) {
            QStringList subargs = url->attributeAt(1).split(":");
            link->setOrderBy(subargs.takeFirst());
            if (subargs.count())
                link->setOrderDirection(subargs.first() == "asc" ? Qt::AscendingOrder : Qt::DescendingOrder);
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

UrlEntity * UrlEntity::addNext(const QStringList & attributes)
{
    if (m_attributes.count())
        return m_next = new UrlEntity(attributes);
    m_attributes << attributes;
    return this;
}

UrlEntity * UrlEntity::addNext(const QString &attr1, const QString &attr2)
{
    if (m_attributes.count())
        return m_next = new UrlEntity(attr1, attr2);
    m_attributes << attr1;
    if (!attr2.isNull())
        m_attributes << attr2;
    return this;
}
