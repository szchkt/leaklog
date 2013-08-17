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

#include "leakagesbyapplicationview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "leakagesbyapplication.h"

#include <QVector>
#include <QApplication>

using namespace Global;

LeakagesByApplicationView::LeakagesByApplicationView(ViewTabSettings *settings):
    View(settings)
{
}

QString LeakagesByApplicationView::renderHTML()
{
    QString html; MTTextStream out(&html);

    class LeakagesByApplication leakages(true);

    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><tr>";
    out << "<th style=\"font-size: medium;\">" << tr("Leakages by Application") << "</th></tr></table><br>";

    for (int t = 0; t < leakages.tableNames().count(); ++t) {
        out << "<table><thead><tr><th rowspan=\"2\" width=\"15%\">" << leakages.tableNames().at(t) << "</th>";
        out << "<th colspan=\"7\">" << tr("Fields") << "</th></tr>";
        out << "<tr><th>" << tr("All") << "</th>";

        for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
            out << "<th>" << attributeValues().value(n) << "</th>";
        }

        out << "</tr></thead>";
        out << "<tr><th>" << tr("All") << "</th>";
        out << "<td>" << leakages.value().at(t) << "</td>";

        for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
            out << "<td>" << leakages.value(LeakagesByApplication::Key::All, attributeValues().value(n)).at(t) << "</td>";
        }

        out << "</tr>";

        for (int i = 0; i < leakages.usedRefrigerants().count(); ++i) {
            out << "<tr><th>" << leakages.usedRefrigerants().value(i) << "</th>";
            out << "<td>" << leakages.value(leakages.usedRefrigerants().key(i)).at(t) << "</td>";

            for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
                out << "<td>" << leakages.value(leakages.usedRefrigerants().key(i), attributeValues().value(n)).at(t) << "</td>";
            }

            out << "</tr>";
        }

        out << "<tr></tr></table><br>";
    }

    return viewTemplate("leakages").arg(html);
}

QString LeakagesByApplicationView::title() const
{
    return tr("Leakages by Application");
}
