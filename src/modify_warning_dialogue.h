/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2010 Matus & Michal Tomlein

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

#ifndef MODIFY_WARNING_DIALOGUE_H
#define MODIFY_WARNING_DIALOGUE_H

#include "modify_dialogue.h"
#include "mtdictionary.h"
#include "mtwidgetpalettes.h"

class WarningRecord;
class QComboBox;
class QLineEdit;
class QPlainTextEdit;

class AttributeFilters;

class AttributeFilter : public QWidget
{
    Q_OBJECT

public:
    AttributeFilter(AttributeFilters *);

private slots:
    void remove();

signals:
    void removeFilter(AttributeFilter *);

private:
    AttributeFilters * parent;
    QComboBox * attribute;
    QComboBox * function;
    QLineEdit * value;

    friend class AttributeFilters;
};

class AttributeFilters : public QWidget
{
    Q_OBJECT

public:
    AttributeFilters(QWidget *);

    int count();
    QString attribute(int);
    QString function(int);
    QString value(int);
    void add(const QString &, const QString &, const QString &);

public slots:
    AttributeFilter * add();
    void remove(AttributeFilter *);

private:
    QList<AttributeFilter *> af_filters;
    MTDictionary af_attributes;
    QStringList af_functions;
    QVBoxLayout * af_vlayout_main;
};

class Conditions;

class Condition : public QWidget
{
    Q_OBJECT

public:
    Condition(Conditions *);

private slots:
    void remove();

signals:
    void removeCondition(Condition *);

private:
    Conditions * parent;
    QPlainTextEdit * expression_ins;
    QComboBox * function;
    QPlainTextEdit * expression_nom;

    friend class Conditions;
};

class Conditions : public QWidget
{
    Q_OBJECT

public:
    Conditions(const QStringList &, QWidget *);

    int count();
    QString expressionIns(int);
    QString function(int);
    QString expressionNom(int);
    void add(const QString &, const QString &, const QString &);

public slots:
    Condition * add();
    void remove(Condition *);

private:
    QList<Condition *> c_conditions;
    QStringList c_used_ids;
    QStringList c_functions;
    QVBoxLayout * c_vlayout_main;
    SearchLineEditPalettes searchLineEditPalettes;
};

class ModifyWarningDialogue : public ModifyDialogue
{
    Q_OBJECT

public:
    ModifyWarningDialogue(WarningRecord *, QWidget * = NULL);

    void setWindowTitle(const QString &);

protected slots:
    void save();

private:
    AttributeFilters * md_filters;
    Conditions * md_conditions;

    friend class WarningRecord;
};

#endif // MODIFY_WARNING_DIALOGUE_H
