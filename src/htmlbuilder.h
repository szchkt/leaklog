#ifndef HTMLBUILDER_H
#define HTMLBUILDER_H

#include <QList>
#include <QString>

class HTMLTableRow;
class HTMLTableCell;
class HTMLHeaderTableCell;

class HTMLElement
{
public:
    virtual const QString html() = 0;
};

class HTMLDataElement : public HTMLElement
{
public:
    HTMLDataElement(const QString &);

    virtual const QString html();

protected:
    QString body;
};

class HTMLParentElement : public HTMLElement
{
public:
    HTMLParentElement(const QString & = QString());
    ~HTMLParentElement();

    virtual const QString html();
    inline const QString tagName() { return tag_name; }

    HTMLParentElement & operator<<(const QString &);
    HTMLParentElement & operator<<(HTMLParentElement *);

protected:
    QList<HTMLElement *> children;
    QString args;
    QString tag_name;
};

class HTMLTable : public HTMLParentElement
{
public:
    HTMLTable(const QString & = QString());

    HTMLTableRow * addRow(const QString & = QString());
};

class HTMLTableRow : public HTMLParentElement
{
public:
    HTMLTableRow(const QString & = QString());

    HTMLTableCell * addCell(const QString & = QString());
    HTMLHeaderTableCell * addHeaderCell(const QString & = QString());
};

class HTMLTableCell : public HTMLParentElement
{
public:
    HTMLTableCell(const QString & = QString());
};

class HTMLHeaderTableCell : public HTMLTableCell
{
public:
    HTMLHeaderTableCell(const QString & = QString());
};

class HTMLDiv : public HTMLParentElement
{
public:
    HTMLDiv(const QString & = QString());
};

#endif // HTMLBUILDER_H
