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

#ifndef HTMLBUILDER_H
#define HTMLBUILDER_H

#include <QStringList>

class HTMLTable;
class HTMLTableRow;
class HTMLTableCell;
class HTMLHeaderTableCell;
class HTMLTableHead;
class HTMLTableBody;
class HTMLTableFoot;
class HTMLBold;
class HTMLItalics;
class HTMLLink;
class HTMLHeading;
class HTMLSubHeading;
class HTMLSubSubHeading;
class HTMLParagraph;
class HTMLStyle;
class QTextStream;

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

class HTMLParent : public HTMLElement
{
public:
    HTMLParent();
    ~HTMLParent();

    virtual void html(QTextStream &);
    virtual const QString html();

    HTMLBold * bold();
    HTMLItalics * italics();
    HTMLLink * link(const QString &, const QString & = QString());
    virtual HTMLTable * table(const QString & = QString(), int = 0);
    HTMLHeading * heading();
    HTMLSubHeading * subHeading();
    HTMLSubSubHeading * subSubHeading();
    HTMLParagraph * paragraph(const QString & = QString());
    HTMLStyle * addStyleElement(const QString & = QString());

    void newLine() { children.append(new HTMLDataElement("<br>")); }

    HTMLParent & operator<<(const QString &);
    HTMLParent & operator<<(HTMLElement *);

protected:
    QList<HTMLElement *> children;
    QString args;
};

class HTMLParentElement : public HTMLParent
{
public:
    HTMLParentElement(const QString & = QString());

    inline const QString tagName() { return tag_name; }
    virtual const QString html();

    const QString & id() { return tag_id; }
    void setId(const QString & tag_id) { this->tag_id = tag_id; }
    void addClass(const QString & tag_class) { tag_classes.append(tag_class); }
    const QString classesInString() { return tag_classes.join(" "); }
    const QStringList & classes() { return tag_classes; }

protected:
    QString tag_id;
    QStringList tag_classes;
    QString tag_name;
};

class HTMLCustomTaggedParentElement : public HTMLParentElement
{
public:
    HTMLCustomTaggedParentElement(const QString & tag_name, const QString & args = QString()) : HTMLParentElement(args)
        { this->tag_name = tag_name; }
};

class HTMLDocument : public HTMLParentElement
{
public:
    HTMLDocument(const QString &);

    HTMLParentElement * body() { return html_body; }
    HTMLParentElement * head() { return html_head; }

private:
    HTMLParentElement * html_body;
    HTMLParentElement * html_head;
};

class HTMLMain : public HTMLParent
{
public:
    HTMLMain() : HTMLParent() {}
};

class HTMLDivMain : public HTMLParent
{
public:
    HTMLDivMain() : HTMLParent() {}

    HTMLTable * table(const QString & = QString(), int = 0);
};

class HTMLTable : public HTMLParentElement
{
public:
    HTMLTable(const QString & = QString(), int = 0);

    virtual HTMLTableRow * addRow(const QString & = QString());

    const QString html();
    const QString customHtml(int);

    HTMLTableHead * thead(const QString & = QString());
    HTMLTableBody * tbody(const QString & = QString());
    HTMLTableFoot * tfoot(const QString & = QString());

protected:
    int cols_in_row;
};

class HTMLTableHead : public HTMLTable
{
public:
    HTMLTableHead(const QString & = QString(), int = 0);
};

class HTMLTableBody : public HTMLTable
{
public:
    HTMLTableBody(const QString & = QString(), int = 0);
};

class HTMLTableFoot : public HTMLTable
{
public:
    HTMLTableFoot(const QString & = QString(), int = 0);
};

class HTMLTableRow : public HTMLParentElement
{
public:
    HTMLTableRow(const QString & = QString());

    virtual HTMLTableCell * addCell(const QString & = QString());
    virtual HTMLHeaderTableCell * addHeaderCell(const QString & = QString());

    const QString customHtml(int, int, bool &);
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

class HTMLDivTable : public HTMLTable
{
public:
    HTMLDivTable(const QString & = QString(), int = 0);

    HTMLTableRow * addRow(const QString & = QString());
};

class HTMLDivTableRow : public HTMLTableRow
{
public:
    HTMLDivTableRow(const QString & = QString());

    HTMLTableCell * addCell(const QString & = QString());
    HTMLHeaderTableCell * addHeaderCell(const QString & = QString());
};

class HTMLDivTableCell : public HTMLTableCell
{
public:
    HTMLDivTableCell(const QString & = QString());
};

class HTMLDivHeaderTableCell : public HTMLHeaderTableCell
{
public:
    HTMLDivHeaderTableCell(const QString & = QString());
};

class HTMLBold : public HTMLParentElement
{
public:
    HTMLBold();
};

class HTMLItalics : public HTMLParentElement
{
public:
    HTMLItalics();
};

class HTMLLink : public HTMLParentElement
{
public:
    HTMLLink(const QString &, const QString & = QString());
};

class HTMLHeading : public HTMLParentElement
{
public:
    HTMLHeading();
};

class HTMLSubHeading : public HTMLParentElement
{
public:
    HTMLSubHeading();
};

class HTMLSubSubHeading : public HTMLParentElement
{
public:
    HTMLSubSubHeading();
};

class HTMLParagraph : public HTMLParentElement
{
public:
    HTMLParagraph(const QString & = QString());
};

class HTMLStyle : public HTMLParentElement
{
public:
    HTMLStyle();
};

#endif // HTMLBUILDER_H
