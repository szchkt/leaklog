/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2025 Matus & Michal Tomlein

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
#include "global.h"
#include "htmlbuilder.h"
#include "mtwebpage.h"

#include <QBuffer>
#include <QDesktopServices>
#include <QApplication>

AboutWidget::AboutWidget()
{
    setupUi(this);

    MTWebPage *page = new MTWebPage(webv_about);
    page->setLinkDelegationPolicy(MTWebPage::DelegateAllLinks);
    webv_about->setPage(page);

    QObject::connect(btn_about_qt, SIGNAL(clicked()), qApp, SLOT(aboutQt()));
#ifdef Q_OS_MAC
    QObject::connect(btn_about_qt, SIGNAL(clicked()), this, SLOT(close()));
#endif
    QObject::connect(btn_acknowledgements, SIGNAL(clicked(bool)), this, SLOT(showAcknowledgements(bool)));
    QObject::connect(btn_licence, SIGNAL(clicked()), this, SLOT(showLicence()));
    QObject::connect(page, SIGNAL(linkClicked(const QUrl &)), this, SLOT(executeLink(const QUrl &)));

    lbl_version->setText(lbl_version->text().arg(QString(LEAKLOG_VERSION) + (LEAKLOG_PREVIEW_VERSION ? QString("-PREVIEW%1").arg(LEAKLOG_PREVIEW_VERSION) : "")));

    showAcknowledgements(false);

    resize(size() * Global::scaleFactor());
}

void AboutWidget::showAcknowledgements(bool show)
{
    HTMLDocument html_doc(show ? "Acknowledgements" : "About Leaklog");

    HTMLParentElement *style = html_doc.head()->addStyleElement();
#ifdef Q_OS_MAC
    QString font = "\"Lucida Grande\", \"Lucida Sans Unicode\"";
    QString font_size = "8pt";
#else
    QString font = "\"MS Shell Dlg 2\", \"MS Shell Dlg\", \"Lucida Grande\", \"Lucida Sans Unicode\", verdana, lucida, sans-serif";
    QString font_size = "8pt";
#endif
    *style << QString("body { font-family: %1; margin: 0px 12px 12px 12px; } img { margin-right: 10px; }").arg(font);
    *style << QString("h1 { font-size: 13pt; color: #414141; } h2 { font-size: 11pt; color: #414141; } p, a { font-size: %2; color: #9F9F9F; }").arg(font_size);

    if (!show) {
        HTMLParentElement *body = html_doc.body();

        HTMLParagraph *p = body->paragraph();
        *p << "&copy; 2008&nbsp;&ndash;&nbsp;2025 <span style=\"font-style:italic;\">Mat&uacute;&scaron; Tomlein, Michal Tomlein, Peter Tomlein</span>";
        p->newLine();
        *p << tr("Slovak Association for Cooling and Air Conditioning Technology");

        *(body->paragraph()) << tr("Leaklog is a leakage control system based on Regulation (EU) 2024/573. It keeps track of findings and parameters of direct and indirect leakage checks using a log. The result is a history of checks, the development of parameters and their comparison with nominal ones and calculation of the amount and percentage of leakage.");
    } else {
        HTMLParentElement *body = html_doc.body();

        *(body->subHeading()) << tr("Translation");

        *(body->paragraph()) << tr("Czech translation: %1").arg(QString("%1 <i>(<a href=\"http://www.chlazeni.cz\">chlazeni.cz</a>)</i>").arg(tr("Czech Association for Cooling and Air Conditioning Technology")));
        *(body->paragraph()) << tr("Polish translation: %1").arg("PROZON Fundacja Ochrony Klimatu <i>(<a href=\"http://prozon.org.pl\">prozon.org.pl</a>)</i>");
        *(body->paragraph()) << tr("Serbian translation: %1").arg(QString("Srđan Đokić, %1 <i>(<a href=\"https://unijaserviseraklima.rs\">unijaserviseraklima.rs</a>)</i>").arg(tr("Association of HVAC technicians of Serbia")));

        *(body->subHeading()) << tr("Contributors");

        QPixmap frigo_logo(":/images/images/frigo_slovakia_logo.jpg");
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        frigo_logo.save(&buffer, "JPG", 100);
        buffer.close();

        HTMLParagraph *p = body->paragraph();
        *p << "<img style=\"float: left;\" src=\"data:image/jpeg;base64," << buffer.data().toBase64() << "\">";
        *p << "Frigo Slovakia s.r.o. <i>(<a href=\"http://www.frigo.sk\">www.frigo.sk</a>)</i>";

        *body << new HTMLDiv("style=\"clear: both;\"");

        *(body->paragraph()) << "Klimaservis Bratislava, s.r.o. <i>(<a href=\"http://www.klimaservisba.sk\">www.klimaservisba.sk</a>)</i>";

        *(body->subHeading()) << tr("Leaklog uses");

        *(body->paragraph()) << tr("%1, licensed under the GNU LGPL").arg("Oxygen Icons <i>(<a href=\"http://www.oxygen-icons.org\">www.oxygen-icons.org</a>)</i>");

        *(body->paragraph()) << tr("%1, licensed under the GNU GPL").arg("Function Parser v4.5.2 <i>(<a href=\"http://warp.povusers.org/FunctionParser\">warp.povusers.org/FunctionParser</a>)</i>");

#ifdef REFPROP
        *(body->paragraph()) << "NIST Reference Fluid Thermodynamic and Transport Properties Database (REFPROP) v9.1 <i>(<a href=\"http://www.nist.gov/srd/nist23.cfm\">www.nist.gov/srd/nist23.cfm</a>)</i>";
#endif
    }

    webv_about->setHtml(html_doc.html());
    webv_about->setZoomFactor(Global::scaleFactor());
}

void AboutWidget::showLicence()
{
    QDesktopServices::openUrl(QUrl(tr("http://www.gnu.org/licenses/gpl-2.0.html")));
}

void AboutWidget::executeLink(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}
