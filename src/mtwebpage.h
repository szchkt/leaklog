/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2023 Matus & Michal Tomlein

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

#ifndef MTWEBPAGE_H
#define MTWEBPAGE_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QWebEnginePage>
#include <QWebEngineProfile>
#else
#include <QtWebEngineWidgets/QWebEnginePage>
#include <QtWebEngineWidgets/QWebEngineProfile>
#endif

class MTWebPage : public QWebEnginePage
{
    Q_OBJECT

public:
    enum LinkDelegationPolicy {
        DontDelegateLinks,
        DelegateExternalLinks,
        DelegateAllLinks
    };

    MTWebPage(QObject *parent = 0): QWebEnginePage(new QWebEngineProfile(parent), parent) {}

    void setLinkDelegationPolicy(LinkDelegationPolicy policy);
    LinkDelegationPolicy linkDelegationPolicy() const;

signals:
    void linkClicked(const QUrl &url);

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);

private:
    LinkDelegationPolicy _linkDelegationPolicy;
};

#endif // MTWEBPAGE_H
