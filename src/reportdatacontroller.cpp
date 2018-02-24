/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2017 Matus & Michal Tomlein

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

#include "reportdatacontroller.h"
#include "reportdata.h"
#include "leakagesbyapplication.h"
#include "toolbarstack.h"
#include "records.h"
#include "defs.h"
#include "global.h"

#include <QTextStream>
#include <QDate>
#include <QMessageBox>
#include <QtWebEngine/QtWebEngine>
#include <QtWebEngineWidgets/QtWebEngineWidgets>

using namespace Global;

ReportDataController::ReportDataController(QWebEngineView *wv, ToolBarStack *parent):
QObject(parent), toolbarstack(parent) {
    wv_main = wv;
    toolbarstack->setReportDataGroupBoxVisible(true);
    toolbarstack->autofillButton()->setEnabled(false);
    toolbarstack->autofillButton()->setStyleSheet("QToolButton:enabled { background-color: red; color: white; }");
    toolbarstack->reportYearLabel()->setText(tr("Report year: %1").arg(QDate::currentDate().year() - 1));
    toolbarstack->reportDataProgressBar()->setVisible(false);
    QObject::connect(toolbarstack->autofillButton(), SIGNAL(clicked()), this, SLOT(autofill()));
    QObject::connect(toolbarstack->doneButton(), SIGNAL(clicked()), this, SLOT(done()));
    QWebEnginePage *page = new QWebEnginePage(wv_main);
    page->profile()->setHttpUserAgent(page->profile()->httpUserAgent() + QString(" Leaklog/%1").arg(LEAKLOG_VERSION));
    wv_main->setPage(page);
    wv_main->setZoomFactor(Global::scaleFactor());
    QObject::connect(wv_main, SIGNAL(loadProgress(int)), this, SLOT(updateProgressBar(int)));
    QObject::connect(wv_main, SIGNAL(loadFinished(bool)), this, SLOT(enableAutofill()));
    //: URL to the data report system of the notified body
    wv_main->load(QUrl(tr("https://szchkt.org/report_data/")));
}

void ReportDataController::currentReportYear(std::function<void(int)> callback)
{
    wv_main->page()->runJavaScript("currentReportYear();", [callback](const QVariant &result) {
        int year = QDate::currentDate().year() - 1;
        if (!result.toString().isEmpty())
            year = result.toInt();
        callback(year);
    });
}

void ReportDataController::canImportLeakagesByApplication(std::function<void(bool)> callback)
{
    wv_main->page()->runJavaScript("canImportLeakagesByApplication();", [callback](const QVariant &result) {
        callback(result.toBool());
    });
}

void ReportDataController::updateProgressBar(int progress)
{
    toolbarstack->reportDataProgressBar()->setVisible(true);
    toolbarstack->reportDataProgressBar()->setValue(progress);
}

void ReportDataController::enableAutofill()
{
    wv_main->page()->runJavaScript(QString("minimumLeaklogVersionRequired(%1);").arg(F_LEAKLOG_VERSION), [this](const QVariant &version_required){
        bool valid = !version_required.toString().isEmpty();
        bool sufficient = version_required.toDouble() <= F_LEAKLOG_VERSION;
        toolbarstack->autofillButton()->setEnabled(valid && sufficient);

        if (valid) {
            if (!sufficient) {
                QMessageBox::warning((QWidget *)parent(), "Leaklog", tr("A newer version of Leaklog is required."));
            } else {
                canImportLeakagesByApplication([this](bool result) {
                    if (result) {
                        toolbarstack->reportYearLabel()->setText(tr("Report year: %1").arg(tr("all")));
                    } else {
                        currentReportYear([this](int year) {
                            toolbarstack->reportYearLabel()->setText(tr("Report year: %1").arg(year));
                        });
                    }
                });
            }
        }

        toolbarstack->reportDataProgressBar()->setVisible(false);
    });
}

void ReportDataController::autofill()
{
    emit processing(true);
    QApplication::processEvents();

    canImportLeakagesByApplication([this](bool result) {
        if (result) {
            reportLeakages();
        } else {
            currentReportYear([this](int year) {
                reportData(year);
            });
        }
    });

    emit processing(false);
}

void ReportDataController::reportData(int year)
{
    QString js; QTextStream out(&js);
    out << "clearAll();" << endl;

    ListOfVariantMaps inspectors(Inspector::query().listAll("id"));
    for (ListOfVariantMaps::const_iterator i = inspectors.constBegin(); i != inspectors.constEnd(); ++i) {
        out << "addEmployee({ \"certification_num\": \"" << i->value("id").toString().rightJustified(4, '0') << "\" });" << endl;
    }

    QSet<QString> refrigerants_by_field;
    refrigerants_by_field << "R134a" << "R404A" << "R407C" << "R410A";

    ReportData data(year, QString(), true, refrigerants_by_field);
    QMap<QString, QVector<double> *>::const_iterator sums_iterator = data.sums_map.constFind(QString::number(year));
    QVector<double> *sum_list = NULL;
    QString refrigerant;

    MTDictionary refr_man_fieldnames;
    refr_man_fieldnames.insert(QString::number(SUMS::PURCHASED), "purchased");
    refr_man_fieldnames.insert(QString::number(SUMS::PURCHASED_RECO), "purchased_reco");
    refr_man_fieldnames.insert(QString::number(SUMS::SOLD), "sold");
    refr_man_fieldnames.insert(QString::number(SUMS::SOLD_RECO), "sold_reco");
    refr_man_fieldnames.insert(QString::number(SUMS::REFR_REGE), "refr_rege");
    refr_man_fieldnames.insert(QString::number(SUMS::REFR_DISP), "refr_disp");
    refr_man_fieldnames.insert(QString::number(SUMS::LEAKED), "leaked");
    refr_man_fieldnames.insert(QString::number(SUMS::LEAKED_RECO), "leaked_reco");

    MTDictionary refr_use_fieldnames;
    refr_use_fieldnames.insert(QString::number(SUMS::NEW_CHARGE), "new_charge");
    refr_use_fieldnames.insert(QString::number(SUMS::REFR_ADD_AM), "refr_add_am");
    refr_use_fieldnames.insert(QString::number(SUMS::REFR_RECO), "refr_reco");

    if (++sums_iterator != data.sums_map.constEnd()) {
        while (sums_iterator != data.sums_map.constEnd() && (sum_list = sums_iterator.value())) {
            refrigerant = sums_iterator.key().split("::").last();
            if (!refrigerant.isEmpty()) {
                bool add = false;
                for (int j = 0; j < refr_man_fieldnames.count(); ++j) {
                    if (sum_list->at(refr_man_fieldnames.key(j).toInt()) != 0.0) {
                        add = true;
                        break;
                    }
                }
                if (add) {
                    out << "addRefrigerantManagementEntry({" << endl;
                    out << "\t\"refrigerant\": \"" << refrigerant.split(':').first() << ":0\"," << endl;
                    for (int j = 0; j < refr_man_fieldnames.count(); ++j) {
                        out << "\t\"" << refr_man_fieldnames.value(j) << "\": ";
                        out << sum_list->at(refr_man_fieldnames.key(j).toInt());
                        if (j == refr_man_fieldnames.count() - 1) out << endl; else out << "," << endl;
                    }
                    out << "});" << endl;
                }

                add = false;
                for (int j = 0; j < refr_use_fieldnames.count(); ++j) {
                    if (sum_list->at(refr_use_fieldnames.key(j).toInt()) != 0.0) {
                        add = true;
                        break;
                    }
                }
                if (add) {
                    out << "addRefrigerantUseEntry({" << endl;
                    out << "\t\"refrigerant\": \"" << refrigerant << "\"," << endl;
                    for (int j = 0; j < refr_use_fieldnames.count(); ++j) {
                        out << "\t\"" << refr_use_fieldnames.value(j) << "\": ";
                        out << sum_list->at(refr_use_fieldnames.key(j).toInt());
                        if (j == refr_use_fieldnames.count() - 1) out << endl; else out << "," << endl;
                    }
                    out << "});" << endl;
                }
            }
            ++sums_iterator;
        }
    }

    wv_main->page()->runJavaScript(js);
}

void ReportDataController::reportLeakages()
{
    QString js; QTextStream out(&js);

    LeakagesByApplication leakages(false);

    QStringList tables;
    tables << "refr_add_am" << "refrigerant_amount" << "refr_add_per";

    Q_ASSERT(tables.count() == LeakagesByApplication::TableCount);

    out << "importLeakagesByApplication({" << endl;

    for (int year = leakages.startYear(); year <= leakages.endYear(); ++year) {
        if (year != leakages.startYear())
            out << "," << endl;

        out << "\t" << year << ": {" << endl;

        bool first_refrigerant = true;
        foreach (const QString &refrigerant, leakages.usedRefrigerants().keys()) {
            if (first_refrigerant)
                first_refrigerant = false;
            else
                out << "," << endl;

            out << "\t\t'" << refrigerant << "': {" << endl;

            for (int t = 0; t < tables.count(); ++t) {
                if (t)
                    out << "," << endl;

                out << "\t\t\t'" << tables.at(t) << "': { ";

                QStringList values;
                for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
                    QString field = attributeValues().key(n).remove("field::");
                    int field_id = fieldOfApplicationToId(field);
                    values << QString("%1: %2").arg(field_id)
                              .arg(leakages.value(year, refrigerant, field).at(t));
                }

                out << values.join(", ") << " }";
            }

            out << endl << "\t\t}";
        }

        out << endl << "\t}";
    }

    out << endl << "});" << endl;

    wv_main->page()->runJavaScript(js);
}

void ReportDataController::done()
{
    toolbarstack->setReportDataGroupBoxVisible(false);
    deleteLater();
}
