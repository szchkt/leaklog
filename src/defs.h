/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2012 Matus & Michal Tomlein

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

#ifndef DEFS_H
#define DEFS_H

#define LEAKLOG_VERSION "0.9.8"
#define F_LEAKLOG_VERSION 0.9083
#define LEAKLOG_PREVIEW_VERSION 10
#define DB_VERSION "0.9.8"
#define F_DB_VERSION 0.9084
#define F_DB_MIN_LEAKLOG_VERSION 0.9083

#define REAL_NUMBER_PRECISION 2
#define REAL_NUMBER_PRECISION_EXP 100.0L

#define INSPECTORS_CATEGORY_ID 1000
#define CIRCUIT_UNITS_CATEGORY_ID 1001

#define JPEG_QUALITY 90

class QString;
class QVariant;
template<class Key, class T>
class QMap;
template<class Key, class T>
class QMultiMap;
template<class T>
class QList;

typedef QMap<QString, QVariant> QVariantMap;
typedef QList<QVariantMap> ListOfVariantMaps;
typedef QMap<QString, QVariantMap> MapOfVariantMaps;
typedef QMultiMap<QString, QVariantMap> MultiMapOfVariantMaps;

#include <QtGlobal>

#ifdef QT_DEBUG
#include <QDebug>
#endif

#endif // DEFS_H
