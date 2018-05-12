/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2018 Matus & Michal Tomlein

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

#ifndef REPORT_DATA_CONTROLLER_H
#define REPORT_DATA_CONTROLLER_H

#include <functional>

#include <QObject>

class ToolBarStack;
class QWebEngineView;

class ReportDataController : public QObject
{
    Q_OBJECT

public:
    ReportDataController(QWebEngineView *, ToolBarStack *);

private slots:
    void updateProgressBar(int);
    void enableAutofill();
    void autofill();
    void done();

signals:
    void processing(bool);

protected:
    void currentReportYear(std::function<void(int)> callback);
    void canImportLeakagesByApplication(std::function<void(bool)> callback);

    void reportData(int year);
    void reportLeakages();

private:
    QWebEngineView *wv_main;
    ToolBarStack *toolbarstack;
};

#endif // REPORT_DATA_CONTROLLER_H
