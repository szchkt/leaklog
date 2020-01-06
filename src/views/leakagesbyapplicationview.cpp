/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2020 Matus & Michal Tomlein

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
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "htmlbuilder.h"

#include <QVector>
#include <QApplication>

using namespace Global;

LeakagesByApplicationView::LeakagesByApplicationView(ViewTabSettings *settings):
    View(settings)
{
}

QString LeakagesByApplicationView::renderHTML(bool)
{
    QString html; MTTextStream out(&html);

    writeServiceCompany(out);

    QStringList units;
    units << "&nbsp;" + QApplication::translate("Units", "kg");
    units << "&nbsp;" + QApplication::translate("Units", "kg");
    units << "&nbsp;" + QApplication::translate("Units", "%");

    class LeakagesByApplication leakages(true);

    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><tr>";
    out << "<th style=\"font-size: medium;\">" << tr("Leakages by Application") << "</th></tr></table><br>";

    out << "<table><thead><tr><th rowspan=\"2\" width=\"15%\">" << tr("Weighted Average") << "</th>";
    out << "<th colspan=\"7\">" << tr("Fields") << "</th></tr>";
    out << "<tr><th>" << tr("All") << "</th>";

    for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
        if (leakages.containsField(attributeValues().key(n).mid(7))) {
            out << "<th>" << attributeValues().value(n) << "</th>";
        }
    }

    out << "</tr><tr><th>" << tr("Refrigerant") << "</th>";

    for (int n = attributeValues().indexOfKey("field"); n < attributeValues().count() && attributeValues().key(n).startsWith("field"); ++n) {
        if (attributeValues().key(n) == "field" || leakages.containsField(attributeValues().key(n).mid(7))) {
            out << "<th>" << tr("Average Leakage") << "</th>";
        }
    }

    out << "</tr></thead>";
    out << "<tr><th>" << tr("All") << "</th>";
    double value = leakages.weightedAverageValue();
    if (value) {
        out << "<td>" << value << units.at(2) << "</td>";
    } else {
        out << "<td>&nbsp;</td>";
    }

    for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
        QString field = attributeValues().key(n).mid(7);
        if (leakages.containsField(field)) {
            double value = leakages.weightedAverageValue(LeakagesByApplication::Key::All, field);
            if (value) {
                out << "<td>" << value << units.at(2) << "</td>";
            } else {
                out << "<td>&nbsp;</td>";
            }
        }
    }

    out << "</tr>";

    for (int i = 0; i < leakages.usedRefrigerants().count(); ++i) {
        out << "<tr><th>" << leakages.usedRefrigerants().value(i) << "</th>";
        double value = leakages.weightedAverageValue(leakages.usedRefrigerants().key(i));
        if (value) {
            out << "<td>" << value << units.at(2) << "</td>";
        } else {
            out << "<td>&nbsp;</td>";
        }

        for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
            QString field = attributeValues().key(n).mid(7);
            if (leakages.containsField(field)) {
                double value = leakages.weightedAverageValue(leakages.usedRefrigerants().key(i), field);
                if (value) {
                    out << "<td>" << value << units.at(2) << "</td>";
                } else {
                    out << "<td>&nbsp;</td>";
                }
            }
        }

        out << "</tr>";
    }

    out << "<tr></tr></table><br>";

    for (int year = leakages.endYear(); year >= leakages.startYear(); --year) {
        out << "<table><thead><tr><th rowspan=\"2\" width=\"15%\">" << year << "</th>";
        out << "<th colspan=\"21\">" << tr("Fields") << "</th></tr>";
        out << "<tr><th colspan=\"3\">" << tr("All") << "</th>";

        for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
            if (leakages.containsField(attributeValues().key(n).mid(7))) {
                out << "<th colspan=\"3\">" << attributeValues().value(n) << "</th>";
            }
        }

        out << "</tr><tr><th>" << tr("Refrigerant") << "</th>";

        for (int n = attributeValues().indexOfKey("field"); n < attributeValues().count() && attributeValues().key(n).startsWith("field"); ++n) {
            if (attributeValues().key(n) == "field" || leakages.containsField(attributeValues().key(n).mid(7))) {
                out << "<th>" << tr("Added") << "</th>";
                out << "<th>" << tr("In Circuits") << "</th>";
                out << "<th>" << tr("Leakage") << "</th>";
            }
        }

        out << "</tr></thead>";
        out << "<tr><th>" << tr("All") << "</th>";
        for (int t = 0; t < LeakagesByApplication::TableCount; ++t) {
            double value = leakages.value(year).at(t);
            if (value) {
                out << "<td>" << value << units.value(t) << "</td>";
            } else {
                out << "<td>&nbsp;</td>";
            }
        }

        for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
            for (int t = 0; t < LeakagesByApplication::TableCount; ++t) {
                QString field = attributeValues().key(n).mid(7);
                if (leakages.containsField(field)) {
                    double value = leakages.value(year, LeakagesByApplication::Key::All, field).at(t);
                    if (value) {
                        out << "<td>" << value << units.value(t) << "</td>";
                    } else {
                        out << "<td>&nbsp;</td>";
                    }
                }
            }
        }

        out << "</tr>";

        for (int i = 0; i < leakages.usedRefrigerants().count(); ++i) {
            out << "<tr><th>" << leakages.usedRefrigerants().value(i) << "</th>";
            for (int t = 0; t < LeakagesByApplication::TableCount; ++t) {
                double value = leakages.value(year, leakages.usedRefrigerants().key(i)).at(t);
                if (value) {
                    out << "<td>" << value << units.value(t) << "</td>";
                } else {
                    out << "<td>&nbsp;</td>";
                }
            }

            for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
                QString field = attributeValues().key(n).mid(7);
                if (leakages.containsField(field)) {
                    for (int t = 0; t < LeakagesByApplication::TableCount; ++t) {
                        double value = leakages.value(year, leakages.usedRefrigerants().key(i), field).at(t);
                        if (value) {
                            out << "<td>" << value << units.value(t) << "</td>";
                        } else {
                            out << "<td>&nbsp;</td>";
                        }
                    }
                }
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
