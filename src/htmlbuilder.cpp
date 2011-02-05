#include "htmlbuilder.h"

#include <QTextStream>

HTMLTable::HTMLTable(const QString & args)
{
    this->args = args;
}

HTMLTable::~HTMLTable()
{
    for (int i = rows.count() - 1; i >= 0; --i) {
        delete rows.takeAt(i);
    }
}

HTMLTableRow * HTMLTable::addRow(const QString & row_args)
{
    HTMLTableRow * row = new HTMLTableRow(row_args);
    rows.append(row);
    return row;
}

const QString HTMLTable::html()
{
    QString str;
    QTextStream out(&str);

    out << "<table " << args << ">";
    for (int i = 0; i < rows.count(); ++i) {
        out << rows.at(i)->html();
    }
    out << "</table>";

    return str;
}

HTMLTableRow::HTMLTableRow(const QString & args)
{
    this->args = args;
}

HTMLTableRow::~HTMLTableRow()
{
    for (int i = cells.count() - 1; i >= 0; --i) {
        delete cells.takeAt(i);
    }
}

HTMLTableCell * HTMLTableRow::addCell(const QString & cell_args)
{
    HTMLTableCell * cell = new HTMLTableCell(cell_args);
    cells.append(cell);
    return cell;
}

HTMLHeaderTableCell * HTMLTableRow::addHeaderCell(const QString & cell_args)
{
    HTMLHeaderTableCell * cell = new HTMLHeaderTableCell(cell_args);
    cells.append(cell);
    return cell;
}

const QString HTMLTableRow::html()
{
    QString str;
    QTextStream out(&str);

    out << "<tr " << args << ">";
    for (int i = 0; i < cells.count(); ++i) {
        out << cells.at(i)->html();
    }
    out << "</tr>";

    return str;
}

HTMLTableCell::HTMLTableCell(const QString & args)
{
    this->args = args;
    tag_name = "td";
}

HTMLTableCell & HTMLTableCell::operator<< (const QString & str)
{
    body.append(str);

    return *this;
}

const QString HTMLTableCell::html()
{
    QString str;
    QTextStream out(&str);

    out << "<" << tagName() << " " << args << ">";
    out << body;
    out << "</" << tagName() <<">";

    return str;
}

HTMLHeaderTableCell::HTMLHeaderTableCell(const QString & args):
HTMLTableCell(args)
{
    tag_name = "th";
}
