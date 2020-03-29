/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2020 Matus & Michal Tomlein

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

#ifndef VIEWTAB_H
#define VIEWTAB_H

#include "viewtabsettings.h"
#include "view.h"
#include "mtwidget.h"
#include "linkparser.h"

#include <QWebEngineUrlSchemeHandler>

class ToolBarStack;
class QTreeWidgetItem;
class QWebEngineView;
class QUrl;

class ViewUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT

public:
    ViewUrlSchemeHandler(View *views[View::ViewCount], QObject *parent);

    void requestStarted(QWebEngineUrlRequestJob *job);

private:
    View *views[View::ViewCount];
};

namespace Ui {
class ViewTab;
}

class ViewTab : public MTWidget, public ViewTabSettings
{
    Q_OBJECT

public:
    explicit ViewTab(QWidget *parent = 0);
    ~ViewTab();
    QObject *object() { return this; }
    void connectSlots(QObject *);

    void scaleFactorChanged();

    void enableAllTools();
    void enableTools();

    MainWindowSettings &mainWindowSettings();

    ToolBarStack *toolBarStack() const;
    QWebEngineView *webView() const;

    View *view(View::ViewID view) { return views[view]; }
    View::ViewID currentView() const;
    QString currentViewTitle() const;
    QString currentTableUUID() const;

    bool isShowDateUpdatedChecked() const;
    bool isShowOwnerChecked() const;
    bool isShowNotesChecked() const;
    bool isShowLeakedChecked() const;
    bool isShowPlaceOfOperationChecked() const;
    bool isShowBuildingChecked() const;
    bool isShowDeviceChecked() const;
    bool isShowManufacturerChecked() const;
    bool isShowTypeChecked() const;
    bool isShowSerialNumberChecked() const;
    bool isShowYearOfPurchaseChecked() const;
    bool isShowDateOfCommissioningChecked() const;
    bool isShowFieldOfApplicationChecked() const;
    bool isShowOilChecked() const;
    bool isShowMostRecentFirstChecked() const;
    bool isCompareValuesChecked() const;
    bool isPrinterFriendlyVersionChecked() const;

    QString appendDefaultOrderToColumn(const QString &column) const;

    bool isServiceCompanySelected() const;
    QString filterServiceCompanyUUID() const;
    QString selectedServiceCompanyUUID() const;
    void setSelectedServiceCompanyUUID(const QString &service_company_uuid);

public slots:
    void setView(View::ViewID view, const QString &table = QString());
    void setView(int view, const QString &table = QString());
    void refreshView();
    void refreshViewIfNeeded();

    void reportData();
    void reportDataFinished();

signals:
    void tabTextChanged(QWidget *, const QString &);

    void viewChanged(View::ViewID);

private slots:
    void setNeedsRefresh();

    void reloadTables(const MTDictionary &tables);
    void addTable(int index, const QString &uuid, const QString &name);
    void removeTable(const QString &uuid);

    void viewChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void executeLink(const QUrl &);
    void executeLink(Link *link);

private:
    void createViewItems();
    void formatGroupItem(QTreeWidgetItem *item);

    void setDefaultWebPage();

    Ui::ViewTab *ui;
    View *views[View::ViewCount];
    ViewUrlSchemeHandler *view_handler;
    QTreeWidgetItem *group_tables;
    QTreeWidgetItem *view_items[View::ViewCount];
    bool needs_refresh;
};

#endif // VIEWTAB_H
