/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2013 Matus & Michal Tomlein

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

#ifndef VIEW_H
#define VIEW_H

#include <QObject>
#include <QMap>

class ViewTabSettings;

class QString;

class View : public QObject
{
    Q_OBJECT

public:
    enum ViewID {
        Store,
        RefrigerantManagement,
        LeakagesByApplication,
        Agenda,
        Inspectors,
        InspectorDetails,             InspectorRequired = InspectorDetails,
                                      InspectorRequiredEnd = InspectorDetails,
        Customers,
        Repairs,
        Circuits,                     CustomerRequired = Circuits,
        TableOfInspections,
        OperatorReport,
        Inspections,                  CircuitRequired = Inspections,
        InspectionDetails,            InspectionRequired = InspectionDetails,
        InspectionImages,
        AssemblyRecordDetails,        InspectionRequiredEnd = AssemblyRecordDetails,
                                      AssemblyRecordsRelated = AssemblyRecordDetails,
        AssemblyRecords,              CircuitRequiredEnd = AssemblyRecords,
                                      CustomerRequiredEnd = AssemblyRecords,
        AssemblyRecordTypes,
        AssemblyRecordItemTypes,
        AssemblyRecordItemCategories,
        CircuitUnitTypes,             AssemblyRecordsRelatedEnd = CircuitUnitTypes,
        ViewCount
    };

    View(ViewTabSettings * settings);

    virtual QString renderHTML() = 0;

    virtual QString title() const = 0;

protected:
    static QString viewTemplate(const QString & view_template);

    ViewTabSettings * settings;

private:
    static QMap<QString, QString> view_templates;
};

#endif // VIEW_H
