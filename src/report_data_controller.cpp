/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2011 Matus & Michal Tomlein

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

#include "report_data_controller.h"
#include "report_data.h"
#include "navigation.h"
#include "records.h"
#include "defs.h"

#include <QTextStream>
#include <QWebView>
#include <QWebFrame>
#include <QDate>

ReportDataController::ReportDataController(QWebView * wv, Navigation * parent):
QObject(parent), navigation(parent) {
    wv_main = wv;
    navigation->setReportDataGroupBoxVisible(true);
    navigation->autofillButton()->setEnabled(false);
    navigation->reportYearLabel()->setText(tr("Report year: %1").arg(QDate::currentDate().year() - 1));
    navigation->reportDataProgressBar()->setVisible(false);
    QObject::connect(navigation->autofillButton(), SIGNAL(clicked()), this, SLOT(autofill()));
    QObject::connect(navigation->doneButton(), SIGNAL(clicked()), this, SLOT(done()));
    wv_main->setPage(new QWebPage(wv_main));
    QObject::connect(wv_main, SIGNAL(loadProgress(int)), this, SLOT(updateProgressBar(int)));
    QObject::connect(wv_main, SIGNAL(loadFinished(bool)), this, SLOT(enableAutofill()));
    //: URL to the data report system of the notified body
    wv_main->load(QUrl::QUrl(tr("http://szchkt.org/cert/")));
}

int ReportDataController::currentReportYear()
{
    int year = QDate::currentDate().year() - 1;
    QVariant variant = wv_main->page()->mainFrame()->evaluateJavaScript("currentReportYear();");
    if (!variant.toString().isEmpty())
        year = variant.toInt();

    return year;
}

void ReportDataController::updateProgressBar(int progress)
{
    navigation->reportDataProgressBar()->setVisible(true);
    navigation->reportDataProgressBar()->setValue(progress);
}

void ReportDataController::enableAutofill()
{
    QVariant version_required = wv_main->page()->mainFrame()->evaluateJavaScript("minimumLeaklogVersionRequired();");
    bool valid = !version_required.toString().isEmpty();
    bool sufficient = version_required.toDouble() <= F_LEAKLOG_VERSION;
    navigation->autofillButton()->setEnabled(valid && sufficient);
    navigation->autofillButton()->setToolTip((valid && !sufficient) ? tr("A newer version of Leaklog is required.") : QString());
    if (valid) navigation->reportYearLabel()->setText(tr("Report year: %1").arg(currentReportYear()));
    navigation->reportDataProgressBar()->setVisible(false);
}

void ReportDataController::autofill()
{
    emit processing(true);
    qApp->processEvents();

    int year = currentReportYear();

    QString js; QTextStream out(&js);
    out << "clearAll();" << endl;

    ListOfVariantMaps inspectors(Inspector("").listAll("id"));
    for (ListOfVariantMaps::const_iterator i = inspectors.constBegin(); i != inspectors.constEnd(); ++i) {
        out << "addEmployee({ \"certification_num\": \"" << i->value("id").toString().rightJustified(4, '0') << "\" });" << endl;
    }

    ReportData data(year);
    QMap<QString, QVector<double> *>::const_iterator sums_iterator = data.sums_map.constFind(QString::number(year));
    QVector<double> * sum_list = NULL;
    QString refrigerant;
    QStringList sums_fieldnames;
    sums_fieldnames << "purchased"
                    << "purchased_reco"
                    << "sold"
                    << "sold_reco"
                    << "new_charge"
                    << "refr_add_am"
                    << "refr_reco"
                    << "refr_rege"
                    << "refr_disp"
                    << "leaked"
                    << "leaked_reco";
    if (++sums_iterator != data.sums_map.constEnd()) {
        while (sums_iterator != data.sums_map.constEnd() && (sum_list = sums_iterator.value())) {
            refrigerant = sums_iterator.key().split("::").last();
            if (!refrigerant.isEmpty()) {
                out << "addRefrigerantManagementEntry({" << endl;
                out << "\t\"refrigerant\": \"" << refrigerant << "\"," << endl;
                for (int j = 0; j < sums_fieldnames.count(); ++j) {
                    out << "\t\"" << sums_fieldnames.at(j) << "\": " << sum_list->at(j);
                    if (j == sums_fieldnames.count() - 1) out << endl; else out << "," << endl;
                }
                out << "});" << endl;
            }
            ++sums_iterator;
        }
    }

    wv_main->page()->mainFrame()->evaluateJavaScript(js);
    emit processing(false);
}

void ReportDataController::done()
{
    navigation->setReportDataGroupBoxVisible(false);
    deleteLater();
}
