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

#include "view.h"
#include "viewtabsettings.h"
#include "mainwindow.h"
#include "global.h"
#include "variables.h"
#include "warnings.h"
#include "dbfile.h"
#include "reportdata.h"
#include "mttextstream.h"
#include "mtaddress.h"
#include "htmlbuilder.h"
#include "variableevaluation.h"

#include <QDate>
#include <QSqlError>
#include <QFileInfo>
#include <QWebView>

using namespace Global;

QMap<QString, QString> View::view_templates;

View::View(ViewTabSettings *settings):
    QObject(settings->object()),
    settings(settings)
{
}

QString View::viewTemplate(const QString & view_template)
{
    if (!view_templates.contains(view_template)) {
#ifdef Q_OS_MAC
        QString font = "\"Lucida Grande\", \"Lucida Sans Unicode\"";
        QString font_size = "9pt";
#else
        QString font = "\"MS Shell Dlg 2\", \"MS Shell Dlg\", \"Lucida Grande\", \"Lucida Sans Unicode\", verdana, lucida, sans-serif";
        QString font_size = "small";
#endif
        QFile file;
        QTextStream in(&file);
        in.setCodec("UTF-8");
        file.setFileName(QString(":/html/%1.html").arg(view_template));
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        view_templates.insert(view_template, in.readAll().arg(font).arg(font_size));
        file.close();
    }
    return view_templates.value(view_template);
}

QString MainWindow::fileNameForCurrentView()
{
    QString title;
    if (current_tab->currentView() != View::ViewCount)
        title = current_tab->view(current_tab->currentView())->title();

    if (current_tab->currentView() == View::AssemblyRecordDetails)
        return title;

    return QString("%1 - %2")
            .arg(QFileInfo(QSqlDatabase::database().databaseName()).baseName())
            .arg(title.replace(':', '.'));
}
