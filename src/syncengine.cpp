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

#include "syncengine.h"

#include "global.h"
#include "records.h"

#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSqlError>
#include <QSettings>
#include <QUrlQuery>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace Global;

static QString const baseURL = "https://leaklog.org";

static QString JournalEntryStringID(int operation_id, int column_id, QString record_uuid)
{
    return QString("%1:%2:%3").arg(operation_id).arg(column_id).arg(record_uuid);
}

Authenticator::Authenticator(QObject *parent):
    QObject(parent),
    _network_manager(new QNetworkAccessManager(this)),
    _reply(NULL)
{
    QSettings settings("SZCHKT", "Leaklog");
    _username = settings.value("username").toString();
    _token = settings.value("auth_token").toString();

    connect(_network_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(requestFinished(QNetworkReply *)));
}

void Authenticator::logIn(const QString &username, const QString &password)
{
    _username = username;
    _token.clear();

    QUrlQuery query;
    query.addQueryItem("locale", QApplication::translate("MainWindow", "en_GB"));

    QUrl url(baseURL + "/auth/");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Leaklog/" LEAKLOG_VERSION);
    QByteArray authorization = "Basic ";
    authorization.append(QString("%1:%2").arg(username).arg(password).toUtf8().toBase64());
    request.setRawHeader("Authorization", authorization);
    request.setRawHeader("X-Source-UUID", sourceUUID().toUtf8());
    request.setRawHeader("Content-Type", "application/json; charset=utf-8");
    sendRequest(request, QJsonDocument(QJsonObject()));
}

void Authenticator::logOut()
{
    _username.clear();
    _token.clear();

    QSettings settings("SZCHKT", "Leaklog");
    settings.remove("username");
    settings.remove("auth_token");

    emit logoutFinished();
}

void Authenticator::sendRequest(const QNetworkRequest &request, const QJsonDocument &document)
{
    if (_reply) {
        _reply->abort();
        _reply->deleteLater();
        _reply = NULL;
    }

    _reply = _network_manager->post(request, document.toJson(QJsonDocument::Compact));
}

void Authenticator::requestFinished(QNetworkReply *reply)
{
    if (_reply == reply) {
        _reply->deleteLater();
        _reply = NULL;
    }

    QJsonParseError error;
    QJsonDocument response = QJsonDocument::fromJson(reply->readAll(), &error);
    if (response.isNull()) {
        _error = error.errorString();
        emit loginFinished(false);
        return;
    }

    QJsonObject root = response.object();

    _token = root.value("token").toString();
    _error = root.value("error").toString();

    if (!_token.isEmpty()) {
        QSettings settings("SZCHKT", "Leaklog");
        settings.setValue("username", _username);
        settings.setValue("auth_token", _token);
    }

    emit loginFinished(!_token.isEmpty());
}

SyncEngine::SyncEngine(Authenticator *authenticator, QObject *parent):
    QObject(parent),
    _authenticator(authenticator),
    _network_manager(new QNetworkAccessManager(this)),
    _reply(NULL)
{
    _database_uuid = DBInfo::databaseUUID();

    connect(_network_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(requestFinished(QNetworkReply *)));
}

QString SyncEngine::databaseName()
{
    if (_database_name.isEmpty())
        _database_name = DBInfo::databaseName();
    return _database_name;
}

QJsonObject SyncEngine::journalState() const
{
    QJsonObject journal;

    MTSqlQuery query("SELECT source_uuid, MAX(entry_id) FROM journal GROUP BY source_uuid");
    while (query.next()) {
        journal.insert(query.value(0).toString() + ":0", query.value(1).toInt());
    }

    return journal;
}

QJsonObject SyncEngine::journalStateForVersion(QJsonObject journal_state, int maximum_version)
{
    foreach (const QString &key, journal_state.keys()) {
        if (key.split(':').value(1).toInt() > maximum_version) {
            journal_state.remove(key);
        }
    }
    return journal_state;
}

void SyncEngine::sync(bool force)
{
    if (!_authenticator->token().isEmpty() && (force || !_reply)) {
        sync(QJsonDocument());
    }
}

bool SyncEngine::sync(const QJsonDocument &response_document)
{
    bool changed = false;

    emit syncStarted();

    QJsonObject root = {
        {"version", F_LEAKLOG_VERSION},
        {"db_version", F_DB_VERSION},
        {"source_uuid", sourceUUID()},
        {"database_uuid", databaseUUID()},
        {"database_name", databaseName()},
        {"journal_version", JournalEntry::Version},
    };

    QJsonObject journal_state = journalState();

    if (response_document.isObject()) {
        QJsonObject response = response_document.object();
        QJsonObject local_journal_state = journalStateForVersion(journal_state, response.value("journal_version").toInt());
        QJsonObject server_journal_state = journalStateForVersion(response.value("journal").toObject(), JournalEntry::Version);
        if (local_journal_state == server_journal_state) {
            return changed;
        }

        QJsonArray entries;

        QStringList local_journal_state_keys = local_journal_state.keys();
        if (local_journal_state_keys.count()) {
            MTSqlQuery query;
            QStringList predicates;
            QVariantList values;
            foreach (const QJsonValue &source_uuid, local_journal_state_keys) {
                QStringList components = source_uuid.toString().split(':');

                predicates << QString("(source_uuid = ? AND entry_id > ?)");
                values << components.first();
                values << server_journal_state.value(source_uuid.toString()).toInt();
            }

            query.prepare(QString("SELECT source_uuid, entry_id, operation_id, table_id, record_uuid, column_id, date_created FROM journal WHERE %1 ORDER BY date_created, source_uuid, entry_id").arg(predicates.join(" OR ")));

            foreach (const QVariant &value, values) {
                query.addBindValue(value);
            }

            if (query.exec()) {
                while (query.next()) {
                    QJsonObject entry = {
                        {"s", query.value(0).toString()},
                        {"e", query.value(1).toInt()},
                        {"o", query.value(2).toInt()},
                        {"t", query.value(3).toInt()},
                        {"r", query.value(4).toString()},
                        {"c", query.value(5).toInt()},
                        {"d", query.value(6).toString()},
                    };
                    entries << entry;
                }
            }
        }

        QJsonArray journal_entries = response.value("journal_entries").toArray();
        if (journal_entries.count()) {
            if (applyJournalEntries(journal_entries, response.value("records").toObject(), entries, journal_state)) {
                journal_state = journalState();
                changed = true;
            }
        }

        int entry_count = 0;
        int total_length = 0;
        QJsonObject records;

        foreach (const QJsonValue &entry_value, entries) {
            entry_count++;

            QJsonObject entry = entry_value.toObject();
            QString record_uuid = entry.value("r").toString();

            QJsonValue value = records.value(record_uuid);
            if (value.isNull())
                continue;

            QJsonObject record = value.toObject();
            int column_id = entry.value("c").toInt();
            if (column_id && !record.value(QString::number(column_id)).isUndefined())
                continue;

            int table_id = entry.value("t").toInt();

            MTSqlQuery record_query;
            record_query.prepare(QString("SELECT %1 FROM %2 WHERE uuid = ?")
                                 .arg(column_id ? JournalEntry::columnNameForID(column_id, "*") : "*")
                                 .arg(JournalEntry::tableNameForID(table_id)));
            record_query.addBindValue(record_uuid);

            if (record_query.exec()) {
                if (record_query.next()) {
                    for (int i = 0; i < record_query.record().count(); ++i) {
                        column_id = JournalEntry::columnIDForName(record_query.record().fieldName(i));
                        if (!column_id)
                            continue;

                        int length = 0;
                        record.insert(QString::number(column_id), jsonValueForVariant(column_id, record_query.value(i), &length));
                        total_length += length;
                    }

                    records.insert(record_uuid, record);
                } else {
                    records.insert(record_uuid, QJsonValue::Null);
                }
            }

            if (total_length > 5242880)
                break;
        }

        while (entries.count() > entry_count) {
            entries.removeLast();
        }

        root.insert("journal_entries", entries);
        root.insert("records", records);
    }

    root.insert("journal", journal_state);
    sendRequest(QJsonDocument(root));

    return changed;
}

bool SyncEngine::applyJournalEntries(const QJsonArray &journal_entries, const QJsonObject &records, const QJsonArray &local_entries, const QJsonObject &journal_state)
{
    QMap<QString, QString> local_entries_by_string_id;
    foreach (const QJsonValue &value, local_entries) {
        QJsonObject entry = value.toObject();
        local_entries_by_string_id.insert(JournalEntryStringID(entry.value("o").toInt(), entry.value("c").toInt(), entry.value("r").toString()), entry.value("d").toString());
    }

    foreach (const QJsonValue &value, journal_entries) {
        QJsonObject entry = value.toObject();

        QString source_uuid = entry.value("s").toString();
        int entry_id = entry.value("e").toInt();

        if (entry_id <= journal_state.value(source_uuid + ":0").toInt())
            continue;

        int table_id = entry.value("t").toInt();
        QString table = JournalEntry::tableNameForID(table_id);
        if (table.isEmpty())
            continue;

        int column_id = entry.value("c").toInt();
        int operation_id = entry.value("o").toInt();
        QString record_uuid = entry.value("r").toString();
        QString date_created = entry.value("d").toString();

        switch (operation_id) {
            case JournalEntry::Insertion: {
                QJsonValue record_value = records.value(record_uuid);
                if (record_value.isNull() || record_value.isUndefined())
                    break;

                QString conflicting_local_entry_date = local_entries_by_string_id.value(JournalEntryStringID(operation_id, column_id, record_uuid));
                if (!conflicting_local_entry_date.isEmpty() && conflicting_local_entry_date > date_created)
                    break;

                MTSqlQuery query;
                query.prepare(QString("SELECT uuid FROM %1 WHERE uuid = ?").arg(table));
                query.addBindValue(record_uuid);

                if (query.exec()) {
                    QJsonObject record = record_value.toObject();

                    QStringList columns;
                    QVariantList values;
                    QStringList placeholders;

                    foreach (const QString &key, record.keys()) {
                        int id = key.toInt();
                        QString column = JournalEntry::columnNameForID(id);
                        if (!column.isEmpty()) {
                            columns << column;
                            values << variantForJsonValue(id, record.value(key));
                            placeholders << "?";
                        }
                    }

                    if (!values.count())
                        break;

                    if (query.next()) {
                        query.prepare(QString("UPDATE %1 SET %2 = ? WHERE uuid = ?").arg(table).arg(columns.join(" = ?, ")));
                        foreach (const QVariant &value, values) {
                            query.addBindValue(value);
                        }
                        query.addBindValue(record_uuid);
                    } else {
                        query.prepare(QString("INSERT INTO %1 (%2) VALUES (%3)").arg(table).arg(columns.join(", "))
                                      .arg(QStringList::fromVector(QVector<QString>(values.count(), "?")).join(", ")));
                        foreach (const QVariant &value, values) {
                            query.addBindValue(value);
                        }
                    }

                    if (!query.exec())
                        return false;
                } else {
                    return false;
                }

                break;
            }

            case JournalEntry::Update: {
                if (!column_id)
                    break;

                QString column = JournalEntry::columnNameForID(column_id);
                if (column.isEmpty())
                    break;

                QJsonValue record_value = records.value(record_uuid);
                if (record_value.isNull() || record_value.isUndefined())
                    break;

                QJsonObject record = record_value.toObject();
                QJsonValue value = record.value(QString::number(column_id));
                if (value.isUndefined())
                    break;

                QString conflicting_local_entry_date = local_entries_by_string_id.value(JournalEntryStringID(operation_id, column_id, record_uuid));
                if (!conflicting_local_entry_date.isEmpty() && conflicting_local_entry_date > date_created)
                    break;

                MTSqlQuery query;
                query.prepare(QString("UPDATE %1 SET %2 = ? WHERE uuid = ?").arg(table).arg(column));
                query.addBindValue(variantForJsonValue(column_id, value));
                query.addBindValue(record_uuid);

                if (!query.exec())
                    return false;

                break;
            }

            case JournalEntry::Deletion: {
                MTSqlQuery query;
                query.prepare(QString("DELETE FROM %1 WHERE uuid = ?").arg(table));
                query.addBindValue(record_uuid);

                if (!query.exec())
                    return false;

                break;
            }
        }

        MTSqlQuery query;
        query.prepare("INSERT INTO journal (source_uuid, entry_id, operation_id, table_id, record_uuid, column_id, date_created) VALUES (?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(source_uuid);
        query.addBindValue(entry_id);
        query.addBindValue(operation_id);
        query.addBindValue(table_id);
        query.addBindValue(record_uuid);
        query.addBindValue(column_id);
        query.addBindValue(date_created);

        if (!query.exec())
            return false;
    }

    return true;
}

void SyncEngine::sendRequest(const QJsonDocument &document)
{
    if (_reply) {
        _reply->abort();
        _reply = NULL;
    }

    QUrlQuery query;
    query.addQueryItem("locale", QApplication::translate("MainWindow", "en_GB"));

    QUrl url(baseURL + "/sync/");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Leaklog/" LEAKLOG_VERSION);
    request.setRawHeader("X-Auth-Token", _authenticator->token().toUtf8());
    request.setRawHeader("X-Source-UUID", sourceUUID().toUtf8());
    request.setRawHeader("Content-Type", "application/json; charset=utf-8");

    _reply = _network_manager->post(request, document.toJson(QJsonDocument::Compact));
    connect(_reply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(uploadProgress(qint64, qint64)));
    connect(_reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
}

void SyncEngine::requestFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (_reply == reply) {
        _reply = NULL;
    }

    QJsonParseError error;
    QByteArray data = reply->readAll();
    QJsonDocument response = QJsonDocument::fromJson(data, &error);
    if (response.isNull()) {
        _error = reply->error() != QNetworkReply::NoError ? reply->errorString() : error.errorString();
        emit syncFinished(false, false);
        return;
    }

    QJsonObject root = response.object();
    _error = root.value("error").toString();
    _action = root.value("action").toString();

    if (!_error.isEmpty()) {
        emit syncFinished(false, false);
    } else if (reply->error() != QNetworkReply::NoError) {
        _error = reply->errorString();
        emit syncFinished(false, false);
    } else {
        bool changed = sync(response);
        emit syncFinished(true, changed);
    }
}

void SyncEngine::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    emit syncProgress(bytesSent / (long double)bytesTotal / 2.0);
}

void SyncEngine::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit syncProgress(bytesReceived / (long double)bytesTotal / 2.0 + 0.5);
}

QJsonValue SyncEngine::jsonValueForVariant(int column_id, const QVariant &variant, int *length)
{
    if (!variant.isNull()) {
        switch (column_id) {
            case 24: // data
                QString data = QString::fromLatin1(variant.toByteArray().toBase64());
                if (length)
                    *length = data.length();
                return data;
        }

        switch (variant.type()) {
            case QVariant::Bool:
                return variant.toBool();

            case QVariant::Char:
            case QVariant::Int:
            case QVariant::UInt:
                return variant.toInt();

            case QVariant::LongLong:
            case QVariant::ULongLong:
                return variant.toLongLong();

            case QVariant::Double:
                return variant.toDouble();

            case QVariant::String:
                return variant.toString();

            case QVariant::ByteArray: {
                QString data = QString::fromLatin1(variant.toByteArray().toBase64());
                if (length)
                    *length = data.length();
                return data;
            }

            default:
                break;
        }
    }

    return QJsonValue::Null;
}

QVariant SyncEngine::variantForJsonValue(int column_id, const QJsonValue &value)
{
    switch (value.type()) {
        case QJsonValue::Null:
        case QJsonValue::Undefined:
            return QVariant(QVariant::String);

        default:
            // data
            if (column_id == 24) {
                return QByteArray::fromBase64(value.toString().toLatin1());
            }

            return value.toVariant();
    }
}
