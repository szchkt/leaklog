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

#ifndef MAIN_WINDOW_SETTINGS_H
#define MAIN_WINDOW_SETTINGS_H

#include <QString>
#include <QVariant>
#include <QList>
#include <QObject>

class QSettings;

class MainWindowSettings : public QObject
{
    Q_OBJECT

public:
    enum DateFormat {
        yyyyMMdd,
        ddMMyyyy,
        dMyyyy,
        ddMMyy,
        dMyy,
        dMMMyyyy,
        dMMMyy
    };

    static QString dateFormatToString(DateFormat date_format);

    enum TimeFormat {
        hhmm,
        hmm
    };

    static QString timeFormatToString(TimeFormat time_format);

    MainWindowSettings();

    void save(QSettings &settings) const;
    void restore(QSettings &settings);

    inline bool customerDetailsVisible() const { return m_customer_details_visible; }
    void setCustomerDetailsVisible(bool customer_details_visible) { m_customer_details_visible = customer_details_visible; }
    void toggleCustomerDetailsVisible() { m_customer_details_visible = !m_customer_details_visible; }

    inline bool circuitDetailsVisible() const { return m_circuit_details_visible; }
    void setCircuitDetailsVisible(bool circuit_details_visible) { m_circuit_details_visible = circuit_details_visible; }
    void toggleCircuitDetailsVisible() { m_circuit_details_visible = !m_circuit_details_visible; }

    inline bool circuitsVisible() const { return m_circuits_visible; }
    void setCircuitsVisible(bool circuits_visible) { m_circuits_visible = circuits_visible; }
    void toggleCircuitsVisible() { m_circuits_visible = !m_circuits_visible; }

    inline bool excludedCircuitsVisible() const { return m_excluded_circuits_visible; }
    void setExcludedCircuitsVisible(bool excluded_circuits_visible) { m_excluded_circuits_visible = excluded_circuits_visible; }
    void toggleExcludedCircuitsVisible() { m_excluded_circuits_visible = !m_excluded_circuits_visible; }

    inline bool decommissionedCircuitsVisible() const { return m_decommissioned_circuits_visible; }
    void setDecommissionedCircuitsVisible(bool decommissioned_circuits_visible) { m_decommissioned_circuits_visible = decommissioned_circuits_visible; }
    void toggleDecommissionedCircuitsVisible() { m_decommissioned_circuits_visible = !m_decommissioned_circuits_visible; }

    inline bool serviceCompanyInformationPrinted() const { return m_service_company_information_printed; }
public slots:
    void setServiceCompanyInformationPrinted(bool service_company_information_printed);
public:

    inline bool serviceCompanyInformationVisible() const { return m_service_company_information_visible; }
public slots:
    void setServiceCompanyInformationVisible(bool service_company_information_visible);
public:

    inline DateFormat dateFormat() const { return m_date_format; }
    inline QString dateFormatString() const { return m_date_format_string; }
    void setDateFormat(DateFormat date_format);
    inline TimeFormat timeFormat() const { return m_time_format; }
    inline QString timeFormatString() const { return m_time_format_string; }
    void setTimeFormat(TimeFormat time_format);
    inline QString dateTimeFormatString(const QString &join_format = "%1 %2") const { return join_format.arg(m_date_format_string).arg(m_time_format_string); }

    inline QString formatDate(const QVariant &date) const { return formatDate(date.toString()); }
    QString formatDate(const QString &date) const;
    inline QString formatDateTime(const QVariant &datetime, const QString &join_format = "%1 %2") const { return formatDateTime(datetime.toString(), join_format); }
    QString formatDateTime(const QString &datetime, const QString &join_format = "%1 %2") const;

    void setOrderByForView(quint64 view, const QString &order_by);
    QString orderByForView(quint64 view) const;

signals:
    void serviceCompanyInformationVisibilityChanged();
    void dateFormatChanged(MainWindowSettings::DateFormat);
    void timeFormatChanged(MainWindowSettings::TimeFormat);

private:
    bool m_customer_details_visible;
    bool m_circuit_details_visible;
    bool m_circuits_visible;
    bool m_excluded_circuits_visible;
    bool m_decommissioned_circuits_visible;
    bool m_service_company_information_printed;
    bool m_service_company_information_visible;

    DateFormat m_date_format;
    QString m_date_format_string;
    TimeFormat m_time_format;
    QString m_time_format_string;
    QMap<quint64, QString> m_view_orders;
};

#endif // MAIN_WINDOW_SETTINGS_H
