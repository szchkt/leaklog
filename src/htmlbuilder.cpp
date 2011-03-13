/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2011 Matus & Michal Tomlein

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

#include "htmlbuilder.h"

#include <QTextStream>

HTMLParentElement::HTMLParentElement(const QString & args)
{
    this->args = args;
}

HTMLParentElement::~HTMLParentElement()
{
    for (int i = children.count() - 1; i >= 0; --i) {
        delete children.takeAt(i);
    }
}

HTMLBold * HTMLParentElement::bold()
{
    HTMLBold * b = new HTMLBold();
    children.append(b);
    return b;
}

HTMLItalics * HTMLParentElement::italics()
{
    HTMLItalics * i = new HTMLItalics();
    children.append(i);
    return i;
}

HTMLLink * HTMLParentElement::link(const QString & url, const QString & args)
{
    HTMLLink * a = new HTMLLink(url, args);
    children.append(a);
    return a;
}

HTMLTable * HTMLParentElement::table(const QString & args)
{
    HTMLTable * table = new HTMLTable(args);
    children.append(table);
    return table;
}

HTMLHeading * HTMLParentElement::heading()
{
    HTMLHeading * heading = new HTMLHeading();
    children.append(heading);
    return heading;
}

HTMLSubHeading * HTMLParentElement::subHeading()
{
    HTMLSubHeading * heading = new HTMLSubHeading();
    children.append(heading);
    return heading;
}

HTMLSubSubHeading * HTMLParentElement::subSubHeading()
{
    HTMLSubSubHeading * heading = new HTMLSubSubHeading();
    children.append(heading);
    return heading;
}

HTMLParagraph * HTMLParentElement::paragraph(const QString & args)
{
    HTMLParagraph * paragraph = new HTMLParagraph(args);
    children.append(paragraph);
    return paragraph;
}

HTMLParentElement & HTMLParentElement::operator<<(const QString & str)
{
    children.append(new HTMLDataElement(str));
    return *this;
}

HTMLParentElement & HTMLParentElement::operator<<(HTMLElement * child)
{
    children.append(child);
    return *this;
}

HTMLDataElement::HTMLDataElement(const QString & body)
{
    this->body = body;
}

const QString HTMLDataElement::html()
{
    return body;
}

const QString HTMLParentElement::html()
{
    QString str;
    QTextStream out(&str);

    out << "<" << tagName() << " " << args << ">";
    for (int i = 0; i < children.count(); ++i) {
        out << children.at(i)->html();
    }
    out << "</" << tagName() <<">";

    return str;
}

HTMLTable::HTMLTable(const QString & args):
HTMLParentElement(args)
{
    tag_name = "table";
}

const QString HTMLTable::customHtml(int cols_in_row)
{
    QString str;
    QTextStream out(&str);

    out << "<table " << args << ">";
    bool cols_left = true;
    int n = 0;
    while (cols_left) {
        for (int i = 0; i < children.count(); ++i) {
            out << ((HTMLTableRow *) children.at(i))->customHtml(n, cols_in_row, cols_left);
        }
        n += cols_in_row;
    }
    out << "</table>";

    return str;
}

HTMLTableRow * HTMLTable::addRow(const QString & row_args)
{
    HTMLTableRow * row = new HTMLTableRow(row_args);
    children.append(row);
    return row;
}

HTMLTableRow::HTMLTableRow(const QString & args):
HTMLParentElement(args)
{
    tag_name = "tr";
}

const QString HTMLTableRow::customHtml(int n, int cols_in_row, bool & cols_left)
{
    QString str;
    QTextStream out(&str);
    int i;

    out << "<tr " << args << ">";
    for (i = n; i < n + cols_in_row && i < children.count(); ++i) {
        out << children.at(i)->html();
    }
    cols_left = i < children.count();
    out << "</tr>";

    return str;
}

HTMLTableCell * HTMLTableRow::addCell(const QString & cell_args)
{
    HTMLTableCell * cell = new HTMLTableCell(cell_args);
    children.append(cell);
    return cell;
}

HTMLHeaderTableCell * HTMLTableRow::addHeaderCell(const QString & cell_args)
{
    HTMLHeaderTableCell * cell = new HTMLHeaderTableCell(cell_args);
    children.append(cell);
    return cell;
}

HTMLTableCell::HTMLTableCell(const QString & args):
HTMLParentElement(args)
{
    tag_name = "td";
}

HTMLHeaderTableCell::HTMLHeaderTableCell(const QString & args):
HTMLTableCell(args)
{
    tag_name = "th";
}

HTMLDiv::HTMLDiv(const QString & args):
HTMLParentElement(args)
{
    tag_name = "div";
}

HTMLBold::HTMLBold():
HTMLParentElement()
{
    tag_name = "b";
}

HTMLItalics::HTMLItalics():
HTMLParentElement()
{
    tag_name = "i";
}

HTMLLink::HTMLLink(const QString & url, const QString & args):
HTMLParentElement(QString("href=\"%1\" %2").arg(url).arg(args))
{
    tag_name = "a";
}

HTMLHeading::HTMLHeading():
HTMLParentElement()
{
    tag_name = "h1";
}

HTMLSubHeading::HTMLSubHeading():
HTMLParentElement()
{
    tag_name = "h2";
}

HTMLSubSubHeading::HTMLSubSubHeading():
HTMLParentElement()
{
    tag_name = "h3";
}

HTMLParagraph::HTMLParagraph(const QString & args):
HTMLParentElement(args)
{
    tag_name = "p";
}
