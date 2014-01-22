/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2014 Matus & Michal Tomlein

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

#include "mtcsvparser.h"

#include <QTextStream>
#include <QStringList>

MTCSVParser::MTCSVParser(QTextStream *stream):
in(stream), skip_lines(0), enclosure_char('"'), separator(','), line_terminator('\n')
{}

bool MTCSVParser::hasNextRow()
{
    while (skip_lines) {
        QChar c;
        while (!in->atEnd()) {
            *in >> c;
            if (c == line_terminator) {
                skip_lines--;
                break;
            }
        }
    }
    return !in->atEnd();
}

QStringList MTCSVParser::nextRow()
{
    QChar c;
    QString cell;
    QStringList row;
    int state = 0;
    /* 0: reading text
       1: reading quoted text
       2: ending quote read
       3: end of line
    */
    while (!in->atEnd()) {
        *in >> c;
        if (state == 0) {
            if (c == line_terminator) {
                state = 3;
                row << cell;
                break;
            }
            if (c == separator) {
                row << cell;
                cell.clear();
            } else if (c == enclosure_char) {
                state = 1;
            } else {
                cell.append(c);
            }
        } else if (state == 1) {
            if (c == enclosure_char) {
                state = 2;
            } else {
                cell.append(c);
            }
        } else if (state == 2) {
            if (c == line_terminator) {
                state = 3;
                row << cell;
                break;
            }
            if (c == separator) {
                state = 0;
                row << cell;
                cell.clear();
            } else if (c == enclosure_char) {
                state = 1;
                cell.append(enclosure_char);
            } else {
                state = 0;
                cell.append(c);
            }
        }
    }
    if (state < 3) {
        row << cell;
    }
    return row;
}
