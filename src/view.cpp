/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2021 Matus & Michal Tomlein

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

#include "view.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "global.h"
#include "htmlbuilder.h"
#include "records.h"
#include "dbfile.h"
#include "mttextstream.h"
#include "mtvariant.h"

#include <QFile>

#define ELLIPSIS_ENABLED 0

using namespace Global;

QMap<QString, QString> View::view_templates;

View::View(ViewTabSettings *settings):
    QObject(settings->object()),
    settings(settings)
{
}

QString View::viewTemplate(const QString &view_template)
{
    if (!view_templates.contains(view_template)) {
        QString style = R"(
            body, td, th {
                font-family: "Lucida Grande", "Segoe UI", "Lucida Sans Unicode", sans-serif;
                font-size: 9pt;
            }
            @media print {
                body {
                    zoom: 0.7;
                }
            }
        )";
        QFile file;
        QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        in.setEncoding(QStringConverter::Utf8);
#else
        in.setCodec("UTF-8");
#endif
        file.setFileName(QString(":/html/%1.html").arg(view_template));
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        view_templates.insert(view_template, in.readAll().arg(style));
        file.close();
    }
    return view_templates.value(view_template);
}

QString View::ellipsis(const QString &text, bool force)
{
    if (ELLIPSIS_ENABLED || force)
        return QString("<div class=\"container\"><div class=\"content\">%1</div><div class=\"spacer\">%1</div><span>&nbsp;</span></div>").arg(escapeString(text));
    return escapeString(text);
}

QString View::ellipsis(const QVariant &text, bool force)
{
    return ellipsis(text.toString(), force);
}

QString View::ellipsis(const MTVariant &text, bool force)
{
    if (ELLIPSIS_ENABLED || force)
        return QString("<div class=\"container\"><div class=\"content\">%1</div><div class=\"spacer\">%1</div><span>&nbsp;</span></div>").arg(text.toHtml());
    return text.toHtml();
}

HTMLTable *View::writeServiceCompany(HTMLTable *table)
{
    ServiceCompany serv_company(settings->selectedServiceCompanyUUID());
    if (!table) table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"");
    table->addClass("service_company");
    HTMLTableRow *_tr = table->addRow();
    HTMLTableCell *_td;
    if (!serv_company.imageFileUUID().isEmpty()) {
        QByteArray byte_array = DBFile(serv_company.imageFileUUID()).data().toBase64();
        if (!byte_array.isNull()) {
            _td = _tr->addCell("rowspan=\"3\" width=\"5%\"");
            *_td << "<img src=\"data:image/jpeg;base64," << byte_array << "\" style=\"max-width: 300px;\">";
        }
    }
    _td = _tr->addHeaderCell("colspan=\"6\" style=\"background-color: #DFDFDF; font-size: medium; width:100%; text-align: center;\"");
    *(_td->link("servicecompany:" + serv_company.uuid() + "/edit")) << Global::escapeString(serv_company.name());
    _tr = table->addRow();
    for (int n = 0; n < ServiceCompany::attributes().count(); ++n) {
        if (ServiceCompany::attributes().key(n) == "name")
            continue;
        if (serv_company.value(ServiceCompany::attributes().key(n)).toString().isEmpty())
            continue;
        _td = _tr->addHeaderCell();
        QString attr = ServiceCompany::attributes().value(n);
        attr.chop(1);
        *_td << attr;
    }
    _tr = table->addRow();
    for (int n = 0; n < ServiceCompany::attributes().count(); ++n) {
        if (ServiceCompany::attributes().key(n) == "name")
            continue;
        QString attr_value = ServiceCompany::attributes().key(n);
        if (serv_company.value(attr_value).toString().isEmpty())
            continue;
        _td = _tr->addCell();
        *_td << MTVariant(serv_company.value(attr_value), attr_value);
    }
    return table;
}

void View::writeServiceCompany(MTTextStream &out)
{
    if (!settings->mainWindowSettings().serviceCompanyInformationVisible()) {
        if (!settings->mainWindowSettings().serviceCompanyInformationPrinted())
            return;
        out << "<div class=\"print_only\">";
    }

    HTMLTable *service_company = writeServiceCompany();
    out << service_company->html();
    delete service_company;
    out << "<br>";

    if (!settings->mainWindowSettings().serviceCompanyInformationVisible()) {
        out << "</div>";
    }
}

void View::writeServiceCompany(HTMLParent &div)
{
    HTMLParent *service_company_parent;

    if (settings->mainWindowSettings().serviceCompanyInformationVisible()) {
        service_company_parent = &div;
    } else if (settings->mainWindowSettings().serviceCompanyInformationPrinted()) {
        HTMLDiv *service_company_div = new HTMLDiv;
        service_company_div->addClass("print_only");
        div << service_company_div;
        service_company_parent = service_company_div;
    } else {
        return;
    }

    *service_company_parent << writeServiceCompany();
    service_company_parent->newLine();
}
