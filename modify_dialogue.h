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

#include "mtdictionary.h"
#include "mtcolourcombobox.h"

#include <QDomDocument>
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
#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>

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

class MainWindow;

class ModifyDialogue : public QDialog
{
    Q_OBJECT

public:
    ModifyDialogue(const QDomElement &, QStringList, bool, MainWindow * = NULL);

private slots:
    void save();

private:
    MainWindow * md_parent;
    QDomElement md_element;
    MTDictionary md_dict;
    MTDictionary md_dict_input;
    MTDictionary md_dict_vars;
    QStringList md_used_ids;
    QMap<QString, QWidget *> md_vars;
};
