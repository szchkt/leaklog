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

HTMLParentElement & HTMLParentElement::operator<<(const QString & str)
{
    children.append(new HTMLDataElement(str));
    return *this;
}

HTMLParentElement & HTMLParentElement::operator<<(HTMLParentElement * child)
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
