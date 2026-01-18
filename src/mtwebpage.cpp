/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2026 Matus & Michal Tomlein

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

#include "mtwebpage.h"

#include <QNetworkRequest>

void MTWebPage::setLinkDelegationPolicy(LinkDelegationPolicy policy)
{
    _linkDelegationPolicy = policy;
}

MTWebPage::LinkDelegationPolicy MTWebPage::linkDelegationPolicy() const
{
    return _linkDelegationPolicy;
}

bool MTWebPage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool /*isMainFrame*/)
{
    if (type != NavigationTypeTyped) {
        switch (linkDelegationPolicy()) {
            case DontDelegateLinks:
                break;
            case DelegateExternalLinks:
            case DelegateAllLinks:
                emit linkClicked(url);
                return false;
        }
    }
    return true;
}
