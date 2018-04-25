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

#ifndef MTTEXTSTREAM_H
#define MTTEXTSTREAM_H

#include "defs.h"
#include "mtvariant.h"

#include <QTextStream>

#include <cmath>

class MTTextStream : public QTextStream
{
public:
    MTTextStream(QString *string, QIODevice::OpenMode openMode = QIODevice::ReadWrite): QTextStream(string, openMode) {
        setLocale(QLocale());
        setRealNumberNotation(SmartNotation);
        setRealNumberPrecision(FLOAT_PRECISION);
    }

    inline MTTextStream &operator<<(double f) {
        QTextStream::operator<<(FLOAT_ROUND(f));
        return *this;
    }
    inline MTTextStream &operator<<(const char *string) {
        QTextStream::operator<<(string);
        return *this;
    }
    inline MTTextStream &operator<<(const QString &string) {
        QTextStream::operator<<(string);
        return *this;
    }
    inline MTTextStream &operator<<(const MTVariant &variant) {
        QTextStream::operator<<(variant.toHtml());
        return *this;
    }
};

#endif // MTTEXTSTREAM_H
