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

#include "viewtab.h"
#include "ui_viewtab.h"

#include "mainwindow.h"
#include "storeview.h"
#include "refrigerantmanagementview.h"
#include "customersview.h"
#include "circuitsview.h"
#include "inspectionsview.h"
#include "inspectiondetailsview.h"
#include "tableview.h"
#include "repairsview.h"
#include "inspectorsview.h"
#include "inspectordetailsview.h"
#include "operatorreportview.h"
#include "leakagesbyapplicationview.h"
#include "agendaview.h"
#include "inspectionimagesview.h"
#include "assemblyrecordsview.h"
#include "assemblyrecorddetailsview.h"
#include "assemblyrecordtypesview.h"
#include "assemblyrecorditemsview.h"
#include "circuitunittypesview.h"
#include "mtwebpage.h"
#include "reportdatacontroller.h"
#include "records.h"
#include "dbfile.h"
#include "global.h"

#include <QBuffer>
#include <QDesktopServices>
#include <QWebEngineProfile>
#include <QWebEngineUrlRequestJob>

class ViewBuffer : public QBuffer
{
public:
    ViewBuffer(QByteArray *buf, QObject *parent): QBuffer(buf, parent), _buffer(buf) {}

    ~ViewBuffer() {
        delete _buffer;
    }

private:
    QByteArray *_buffer;
};

ViewUrlSchemeHandler::ViewUrlSchemeHandler(View *views[View::ViewCount], QObject *parent):
    QWebEngineUrlSchemeHandler(parent)
{
    for (int i = 0; i < View::ViewCount; ++i) {
        this->views[i] = views[i];
    }
}

void ViewUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *job)
{
    int view = job->requestUrl().host().mid(1).toInt();
    if (view >= 0 && view < View::ViewCount) {
        QByteArray *byte_array = new QByteArray(views[view]->renderHTML().toUtf8());
        ViewBuffer *buffer = new ViewBuffer(byte_array, job);
        job->reply(QString("text/html").toUtf8(), buffer);
    } else {
        job->fail(QWebEngineUrlRequestJob::UrlNotFound);
    }
}

ViewTab::ViewTab(QWidget *parent):
    MTWidget(parent),
    ui(new Ui::ViewTab),
    needs_refresh(false)
{
    ui->setupUi(this);

    QObject::connect(parentWindow(), SIGNAL(tablesChanged(const MTDictionary &)), this, SLOT(reloadTables(const MTDictionary &)));
    QObject::connect(parentWindow(), SIGNAL(tableAdded(int, const QString &, const QString &)), this, SLOT(addTable(int, const QString &, const QString &)));
    QObject::connect(parentWindow(), SIGNAL(tableRemoved(const QString &)), this, SLOT(removeTable(const QString &)));
    QObject::connect(ui->trw_navigation, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
                     this, SLOT(viewChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    QObject::connect(parentWindow(), SIGNAL(serviceCompaniesChanged()), ui->toolbarstack, SLOT(loadServiceCompanies()));
    QObject::connect(this, SIGNAL(viewChanged(View::ViewID)),
                     ui->toolbarstack, SLOT(viewChanged(View::ViewID)));

    ui->splitter->setStyleSheet("QSplitter { background-color: #B8B8B8; }");

    ui->trw_navigation->setIconSize(QSize(20, 20));

    createViewItems();
    setDefaultWebPage();

    scaleFactorChanged();

    for (int i = 0; i < ui->trw_navigation->topLevelItemCount(); ++i)
        ui->trw_navigation->topLevelItem(i)->setExpanded(true);

    ui->toolbarstack->setSettings(this);
}

ViewTab::~ViewTab()
{
    delete ui;
}

void ViewTab::scaleFactorChanged()
{
    double scale = Global::scaleFactor();

    ui->trw_navigation->setMinimumWidth(180 * scale);
    ui->splitter->setSizes(QList<int>() << 10 << 1000);

    QString style = QString("QTreeWidget::item { padding-top: %1px; padding-bottom: %1px; }").arg(2 * scale);
#ifdef Q_OS_MAC
    style += R"(
        QTreeWidget { background-color: #EEEEEE; }
        QTreeWidget:!active { background-color: #F6F6F6; }
        QTreeWidget::item:!has-children:!selected:!disabled { color: #3D3D50; }
        QTreeWidget::item:!has-children:!selected:disabled { color: #777777; }
        QTreeWidget::item:selected { background-color: #CECECE; color: #281C28; }
        QTreeWidget::item:selected:!active { background-color: #CDCDCD; color: #281C28; }
        QTreeWidget::item:!selected { background-color: transparent; }
        QTreeWidget::item:!selected:!active { background-color: transparent; }
        QTreeWidget::item:has-children { padding-left: 3px; color: #777777; }
        QTreeWidget::item:has-children:!active { color: #777777; }
    )";
#else
    style += QString("QTreeWidget { background-color: white; }");
    style += QString("QTreeWidget::item:selected { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #DAECFC, stop: 1 #C4E0FC);"
                                                  "border-color: #569DE5; border-style: solid; border-width: 1px; }");
    style += QString("QTreeWidget::item:!selected { background-color: white; }");
    style += QString("QTreeWidget::item:selected:disabled { background-color: #F7F7F7;"
                                                           "color: #787878; border-color: #DEDEDE; border-style: solid; border-width: 1px; }");
    style += QString("QTreeWidget::item:has-children { padding-left: %1px; color: #1E3287; }").arg(3 * scale);
#endif
    style += QString("QTreeWidget::item:!has-children { padding-left: %1px; }").arg(7 * scale);

    ui->trw_navigation->setStyleSheet(style);

    for (int i = 0; i < ui->trw_navigation->topLevelItemCount(); ++i) {
#ifdef Q_OS_MAC
        ui->trw_navigation->topLevelItem(i)->setTextAlignment(0, Qt::AlignLeft | Qt::AlignBottom);
        ui->trw_navigation->topLevelItem(i)->setSizeHint(0, QSize(0, i ? 28 : 24));
#else
        ui->trw_navigation->topLevelItem(i)->setSizeHint(0, QSize(0, 28 * scale));
#endif
    }

    ui->wv_main->setZoomFactor(scale);

    ui->toolbarstack->scaleFactorChanged();
}

void ViewTab::createViewItems()
{
    views[View::Store] = new StoreView(this);
    views[View::RefrigerantManagement] = new RefrigerantManagementView(this);
    views[View::Customers] = new CustomersView(this);
    views[View::Circuits] = new CircuitsView(this);
    views[View::Inspections] = new InspectionsView(this);
    views[View::InspectionDetails] = new InspectionDetailsView(this);
    views[View::TableOfInspections] = new TableView(this);
    views[View::Repairs] = new RepairsView(this);
    views[View::Inspectors] = new InspectorsView(this);
    views[View::InspectorDetails] = new InspectorDetailsView(this);
    views[View::OperatorReport] = new OperatorReportView(this);
    views[View::LeakagesByApplication] = new LeakagesByApplicationView(this);
    views[View::Agenda] = new AgendaView(this);
    views[View::InspectionImages] = new InspectionImagesView(this);
    views[View::AssemblyRecords] = new AssemblyRecordsView(this);
    views[View::AssemblyRecordDetails] = new AssemblyRecordDetailsView(this);
    views[View::AssemblyRecordTypes] = new AssemblyRecordTypesView(this);
    views[View::AssemblyRecordItems] = new AssemblyRecordItemsView(this);
    views[View::CircuitUnitTypes] = new CircuitUnitTypesView(this);

    view_handler = new ViewUrlSchemeHandler(views, this);

    QTreeWidgetItem *group_service_company = new QTreeWidgetItem(ui->trw_navigation);
    group_service_company->setText(0, tr("Service Company"));
    formatGroupItem(group_service_company);

    QTreeWidgetItem *item_store = new QTreeWidgetItem(group_service_company);
    item_store->setText(0, tr("Store"));
    item_store->setData(0, Qt::UserRole, View::Store);
    item_store->setIcon(0, QIcon(":/images/images/store_view.png"));
    view_items[View::Store] = item_store;

    QTreeWidgetItem *item_refrigerant_management = new QTreeWidgetItem(group_service_company);
    item_refrigerant_management->setText(0, tr("Refrigerant Management"));
    item_refrigerant_management->setData(0, Qt::UserRole, View::RefrigerantManagement);
    item_refrigerant_management->setIcon(0, QIcon(":/images/images/management_view.png"));
    view_items[View::RefrigerantManagement] = item_refrigerant_management;

    QTreeWidgetItem *item_leakages = new QTreeWidgetItem(group_service_company);
    item_leakages->setText(0, tr("Leakages by Application"));
    item_leakages->setData(0, Qt::UserRole, View::LeakagesByApplication);
    item_leakages->setIcon(0, QIcon(":/images/images/leakages_view.png"));
    view_items[View::LeakagesByApplication] = item_leakages;

    QTreeWidgetItem *item_agenda = new QTreeWidgetItem(group_service_company);
    item_agenda->setText(0, tr("Agenda"));
    item_agenda->setData(0, Qt::UserRole, View::Agenda);
    item_agenda->setIcon(0, QIcon(":/images/images/agenda_view.png"));
    view_items[View::Agenda] = item_agenda;

    QTreeWidgetItem *item_inspectors = new QTreeWidgetItem(group_service_company);
    item_inspectors->setText(0, tr("Inspectors"));
    item_inspectors->setData(0, Qt::UserRole, View::Inspectors);
    item_inspectors->setIcon(0, QIcon(":/images/images/inspectors_view.png"));
    view_items[View::Inspectors] = item_inspectors;

    QTreeWidgetItem *item_inspector_details = new QTreeWidgetItem(group_service_company);
    item_inspector_details->setText(0, tr("Inspector"));
    item_inspector_details->setData(0, Qt::UserRole, View::InspectorDetails);
    item_inspector_details->setIcon(0, QIcon(":/images/images/inspector_view.png"));
    view_items[View::InspectorDetails] = item_inspector_details;

    QTreeWidgetItem *item_customers = new QTreeWidgetItem(group_service_company);
    item_customers->setText(0, tr("Customers"));
    item_customers->setData(0, Qt::UserRole, View::Customers);
    item_customers->setIcon(0, QIcon(":/images/images/customers_view.png"));
    view_items[View::Customers] = item_customers;

    QTreeWidgetItem *item_operator_report = new QTreeWidgetItem(group_service_company);
    item_operator_report->setText(0, tr("Operator Report"));
    item_operator_report->setData(0, Qt::UserRole, View::OperatorReport);
    item_operator_report->setIcon(0, QIcon(":/images/images/report_view.png"));
    view_items[View::OperatorReport] = item_operator_report;

    QTreeWidgetItem *group_basic_logbook = new QTreeWidgetItem(ui->trw_navigation);
    group_basic_logbook->setText(0, tr("Basic Logbook"));
    formatGroupItem(group_basic_logbook);

    QTreeWidgetItem *item_repairs = new QTreeWidgetItem(group_basic_logbook);
    item_repairs->setText(0, tr("Repairs"));
    item_repairs->setData(0, Qt::UserRole, View::Repairs);
    item_repairs->setIcon(0, QIcon(":/images/images/repair_view.png"));
    view_items[View::Repairs] = item_repairs;

    QTreeWidgetItem *group_detailed_logbook = new QTreeWidgetItem(ui->trw_navigation);
    group_detailed_logbook->setText(0, tr("Detailed Logbook"));
    formatGroupItem(group_detailed_logbook);

    QTreeWidgetItem *item_circuits = new QTreeWidgetItem(group_detailed_logbook);
    item_circuits->setText(0, tr("Circuits"));
    item_circuits->setData(0, Qt::UserRole, View::Circuits);
    item_circuits->setIcon(0, QIcon(":/images/images/circuit_view.png"));
    view_items[View::Circuits] = item_circuits;

    QTreeWidgetItem *item_inspections = new QTreeWidgetItem(group_detailed_logbook);
    item_inspections->setText(0, tr("Inspections"));
    item_inspections->setData(0, Qt::UserRole, View::Inspections);
    item_inspections->setIcon(0, QIcon(":/images/images/inspection_view.png"));
    view_items[View::Inspections] = item_inspections;

    QTreeWidgetItem *item_inspection_details = new QTreeWidgetItem(group_detailed_logbook);
    item_inspection_details->setText(0, tr("Inspection"));
    item_inspection_details->setData(0, Qt::UserRole, View::InspectionDetails);
    item_inspection_details->setIcon(0, QIcon(":/images/images/inspection_view.png"));
    view_items[View::InspectionDetails] = item_inspection_details;

    QTreeWidgetItem *item_inspection_images = new QTreeWidgetItem(group_detailed_logbook);
    item_inspection_images->setText(0, tr("Inspection Images"));
    item_inspection_images->setData(0, Qt::UserRole, View::InspectionImages);
    item_inspection_images->setIcon(0, QIcon(":/images/images/image.png"));
    view_items[View::InspectionImages] = item_inspection_images;

    QTreeWidgetItem *item_assembly_records = new QTreeWidgetItem(group_detailed_logbook);
    item_assembly_records->setText(0, tr("Assembly Records"));
    item_assembly_records->setData(0, Qt::UserRole, View::AssemblyRecords);
    item_assembly_records->setIcon(0, QIcon(":/images/images/assembly_records_view.png"));
    view_items[View::AssemblyRecords] = item_assembly_records;

    QTreeWidgetItem *item_assembly_record_details = new QTreeWidgetItem(group_detailed_logbook);
    item_assembly_record_details->setText(0, tr("Assembly Record"));
    item_assembly_record_details->setData(0, Qt::UserRole, View::AssemblyRecordDetails);
    item_assembly_record_details->setIcon(0, QIcon(":/images/images/assembly_record_view.png"));
    view_items[View::AssemblyRecordDetails] = item_assembly_record_details;

    group_tables = new QTreeWidgetItem(ui->trw_navigation);
    group_tables->setText(0, tr("Tables"));
    formatGroupItem(group_tables);
    view_items[View::TableOfInspections] = NULL;

    QTreeWidgetItem *group_assembly_records = new QTreeWidgetItem(ui->trw_navigation);
    group_assembly_records->setText(0, tr("Assembly Records"));
    formatGroupItem(group_assembly_records);

    QTreeWidgetItem *item_assembly_record_types = new QTreeWidgetItem(group_assembly_records);
    item_assembly_record_types->setText(0, tr("Types"));
    item_assembly_record_types->setData(0, Qt::UserRole, View::AssemblyRecordTypes);
    item_assembly_record_types->setIcon(0, QIcon(":/images/images/assembly_record_types_view.png"));
    view_items[View::AssemblyRecordTypes] = item_assembly_record_types;

    QTreeWidgetItem *item_assembly_record_items = new QTreeWidgetItem(group_assembly_records);
    item_assembly_record_items->setText(0, tr("Items"));
    item_assembly_record_items->setData(0, Qt::UserRole, View::AssemblyRecordItems);
    item_assembly_record_items->setIcon(0, QIcon(":/images/images/assembly_record_items_view.png"));
    view_items[View::AssemblyRecordItems] = item_assembly_record_items;

    QTreeWidgetItem *item_circuit_unit_types = new QTreeWidgetItem(group_assembly_records);
    item_circuit_unit_types->setText(0, tr("Circuit Unit Types"));
    item_circuit_unit_types->setData(0, Qt::UserRole, View::CircuitUnitTypes);
    item_circuit_unit_types->setIcon(0, QIcon(":/images/images/circuit_unit_types_view.png"));
    view_items[View::CircuitUnitTypes] = item_circuit_unit_types;
}

void ViewTab::formatGroupItem(QTreeWidgetItem *item)
{
    QFont font = item->font(0);
#ifdef Q_OS_MAC
    font.setBold(true);
    if (Global::macVersion() == 12 /* 10.10 */) {
        font.setLetterSpacing(QFont::PercentageSpacing, 105);
    }
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() + 3);
#endif
    item->setFont(0, font);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
}

void ViewTab::setNeedsRefresh()
{
    if (parentWindow()->currentTab() != this)
        needs_refresh = true;
}

void ViewTab::reloadTables(const MTDictionary &tables)
{
    foreach (QTreeWidgetItem *item, group_tables->takeChildren())
        delete item;

    for (int i = 0; i < tables.count(); ++i) {
        addTable(-1, tables.key(i), tables.value(i));
    }
}

void ViewTab::addTable(int index, const QString &uuid, const QString &name)
{
    QTreeWidgetItem *item_table = new QTreeWidgetItem;
    item_table->setText(0, name);
    item_table->setData(0, Qt::UserRole, View::TableOfInspections);
    item_table->setData(1, Qt::UserRole, uuid);
    item_table->setIcon(0, QIcon(":/images/images/table_view.png"));
    if (index < 0)
        group_tables->addChild(item_table);
    else
        group_tables->insertChild(index, item_table);
}

void ViewTab::removeTable(const QString &uuid)
{
    for (int i = 0; i < group_tables->childCount(); ++i) {
        QTreeWidgetItem *item = group_tables->child(i);
        if (item->data(1, Qt::UserRole).toString() == uuid) {
            delete item;
            break;
        }
    }
}

void ViewTab::connectSlots(QObject *receiver)
{
    QObject::connect(receiver, SIGNAL(databaseModified()), this, SLOT(setNeedsRefresh()));
    QObject::connect(this, SIGNAL(tabTextChanged(QWidget *, const QString &)), receiver, SLOT(tabTextChanged(QWidget *, const QString &)));

    ui->toolbarstack->connectSlots(receiver);
}

void ViewTab::enableAllTools()
{
    parentWindow()->enableTools();
}

void ViewTab::enableTools()
{
    view_items[View::Circuits]->setDisabled(!isCustomerSelected());
    for (int i = 0; i < group_tables->childCount(); ++i)
        group_tables->child(i)->setDisabled(!isCustomerSelected());
    view_items[View::OperatorReport]->setDisabled(!isCustomerSelected());

    view_items[View::Inspections]->setDisabled(!isCircuitSelected());
    view_items[View::AssemblyRecords]->setDisabled(!isCircuitSelected());

    view_items[View::InspectionDetails]->setDisabled(!isInspectionSelected());
    view_items[View::InspectionImages]->setDisabled(!isInspectionSelected());
    view_items[View::AssemblyRecordDetails]->setDisabled(!isInspectionSelected());

    view_items[View::InspectorDetails]->setDisabled(!isInspectorSelected());

    ui->toolbarstack->enableTools();
}

MainWindowSettings &ViewTab::mainWindowSettings()
{
    return parentWindow()->settings();
}

ToolBarStack *ViewTab::toolBarStack() const
{
    return ui->toolbarstack;
}

QWebEngineView *ViewTab::webView() const
{
    return ui->wv_main;
}

void ViewTab::setView(View::ViewID view, const QString &table_uuid)
{
    if (view < 0 || view >= View::ViewCount)
        view = View::Store;

    if (view_items[view]) {
        if (ui->trw_navigation->currentItem() == view_items[view]) {
            refreshView();
        } else {
            bool disabled = view_items[view]->isDisabled();
            view_items[view]->setDisabled(false);
            ui->trw_navigation->setCurrentItem(view_items[view]);
            view_items[view]->setDisabled(disabled);
        }
    } else if (view == View::TableOfInspections) {
        QTreeWidgetItem *item = ui->trw_navigation->currentItem();

        if (!group_tables->childCount())
            return;

        if (item && item->parent() == group_tables && (table_uuid.isEmpty() || item->data(1, Qt::UserRole) == table_uuid)) {
            refreshView();
            return;
        }

        int i = 0;
        for (int j = 0; j < group_tables->childCount(); ++j) {
            if (group_tables->child(j)->data(1, Qt::UserRole) == table_uuid) {
                i = j;
                break;
            }
        }

        ui->trw_navigation->setCurrentItem(group_tables->child(i));
    }
}

void ViewTab::setView(int view, const QString &table)
{
    setView((View::ViewID)view, table);
}

void ViewTab::refreshView()
{
    viewChanged(ui->trw_navigation->currentItem(), NULL);
}

void ViewTab::refreshViewIfNeeded()
{
    if (needs_refresh) {
        needs_refresh = false;
        refreshView();
    }
}

View::ViewID ViewTab::currentView() const
{
    QTreeWidgetItem *item = ui->trw_navigation->currentItem();

    if (item)
        return (View::ViewID)item->data(0, Qt::UserRole).toInt();

    return View::ViewCount;
}

QString ViewTab::currentViewTitle() const
{
    QTreeWidgetItem *item = ui->trw_navigation->currentItem();

    if (item)
        return item->text(0);

    return QString();
}

QString ViewTab::currentTableUUID() const
{
    QTreeWidgetItem *item = ui->trw_navigation->currentItem();

    if (item && item->parent() == group_tables)
        return item->data(1, Qt::UserRole).toString();

    return QString();
}

bool ViewTab::isShowDateUpdatedChecked() const
{
    return parentWindow()->isShowDateUpdatedChecked();
}

bool ViewTab::isShowOwnerChecked() const
{
    return parentWindow()->isShowOwnerChecked();
}

bool ViewTab::isShowNotesChecked() const
{
    return parentWindow()->isShowNotesChecked();
}

bool ViewTab::isShowLeakedChecked() const
{
    return parentWindow()->isShowLeakedChecked();
}

bool ViewTab::isShowPlaceOfOperationChecked() const
{
    return parentWindow()->isShowPlaceOfOperationChecked();
}

bool ViewTab::isShowBuildingChecked() const
{
    return parentWindow()->isShowBuildingChecked();
}

bool ViewTab::isShowDeviceChecked() const
{
    return parentWindow()->isShowDeviceChecked();
}

bool ViewTab::isShowManufacturerChecked() const
{
    return parentWindow()->isShowManufacturerChecked();
}

bool ViewTab::isShowTypeChecked() const
{
    return parentWindow()->isShowTypeChecked();
}

bool ViewTab::isShowSerialNumberChecked() const
{
    return parentWindow()->isShowSerialNumberChecked();
}

bool ViewTab::isShowYearOfPurchaseChecked() const
{
    return parentWindow()->isShowYearOfPurchaseChecked();
}

bool ViewTab::isShowDateOfCommissioningChecked() const
{
    return parentWindow()->isShowDateOfCommissioningChecked();
}

bool ViewTab::isShowFieldOfApplicationChecked() const
{
    return parentWindow()->isShowFieldOfApplicationChecked();
}

bool ViewTab::isShowOilChecked() const
{
    return parentWindow()->isShowOilChecked();
}

bool ViewTab::isShowMostRecentFirstChecked() const
{
    return parentWindow()->isShowMostRecentFirstChecked();
}

bool ViewTab::isCompareValuesChecked() const
{
    return parentWindow()->isCompareValuesChecked();
}

bool ViewTab::isPrinterFriendlyVersionChecked() const
{
    return parentWindow()->isPrinterFriendlyVersionChecked();
}

QString ViewTab::appendDefaultOrderToColumn(const QString &column) const
{
    return parentWindow()->appendDefaultOrderToColumn(column);
}

QString ViewTab::filterServiceCompanyUUID() const
{
    return ui->toolbarstack->filterServiceCompanyUUID();
}

bool ViewTab::isFilterServiceCompanyChecked() const
{
    return ui->toolbarstack->isFilterServiceCompanyChecked();
}

void ViewTab::setFilterServiceCompanyChecked(bool checked)
{
    ui->toolbarstack->setFilterServiceCompanyChecked(checked);
}

bool ViewTab::isServiceCompanySelected() const
{
    return ui->toolbarstack->isServiceCompanySelected();
}

QString ViewTab::selectedServiceCompanyUUID() const
{
    return ui->toolbarstack->selectedServiceCompanyUUID();
}

void ViewTab::setSelectedServiceCompanyUUID(const QString &service_company_uuid)
{
    ui->toolbarstack->setSelectedServiceCompanyUUID(service_company_uuid);
}

void ViewTab::viewChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if (current == previous)
        return;

    if (!current || !current->parent())
        return;

    View::ViewID view = (View::ViewID)current->data(0, Qt::UserRole).toInt();

    if (!isCustomerSelected() && view >= View::CustomerRequired && view <= View::CustomerRequiredEnd) {
        setView(View::Customers);
        return;
    } else if (!isCircuitSelected() && view >= View::CircuitRequired && view <= View::CircuitRequiredEnd) {
        setView(View::Circuits);
        return;
    } else if (!isInspectionSelected() && view >= View::InspectionRequired && view <= View::InspectionRequiredEnd) {
        setView(View::Inspections);
        return;
    } else if (!isInspectorSelected() && view >= View::InspectorRequired && view <= View::InspectorRequiredEnd) {
        setView(View::Inspectors);
        return;
    }

    emit viewChanged(view);

    QString tabText = current->text(0);
    if (isCustomerSelected() && view >= View::CustomerRequired && view <= View::CustomerRequiredEnd) {
        Customer customer(selectedCustomerUUID());
        tabText.append(QString::fromUtf8(" \342\200\224 %1").arg(customer.companyName().isEmpty() ? customer.companyID() : customer.companyName()));
    }
    emit tabTextChanged(this, tabText);

    if (parentWindow()->currentTab() == this)
        ui->wv_main->setUrl(QUrl(QString("view://v%1").arg(view)));
    else
        setNeedsRefresh();
}

void ViewTab::executeLink(const QUrl &url)
{
    if (url.scheme().startsWith("http")) {
        QDesktopServices::openUrl(url);
    }
    Link *link = linkParser().parse(url.toString());
    if (link) {
        executeLink(link);
    }
}

void ViewTab::executeLink(Link *link)
{
    if (!link->orderBy().isEmpty()) {
        mainWindowSettings().setOrderByForView(link->views(), link->orderBy());
        refreshView();
        return;
    }

    QString id;

    bool select_with_javascript = false;
    bool view_changed = link->viewId() != currentView();

    switch (link->viewAt(0)) {
    case LinkParser::ToggleDetailsVisible:
        id = link->idValue("toggledetailsvisible");
        if (id == "customer")
            mainWindowSettings().toggleCustomerDetailsVisible();
        else if (id == "circuit")
            mainWindowSettings().toggleCircuitDetailsVisible();
        else if (id == "circuits")
            mainWindowSettings().toggleCircuitsVisible();
        else if (id == "excludedcircuits")
            mainWindowSettings().toggleExcludedCircuitsVisible();
        else if (id == "decommissionedcircuits")
            mainWindowSettings().toggleDecommissionedCircuitsVisible();
        refreshView();
        break;

    case LinkParser::Customer:
        select_with_javascript = !view_changed;
        id = link->idValue("customer");
        if (link->action() == Link::Star) {
            if (link->countViews() <= 1) {
                parentWindow()->starCustomer(id);
            }
        } else if (id != selectedCustomerUUID()) {
            loadCustomer(id, view_changed && link->countViews() <= 1 && link->action() == Link::View);
        } else if (link->countViews() <= 1 && link->action() == Link::View) {
            setView(View::Circuits);
        }

        if (link->countViews() <= 1 && link->action() == Link::Edit) {
            select_with_javascript = true;
            parentWindow()->editCustomer();
        }
        break;

    case LinkParser::Repair:
        select_with_javascript = !view_changed;
        loadRepair(link->idValue("repair"), view_changed && link->action() == Link::View);
        if (link->action() == Link::Edit) {
            select_with_javascript = true;
            parentWindow()->editRepair();
        }
        break;

    case LinkParser::Store:
        if (link->action() == Link::Edit)
            parentWindow()->editServiceCompany(link->idValue("servicecompany"));
        else
            setView(View::Store);
        break;

    case LinkParser::Inspector:
        select_with_javascript = !view_changed;
        loadInspector(link->idValue("inspector"), view_changed && link->action() == Link::View);
        if (link->action() == Link::Edit)
            parentWindow()->editInspector();
        break;

    case LinkParser::InspectorReport:
        loadInspectorReport(link->idValue("inspectorreport"), link->action() == Link::View);
        break;

    case LinkParser::AllCustomers:
        setView(View::Customers);
        break;

    case LinkParser::ToggleDetailedView:
        id = link->idValue("toggledetailedview");
        ((StoreView *)views[View::Store])->toggleYear(id.toInt());
        refreshView();
        ui->wv_main->page()->runJavaScript(QString("document.getElementById('%1').scrollIntoView(true);").arg(id));
        break;

    case LinkParser::RefrigerantManagement:
        setView(View::RefrigerantManagement);
        break;

    case LinkParser::RefrigerantRecord:
        if (link->action() == Link::Edit)
            parentWindow()->editRefrigerantRecord(link->idValue("refrigerantrecord"));
        break;

    case LinkParser::LeakagesByApplication:
        setView(View::LeakagesByApplication);
        break;

    case LinkParser::Agenda:
        setView(View::Agenda);
        break;

    case LinkParser::AssemblyRecordType:
        select_with_javascript = !view_changed;
        loadAssemblyRecordType(link->idValue("assemblyrecordtype"), view_changed && link->action() == Link::View);
        if (link->action() == Link::Edit)
            parentWindow()->editAssemblyRecordType();
        break;

    case LinkParser::AssemblyRecordItemType:
        select_with_javascript = !view_changed;
        loadAssemblyRecordItemType(link->idValue("assemblyrecorditemtype"), view_changed && link->action() == Link::View);
        if (link->action() == Link::Edit)
            parentWindow()->editAssemblyRecordItemType();
        break;

    case LinkParser::AssemblyRecordCategory:
        select_with_javascript = !view_changed;
        loadAssemblyRecordItemCategory(link->idValue("assemblyrecorditemcategory"), view_changed && link->action() == Link::View);
        if (link->action() == Link::Edit)
            parentWindow()->editAssemblyRecordItemCategory();
        break;

    case LinkParser::CircuitUnitType:
        select_with_javascript = !view_changed;
        loadCircuitUnitType(link->idValue("circuitunittype"), view_changed && link->action() == Link::View);
        if (link->action() == Link::Edit)
            parentWindow()->editCircuitUnitType();
        break;

    case LinkParser::AllAssemblyRecords:
        setView(View::AssemblyRecords);
        break;

    case LinkParser::AllInspectors:
        setView(View::Inspectors);
        break;

    case LinkParser::AllAssemblyRecordTypes:
        setView(View::AssemblyRecordTypes);
        break;

    case LinkParser::AllAssemblyRecordItems:
        setView(View::AssemblyRecordItems);
        break;

    case LinkParser::AllCircuitUnitTypes:
        setView(View::CircuitUnitTypes);
        break;

    case LinkParser::AllRepairs:
        setView(View::Repairs);
        break;
    }

    switch (link->viewAt(1)) {
    case LinkParser::Circuit:
        if (link->action() == Link::Star) {
            if (link->countViews() <= 2) {
                parentWindow()->starCircuit(link->idValue("customer"), link->idValue("circuit"));
            }
        } else if (link->idValue("circuit") != selectedCircuitUUID()) {
            loadCircuit(link->idValue("circuit"), link->countViews() <= 2 && link->action() == Link::View);
        } else if (link->countViews() <= 2 && link->action() == Link::View) {
            setView(View::Inspections);
        }

        if (link->countViews() <= 2 && link->action() == Link::Edit) {
            select_with_javascript = true;
            parentWindow()->editCircuit();
        }
        break;

    case LinkParser::AllAssemblyRecords:
        setView(View::AssemblyRecords);
        break;

    case LinkParser::OperatorReport:
        setView(View::OperatorReport);
        break;

    case LinkParser::AllRepairs:
        setView(View::Repairs);
        break;
    }

    switch (link->viewAt(2)) {
    case LinkParser::Inspection:
        if (link->action() == Link::Remove) {
            parentWindow()->removeInspection(link->idValue("inspection"));
            break;
        }

        if (link->idValue("inspection") != selectedInspectionUUID())
            loadInspection(link->idValue("inspection"), link->countViews() <= 3 && link->action() == Link::View);
        else if (link->countViews() <= 3 && link->action() == Link::View)
            setView(View::InspectionDetails);

        if (link->action() == Link::Edit) {
            select_with_javascript = true;
            parentWindow()->editInspection();
        }
        break;

    case LinkParser::AssemblyRecord:
        id = link->lastIdValue();
        id.remove(0, id.indexOf(":") + 1);
        if (id != selectedInspectionUUID())
            loadAssemblyRecord(id, link->action() == Link::View);
        else if (link->action() == Link::View)
            setView(View::AssemblyRecordDetails);
        break;

    case LinkParser::TableOfInspections:
        setView(View::TableOfInspections);
        break;

    case LinkParser::Compressor:
        select_with_javascript = false;
        id = link->idValue("compressor");
        if (id.isEmpty())
            clearSelectedCompressor();
        else
            setSelectedCompressorUUID(id);
        break;

    case LinkParser::AllAssemblyRecords:
        setView(View::AssemblyRecords);
        break;
    }

    switch (link->viewAt(3)) {
    case LinkParser::AssemblyRecord:
        setView(View::AssemblyRecordDetails);
        break;

    case LinkParser::TableOfInspections:
        setView(View::TableOfInspections);
        break;

    case LinkParser::InspectionImages:
        setView(View::InspectionImages);
        break;
    }

    if (!link->countIds())
        select_with_javascript = false;
    if (select_with_javascript) {
        ui->wv_main->page()->runJavaScript(QString("select('%1:%2');").arg(link->lastIdKey()).arg(link->lastIdValue()));
    }
}

void ViewTab::setDefaultWebPage()
{
    MTWebPage *page = new MTWebPage(ui->wv_main);
    page->setLinkDelegationPolicy(MTWebPage::DelegateAllLinks);
    ui->wv_main->setPage(page);
    ui->wv_main->setZoomFactor(Global::scaleFactor());

    page->profile()->installUrlSchemeHandler(QString("dbfile").toUtf8(), new DBFileUrlSchemeHandler);
    page->profile()->installUrlSchemeHandler(QString("view").toUtf8(), view_handler);

    QObject::connect(page, SIGNAL(linkClicked(const QUrl &)), this, SLOT(executeLink(const QUrl &)));
}

void ViewTab::reportData()
{
    ui->trw_navigation->setVisible(false);

    ReportDataController *controller = new ReportDataController(ui->wv_main, ui->toolbarstack);
    QObject::connect(controller, SIGNAL(processing(bool)), this, SLOT(setDisabled(bool)));
    QObject::connect(controller, SIGNAL(destroyed()), this, SLOT(reportDataFinished()));
}

void ViewTab::reportDataFinished()
{
    ui->trw_navigation->setVisible(true);

    parentWindow()->reportDataFinished();
    setDefaultWebPage();
    refreshView();
}
