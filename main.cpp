/******************************************************************************
 *                                  Leaklog                                   *
 * -------------------------------------------------------------------------- *
 * Version:      0.9.3                                                        *
 * Qt version:   4.4.0 or higher required                                     *
 * -------------------------------------------------------------------------- *
 * Leaklog is a leakage control system based on the EU Regulation             *
 * No 842/2006. It keeps track of findings and parameters of direct and       *
 * indirect leakage checks using a log. The result is a history of checks,    *
 * the development of parameters and their comparison with nominal ones and   *
 * calculation of the amount and percentage of leakage.                       *
 * -------------------------------------------------------------------------- *
 * Leaklog is distributed under the terms of the GPL v2, see details below.   *
 ******************************************************************************/

#include <QApplication>

#include "main_window.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	QSettings settings("SZCHKT", "Leaklog");
	QString lang = settings.value("lang").toString();
	if (lang.isEmpty()) {
		//lang = QLocale::languageToString(QLocale::system().language());
        lang = "Slovak";
		settings.setValue("lang", lang);
	}
	if (lang == "C") { lang = "Slovak"; settings.setValue("lang", lang); }
	if (lang != "English") {
		QTranslator * translator = new QTranslator;
		translator->load(QString(":/i18n/Leaklog-%1.qm").arg(lang.replace(" ", "_")));
		app.installTranslator(translator);
	}

	MainWindow * leaklog_window = new MainWindow;
	leaklog_window->show();
	return app.exec();
}

/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2009 Matus & Michal Tomlein

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
