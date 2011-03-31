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

HTMLParent::HTMLParent()
{}

HTMLParent::~HTMLParent()
{
    for (int i = children.count() - 1; i >= 0; --i) {
        delete children.takeAt(i);
    }
}

HTMLBold * HTMLParent::bold()
{
    HTMLBold * b = new HTMLBold();
    children.append(b);
    return b;
}

HTMLItalics * HTMLParent::italics()
{
    HTMLItalics * i = new HTMLItalics();
    children.append(i);
    return i;
}

HTMLLink * HTMLParent::link(const QString & url, const QString & args)
{
    HTMLLink * a = new HTMLLink(url, args);
    children.append(a);
    return a;
}

HTMLTable * HTMLParent::table(const QString & args, int cols_in_row)
{
    HTMLTable * table = new HTMLTable(args, cols_in_row);
    children.append(table);
    return table;
}

HTMLHeading * HTMLParent::heading()
{
    HTMLHeading * heading = new HTMLHeading();
    children.append(heading);
    return heading;
}

HTMLSubHeading * HTMLParent::subHeading()
{
    HTMLSubHeading * heading = new HTMLSubHeading();
    children.append(heading);
    return heading;
}

HTMLSubSubHeading * HTMLParent::subSubHeading()
{
    HTMLSubSubHeading * heading = new HTMLSubSubHeading();
    children.append(heading);
    return heading;
}

HTMLParagraph * HTMLParent::paragraph(const QString & args)
{
    HTMLParagraph * paragraph = new HTMLParagraph(args);
    children.append(paragraph);
    return paragraph;
}

HTMLParent & HTMLParent::operator<<(const QString & str)
{
    children.append(new HTMLDataElement(str));
    return *this;
}

HTMLParent & HTMLParent::operator<<(HTMLElement * child)
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

void HTMLParent::html(QTextStream & out)
{
    for (int i = 0; i < children.count(); ++i) {
        out << children.at(i)->html();
    }
}

const QString HTMLParent::html()
{
    QString str;
    QTextStream out(&str);

    html(out);

    return str;
}

HTMLParentElement::HTMLParentElement(const QString & args):
HTMLParent()
{
    this->args = args;
}

HTMLTable * HTMLDivMain::table(const QString & args, int cols_in_row)
{
    HTMLTable * table = new HTMLDivTable(args, cols_in_row);
    children.append(table);
    return table;
}

const QString HTMLParentElement::html()
{
    QString str;
    QTextStream out(&str);

    out << "<" << tagName() << " " << args;
    if (!id().isEmpty()) out << " id=\"" << id() << "\"";
    QString classes_str = classesInString();
    if (!classes_str.isEmpty()) out << " class=\"" << classes_str << "\"";
    out << ">";

    HTMLParent::html(out);

    out << "</" << tagName() <<">";

    return str;
}

HTMLTable::HTMLTable(const QString & args, int cols_in_row):
HTMLParentElement(args),
cols_in_row(cols_in_row)
{
    tag_name = "table";
}

const QString HTMLTable::html()
{
    if (cols_in_row)
        return customHtml(cols_in_row);
    else
        return HTMLParentElement::html();
}

const QString HTMLTable::customHtml(int cols_in_row)
{
    QString str;
    QTextStream out(&str);

    out << "<" << tag_name << " " << args << ">";
    bool cols_left = true;
    int n = 0;
    while (cols_left) {
        for (int i = 0; i < children.count(); ++i) {
            out << ((HTMLTableRow *) children.at(i))->customHtml(n, cols_in_row, cols_left);
        }
        n += cols_in_row;
    }
    out << "</" << tag_name << ">";

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

    out << "<" << tag_name << " " << args << ">";
    for (i = n; i < n + cols_in_row && i < children.count(); ++i) {
        out << children.at(i)->html();
    }
    cols_left = i < children.count();
    out << "</" << tag_name << ">";

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

HTMLDivTable::HTMLDivTable(const QString & args, int cols_in_row):
HTMLTable(args, cols_in_row)
{
    tag_name = "div";
    addClass("table");
}

HTMLTableRow * HTMLDivTable::addRow(const QString & row_args)
{
    HTMLDivTableRow * row = new HTMLDivTableRow(row_args);
    children.append(row);
    return row;
}

HTMLDivTableRow::HTMLDivTableRow(const QString & args):
HTMLTableRow(args)
{
    tag_name = "div";
    addClass("table_row");
}

HTMLTableCell * HTMLDivTableRow::addCell(const QString & cell_args)
{
    HTMLDivTableCell * cell = new HTMLDivTableCell(cell_args);
    children.append(cell);
    return cell;
}

HTMLHeaderTableCell * HTMLDivTableRow::addHeaderCell(const QString & cell_args)
{
    HTMLDivHeaderTableCell * cell = new HTMLDivHeaderTableCell(cell_args);
    children.append(cell);
    return cell;
}

HTMLDivTableCell::HTMLDivTableCell(const QString & args):
HTMLTableCell(args)
{
    tag_name = "div";
    addClass("table_cell");
}

HTMLDivHeaderTableCell::HTMLDivHeaderTableCell(const QString & args):
HTMLHeaderTableCell(args)
{
    tag_name = "div";
    addClass("table_header_cell");
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
