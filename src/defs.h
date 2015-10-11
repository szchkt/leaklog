/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2015 Matus & Michal Tomlein

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

#define LEAKLOG_VERSION "1.9.0"
#define F_LEAKLOG_VERSION 1.0900
#define LEAKLOG_PREVIEW_VERSION 0
#define DB_VERSION "1.9"
#define F_DB_VERSION 1.9
#define F_DB_MIN_LEAKLOG_VERSION 1.0900

#define REAL_NUMBER_PRECISION 2
#define REAL_NUMBER_PRECISION_EXP 100.0L

#define INSPECTORS_CATEGORY_UUID "ba716306-f29c-44d4-91bd-db5d18b7b99f"
#define CIRCUIT_UNITS_CATEGORY_UUID "2904276f-a198-4293-9a97-8e01f0074253"

#define LEAKAGES_TABLE_UUID "b9ae82a1-ab40-4e84-9ca0-1d41489ce29b"
#define PRESSURES_AND_TEMPERATURES_TABLE_UUID "c88c1ad5-26af-47a1-871c-9e32ee02feba"
#define COMPRESSORS_TABLE_UUID "95372ed4-0652-452a-8d84-ee9c5b151572"

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
class QMultiMap;
template<class T>
class QList;

typedef QMap<QString, QVariant> QVariantMap;
typedef QList<QVariantMap> ListOfVariantMaps;
typedef QMap<QString, QVariantMap> MapOfVariantMaps;
typedef QMultiMap<QString, QVariantMap> MultiMapOfVariantMaps;

#include <QtGlobal>
#include <QDebug>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#define setSectionResizeMode setResizeMode
#endif

#endif // DEFS_H
