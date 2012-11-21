/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2012 Matus & Michal Tomlein

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

#include "aboutwidget.h"
#include "defs.h"
#include "htmlbuilder.h"

#include <QBuffer>

AboutWidget::AboutWidget()
{
    setupUi(this);
    QObject::connect(btn_close, SIGNAL(clicked()), this, SLOT(close()));

    HTMLDocument html_doc("About Leaklog");

    HTMLParentElement * style = html_doc.head()->addStyleElement();
#ifdef Q_WS_MAC
    QString font = "\"Lucida Grande\", \"Lucida Sans Unicode\"";
    QString font_size = "9pt";
#else
    QString font = "\"MS Shell Dlg 2\", \"MS Shell Dlg\", \"Lucida Grande\", \"Lucida Sans Unicode\", verdana, lucida, sans-serif";
    QString font_size = "small";
#endif
    *style << QString("body { font-family: %1; } img { margin-right: 10px; }").arg(font);
    *style << QString("h1 { font-size: 13pt; } h2 { font-size: 11pt; } p { font-size: %2; }").arg(font_size);

    HTMLParentElement * body = html_doc.body();

    *(body->heading()) << "Leaklog";
    *(body->subHeading()) << tr("Version")
                             << QString(" %1").arg(LEAKLOG_VERSION)
                             << (LEAKLOG_PREVIEW_VERSION ? QString("-PREVIEW%1").arg(LEAKLOG_PREVIEW_VERSION) : "");

    *(body->paragraph()) << tr("Leaklog is a leakage control system based on the EU Regulation No 842/2006. It keeps track of findings and parameters of direct and indirect leakage checks using a log. The result is a history of checks, the development of parameters and their comparison with nominal ones and calculation of the amount and percentage of leakage.");

    *(body->paragraph()) << tr("This program is distributed under the terms of the GPL v2.");

    HTMLParagraph * p = body->paragraph();
    *p << "Copyright (C) 2008-2012 <span style=\"font-style:italic;\">Matus Tomlein, Michal Tomlein, Peter Tomlein</span>";
    p->newLine();
    *p << tr("Slovak Association for Cooling and Air Conditioning Technology");

    *(body->paragraph()) << tr("The program is provided AS IS with ABSOLUTELY NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.");

    *(body->subHeading()) << tr("List of contributors:");

    QPixmap frigo_logo(":/images/images/frigo_slovakia_logo.jpg");
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    frigo_logo.save(&buffer, "JPG", 100);
    buffer.close();

    p = body->paragraph();
    *p << "<img style=\"float: left;\" src=\"data:image/jpeg;base64," << buffer.data().toBase64() << "\">";
    HTMLDiv * div = new HTMLDiv("style=\"padding: 10px;\"");
    *div << "Frigo Slovakia s.r.o., <i>http://www.frigo.sk/</i>";
    *p << div;

    *(body->paragraph()) << "Klimaservis Bratislava, s.r.o., <i>http://www.klimaservisba.sk/</i>";

    webv_about->setHtml(html_doc.html());

    // +++ ABOUT QT +++

    HTMLDocument html_qt_doc("About Qt");
    style = html_qt_doc.head()->addStyleElement();
    *style << QString("body { font-family: %1; } img { margin-right: 10px; }").arg(font);
    *style << QString("h1 { font-size: 13pt; } h2 { font-size: 11pt; } p { font-size: %2; }").arg(font_size);

    body = html_qt_doc.body();
    *(body->heading()) << tr("About Qt");

    *(body->paragraph("style=\"font-style:italic;\"")) << tr("This program uses Qt Open Source Edition version %1.").arg(qVersion());
    *(body->paragraph()) << tr("Qt is a C++ toolkit for cross-platform application development.");
    *(body->paragraph()) << tr("Qt provides single-source portability across MS Windows, Mac OS X, Linux, and all major commercial Unix variants. Qt is also available for embedded devices as Qt for Embedded Linux and Qt for Windows CE.");
    *(body->paragraph()) << tr("Qt is a Nokia product. See <span style=\"font-style:italic;\">qt.nokia.com</span> for more information.");

    webv_about_qt->setHtml(html_qt_doc.html());
}
