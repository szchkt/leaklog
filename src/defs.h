/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2019 Matus & Michal Tomlein

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

#define LEAKLOG_VERSION "2.0.4"
#define F_LEAKLOG_VERSION 2.0004
#define LEAKLOG_PREVIEW_VERSION 0
#define DB_VERSION "2.0"
#define F_DB_VERSION 2.0
#define F_DB_MIN_LEAKLOG_VERSION 2.0000

#define REAL_NUMBER_PRECISION 2
#define REAL_NUMBER_PRECISION_EXP 100.0L
#define FLOAT_FORMAT 'g'
#define FLOAT_PRECISION 12
#define FLOAT_ROUND(f) (double)(roundl((f) * 1000.0L) / 1000.0L)
#define FLOAT_ARG(f) FLOAT_ROUND(f), 0, FLOAT_FORMAT, FLOAT_PRECISION, QLatin1Char(' ')

#define INSPECTORS_CATEGORY_UUID "9aaa2a1c-1b5e-507a-81a6-bd82079f430d"
#define CIRCUIT_UNITS_CATEGORY_UUID "d27baba8-10b4-5dfa-bbcd-d825095b3386"

#define LEAKAGES_TABLE_UUID "61da71bc-c08d-529f-bd97-17d9012fbe35"
#define PRESSURES_AND_TEMPERATURES_TABLE_UUID "cd582385-b81f-5826-8820-85870c3f9bd2"
#define COMPRESSORS_TABLE_UUID "129ff63b-150f-5dbe-950b-07a7698c3029"

#define JPEG_QUALITY 90

#define DATE_FORMAT "yyyy.MM.dd"
#define TIME_FORMAT "hh:mm"
#define DATE_TIME_FORMAT DATE_FORMAT "-" TIME_FORMAT

#define UNIT_SEPARATOR QChar(31)

class QString;
class QVariant;
template<class Key, class T>
class QMap;
template<class Key, class T>
class QMapIterator;
template<class Key, class T>
class QMultiMap;
template<class T>
class QList;

typedef QMap<QString, QVariant> QVariantMap;
typedef QMapIterator<QString, QVariant> QVariantMapIterator;
typedef QList<QVariantMap> ListOfVariantMaps;
typedef QMap<QString, QVariantMap> MapOfVariantMaps;
typedef QMultiMap<QString, QVariantMap> MultiMapOfVariantMaps;

#include <QtGlobal>
#include <QDebug>

#endif // DEFS_H
