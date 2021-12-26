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

#include "main.h"
#include "activityeventfilter.h"

#include <QSettings>
#include <QTranslator>
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
#include <QWebEngineUrlScheme>
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_MAC
    QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    QFont::insertSubstitution(".Helvetica Neue DeskInterface", "Helvetica Neue");
    QFont::insertSubstitution(".SF NS Text", "Lucida Grande");
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    QWebEngineUrlScheme::registerScheme(QWebEngineUrlScheme("dbfile"));
    QWebEngineUrlScheme::registerScheme(QWebEngineUrlScheme("view"));
#endif

    MTApplication app(argc, argv);
    app.setApplicationName("Leaklog");
    app.setApplicationVersion(LEAKLOG_VERSION);

#ifdef Q_OS_WIN32
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (QSysInfo::WindowsVersion > QSysInfo::WV_6_1)
        QApplication::setStyle("windowsxp");
#endif
#endif

    QSettings settings("SZCHKT", "Leaklog");
    QString lang = settings.value("lang").toString();
    if (lang.isEmpty()) {
        lang = QLocale::languageToString(QLocale::system().language());
        settings.setValue("lang", lang);
    }
    if (lang == "C") {
        lang = "Slovak";
        settings.setValue("lang", lang);
    }
    if (lang != "English") {
        QTranslator *translator = new QTranslator;
        translator->load(QString(":/i18n/Leaklog-%1.qm").arg(lang.replace(" ", "_")));
        app.installTranslator(translator);
    }
    QLocale locale(QApplication::translate("MainWindow", "en_GB"));
    locale.setNumberOptions(QLocale::OmitGroupSeparator);
    QLocale::setDefault(locale);

    MainWindow *window = new MainWindow;
    app.setAppMainWindow(window);

    ActivityEventFilter *filter = new ActivityEventFilter(&app);
    QObject::connect(filter, SIGNAL(timeout()), window, SLOT(autosave()));
    QObject::connect(filter, SIGNAL(performPeriodicTasks()), window, SLOT(autosync()));
    app.installEventFilter(filter);

    return app.exec();
}
