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

#ifndef MTTEXTSTREAM_H
#define MTTEXTSTREAM_H

#include "mtvariant.h"

#include <QTextStream>

#include <cmath>

class MTTextStream : public QTextStream
{
public:
    MTTextStream(QString * string, QIODevice::OpenMode openMode = QIODevice::ReadWrite): QTextStream(string, openMode) {}

    inline MTTextStream & operator<<(double f) {
        long double ld = (long double)f;
        if (round(ld) == ld) this->QTextStream::operator<<(f);
        else this->QTextStream::operator<<((double)(round(ld * REAL_NUMBER_PRECISION_EXP) / REAL_NUMBER_PRECISION_EXP));
        return *this;
    }
    inline MTTextStream & operator<<(const char * string) {
        this->QTextStream::operator<<(string); return *this;
    }
    inline MTTextStream & operator<<(const QString & string) {
        this->QTextStream::operator<<(string); return *this;
    }
    inline MTTextStream & operator<<(const MTVariant & variant) {
        this->QTextStream::operator<<(variant.toHtml()); return *this;
    }
};

#endif // MTTEXTSTREAM_H
