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

#include "mainwindowsettings.h"
#include "records.h"

#include <QSettings>
#include <QDateTime>

QString MainWindowSettings::dateFormatToString(DateFormat date_format)
{
    switch (date_format) {
        case yyyyMMdd: return tr("yyyy/MM/dd");
        case ddMMyyyy: return tr("dd/MM/yyyy");
        case dMyyyy: return tr("d/M/yyyy");
        case ddMMyy: return tr("dd/MM/yy");
        case dMyy: return tr("d/M/yy");
        case dMMMyyyy: return tr("d MMM yyyy");
        case dMMMyy: return tr("d MMM yy");
    }
    return tr("dd/MM/yyyy");
}

QString MainWindowSettings::timeFormatToString(TimeFormat time_format)
{
    switch (time_format) {
        case hhmm: return tr("hh:mm");
        case hmm: return tr("h:mm");
    }
    return tr("hh:mm");
}

MainWindowSettings::MainWindowSettings():
    QObject(),
    m_customer_details_visible(false),
    m_circuit_details_visible(false),
    m_circuits_visible(true),
    m_excluded_circuits_visible(true),
    m_decommissioned_circuits_visible(true),
    m_service_company_information_printed(true),
    m_service_company_information_visible(false),
    m_date_format(ddMMyyyy),
    m_time_format(hhmm)
{
}

void MainWindowSettings::save(QSettings &settings) const
{
    settings.setValue("main_window/customer_details_visible", m_customer_details_visible);
    settings.setValue("main_window/circuit_details_visible", m_circuit_details_visible);
    settings.setValue("main_window/circuits_visible", m_circuits_visible);
    settings.setValue("main_window/excluded_circuits_visible", m_excluded_circuits_visible);
    settings.setValue("main_window/decommissioned_circuits_visible", m_decommissioned_circuits_visible);

    settings.setValue("main_window/service_company_information_printed", m_service_company_information_printed);
    settings.setValue("main_window/service_company_information_visible", m_service_company_information_visible);

    settings.setValue("main_window/date_format", m_date_format);
    settings.setValue("main_window/time_format", m_time_format);

    settings.beginGroup("main_window/view_orders");
    for (QMap<quint64, QString>::const_iterator i = m_view_orders.constBegin(); i != m_view_orders.constEnd(); ++i)
        settings.setValue(QString::number(i.key()), i.value());
    settings.endGroup();
}

void MainWindowSettings::restore(QSettings &settings)
{
    m_customer_details_visible = settings.value("main_window/customer_details_visible", false).toBool();
    m_circuit_details_visible = settings.value("main_window/circuit_details_visible", false).toBool();
    m_circuits_visible = settings.value("main_window/circuits_visible", true).toBool();
    m_excluded_circuits_visible = settings.value("main_window/excluded_circuits_visible", true).toBool();
    m_decommissioned_circuits_visible = settings.value("main_window/decommissioned_circuits_visible", true).toBool();

    setServiceCompanyInformationPrinted(settings.value("main_window/service_company_information_printed", true).toBool());
    setServiceCompanyInformationVisible(settings.value("main_window/service_company_information_visible", false).toBool());

    setDateFormat((DateFormat)settings.value("main_window/date_format", ddMMyyyy).toInt());
    setTimeFormat((TimeFormat)settings.value("main_window/time_format", hhmm).toInt());

    settings.beginGroup("main_window/view_orders");
    foreach (const QString &key, settings.allKeys())
        m_view_orders.insert(key.toULongLong(), settings.value(key).toString());
    settings.endGroup();
}

void MainWindowSettings::setServiceCompanyInformationPrinted(bool service_company_information_printed)
{
    if (m_service_company_information_printed == service_company_information_printed)
        return;

    m_service_company_information_printed = service_company_information_printed;
    emit serviceCompanyInformationVisibilityChanged();
}

void MainWindowSettings::setServiceCompanyInformationVisible(bool service_company_information_visible)
{
    if (m_service_company_information_visible == service_company_information_visible)
        return;

    m_service_company_information_visible = service_company_information_visible;
    emit serviceCompanyInformationVisibilityChanged();
}

void MainWindowSettings::setDateFormat(DateFormat date_format)
{
    m_date_format = date_format;
    m_date_format_string = dateFormatToString(date_format);
    emit dateFormatChanged(date_format);
}

void MainWindowSettings::setTimeFormat(TimeFormat time_format)
{
    m_time_format = time_format;
    m_time_format_string = timeFormatToString(time_format);
    emit timeFormatChanged(time_format);
}

QString MainWindowSettings::formatDate(const QString &date) const
{
    return QDate::fromString(date, DATE_FORMAT).toString(dateFormatString());
}

QString MainWindowSettings::formatDateTime(const QString &datetime, const QString &join_format) const
{
    return QDateTime::fromString(datetime, DATE_TIME_FORMAT).toString(dateTimeFormatString(join_format));
}

void MainWindowSettings::setOrderByForView(quint64 view, const QString &order_by)
{
    m_view_orders.insert(view, order_by);
}

QString MainWindowSettings::orderByForView(quint64 view) const
{
    return m_view_orders.value(view);
}
