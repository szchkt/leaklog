/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008 Matus & Michal Tomlein

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

#ifndef MODIFY_DIALOGUE_H
#define MODIFY_DIALOGUE_H

#include "global.h"
#include "mtcolourcombobox.h"

#include <QTextDocument>
#include <QDateTimeEdit>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>

using namespace Global;

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(QStringList, QTextDocument * = 0);

protected:
    void highlightBlock(const QString &);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;
    QTextCharFormat keywordFormat;
};

class ModifyDialogue : public QDialog
{
    Q_OBJECT

public:
    ModifyDialogue(const MTRecord &, QWidget * = NULL);
    inline MTRecord record() { return md_record; };

private slots:
    virtual void save();

protected:
    void init(const MTRecord &, const QStringList &);
    ModifyDialogue(const MTRecord &, const QStringList &, QWidget * = NULL);

    QWidget * createInputWidget(const QStringList &, const QString &, const QString &);
    QVariant getInputFromWidget(QWidget *, const QStringList &, const QString &);

    MTRecord md_record;
    MTDictionary md_dict;
    MTDictionary md_dict_input;
    QStringList md_used_ids;
    QMap<QString, QWidget *> md_vars;
    StringVariantMap md_values;
    QGridLayout * md_grid_main;
};

#endif // MODIFY_DIALOGUE_H
