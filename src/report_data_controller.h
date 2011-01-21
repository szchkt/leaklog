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

#ifndef REPORT_DATA_CONTROLLER_H
#define REPORT_DATA_CONTROLLER_H

#include <QObject>

class Navigation;
class QWebView;

class ReportDataController : public QObject
{
    Q_OBJECT

public:
    ReportDataController(QWebView *, Navigation *);

private slots:
    void updateProgressBar(int);
    void enableAutofill();
    void autofill();
    void done();

signals:
    void processing(bool);

protected:
    int currentReportYear();

private:
    QWebView * wv_main;
    Navigation * navigation;
};

#endif // REPORT_DATA_CONTROLLER_H
