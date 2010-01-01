/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2010 Matus & Michal Tomlein

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

#include "about_widget.h"
#include "defs.h"

AboutWidget::AboutWidget()
{
    setupUi(this);
    QObject::connect(btn_close, SIGNAL(released()), this, SLOT(close()));
    QString about = "<p style=\"font-family: sans-serif; font-style:italic;\"><span style=\"font-size:12pt;\">Leaklog</span><br>";
    about.append("<span style=\"font-size:8pt;\">");
    about.append(tr("Version"));
    about.append(QString(" %1%2</span></p><p></p>").arg(LEAKLOG_VERSION).arg(LEAKLOG_PREVIEW_VERSION ? QString("-PREVIEW%1").arg(LEAKLOG_PREVIEW_VERSION) : ""));
    about.append("<p style=\"font-family: sans-serif; font-size:8pt;\">");
    about.append(tr("Leaklog is a leakage control system based on the EU Regulation No 842/2006. It keeps track of findings and parameters of direct and indirect leakage checks using a log. The result is a history of checks, the development of parameters and their comparison with nominal ones and calculation of the amount and percentage of leakage."));
    about.append("</p><p></p>");
    about.append("<p style=\"font-family: sans-serif; font-size:8pt;\">");
    about.append(tr("This program is distributed under the terms of the GPL v2."));
    about.append("</p><p></p>");
    about.append("<p style=\"font-family: sans-serif; font-size:8pt;\">Copyright (C) 2008-2010 <span style=\"font-style:italic;\">Matus & Michal Tomlein</span></p>");
    about.append("<p></p><p style=\"font-family: sans-serif; font-size:8pt;\">");
    about.append(tr("The program is provided AS IS with ABSOLUTELY NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE."));
    about.append("</p>");
#ifdef Q_WS_MAC
    about.remove("font-family: sans-serif;");
    about.replace("font-size:12pt;", "font-size:14pt;");
    about.replace("font-size:8pt;", "font-size:10pt;");
#endif
    txb_about->setHtml(about);
    QString about_qt = "<p style=\"font-family: sans-serif; font-style:italic;\"><span style=\"font-size:12pt;\">";
    about_qt.append(tr("About Qt"));
    about_qt.append("</span></p><p></p><p style=\"font-family: sans-serif; font-size:8pt; font-style:italic;\">");
    about_qt.append(tr("This program uses Qt Open Source Edition version %1.").arg(qVersion()));
    about_qt.append("</p><p></p><p style=\"font-family: sans-serif; font-size:8pt;\">");
    about_qt.append(tr("Qt is a C++ toolkit for cross-platform application development."));
    about_qt.append("</p><p></p><p style=\"font-family: sans-serif; font-size:8pt;\">");
    about_qt.append(tr("Qt provides single-source portability across MS Windows, Mac OS X, Linux and all major commercial Unix variants. Qt is also available for embedded devices as Qt for Embedded Linux and Qt for Windows CE."));
    about_qt.append("</p><p></p><p style=\"font-family: sans-serif; font-size:8pt;\">");
    about_qt.append(tr("Qt is a Nokia product. See <span style=\"font-style:italic;\">www.qtsoftware.com/qt</span> for more information."));
    about_qt.append("</p>");
#ifdef Q_WS_MAC
    about_qt.remove("font-family: sans-serif;");
    about_qt.replace("font-size:12pt;", "font-size:14pt;");
    about_qt.replace("font-size:8pt;", "font-size:10pt;");
#endif
    txb_about_qt->setHtml(about_qt);
}
