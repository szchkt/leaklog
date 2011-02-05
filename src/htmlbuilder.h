#ifndef HTMLBUILDER_H
#define HTMLBUILDER_H

#include <QList>
#include <QString>

class HTMLTableRow;
class HTMLTableCell;
class HTMLHeaderTableCell;

class HTMLTable
{
public:
    HTMLTable(const QString & = QString());
    ~HTMLTable();

    HTMLTableRow * addRow(const QString & = QString());

    const QString html();

private:
    QList<HTMLTableRow *> rows;
    QString args;
};

class HTMLTableRow
{
public:
    HTMLTableRow(const QString &);
    ~HTMLTableRow();

    HTMLTableCell * addCell(const QString & = QString());
    HTMLHeaderTableCell * addHeaderCell(const QString & = QString());

    const QString html();

private:
    QList<HTMLTableCell *> cells;
    QString args;
};

class HTMLTableCell
{
public:
    HTMLTableCell(const QString &);

    HTMLTableCell & operator<<(const QString &);

    const QString html();
    inline const QString tagName() { return tag_name; }

protected:
    QString args;
    QString body;
    QString tag_name;
};

class HTMLHeaderTableCell : public HTMLTableCell
{
public:
    HTMLHeaderTableCell(const QString &);
};

#endif // HTMLBUILDER_H
