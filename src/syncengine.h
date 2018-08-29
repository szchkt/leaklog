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

#ifndef SYNCENGINE_H
#define SYNCENGINE_H

#include <QObject>
#include <QString>

#include <functional>

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;

class Authenticator : public QObject
{
    Q_OBJECT

public:
    Authenticator(QObject *parent = NULL);
    ~Authenticator();

    void logIn(const QString &username, const QString &password);
    void logOut();

    void getDatabases(std::function<void(bool success, const QJsonDocument &databases)> completionHandler);

    inline QString username() const { return _username; }
    inline QString token() const { return _token; }
    inline QString error() const { return _error; }

signals:
    void loginFinished(bool);
    void logoutFinished();

private:
    void sendRequest(const QNetworkRequest &request, const QJsonDocument &document);

private slots:
    void requestFinished(QNetworkReply *reply);

private:
    QString _username;
    QString _token;
    QString _error;
    QNetworkAccessManager *_network_manager;
    QNetworkReply *_reply;
    QMap<QNetworkReply *, std::function<void(bool, const QJsonDocument &)>> *_completionHandlers;
};

class SyncEngine : public QObject
{
    Q_OBJECT

public:
    SyncEngine(Authenticator *authenticator, QObject *parent = NULL);

    inline QString databaseUUID() const { return _database_uuid; }
    QString databaseName();
    inline QString error() const { return _error; }
    inline QString action() const { return _action; }

    QJsonObject journalState() const;
    static QJsonObject journalStateForVersion(QJsonObject journal_state, int maximum_version);
    void sync(bool force = false);

    static QJsonValue jsonValueForVariant(int column_id, const QVariant &variant, int *length = NULL);
    static QVariant variantForJsonValue(int column_id, const QJsonValue &value);

signals:
    void syncStarted();
    void syncProgress(double progress);
    void syncFinished(bool success, bool changed);

private:
    bool sync(const QJsonDocument &response_document);
    bool applyJournalEntries(const QJsonArray &journal_entries, const QJsonObject &records, const QJsonArray &local_entries, const QJsonObject &journal_state);
    void sendRequest(const QJsonDocument &document);

private slots:
    void requestFinished(QNetworkReply *reply);

private:
    Authenticator *_authenticator;
    QString _database_uuid;
    QString _database_name;
    QString _error;
    QString _action;
    QNetworkAccessManager *_network_manager;
    QNetworkReply *_reply;
};

#endif // SYNCENGINE_H
