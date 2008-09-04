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

#include "mtdictionary.h"
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
#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>
#include <QSqlQuery>
#include <QSqlRecord>

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

class MTRecord : public QObject
{
    Q_OBJECT

public:
    MTRecord() {};
    MTRecord(const QString &, const QString &, const MTDictionary &);
    MTRecord(const MTRecord &);
    MTRecord & operator=(const MTRecord &);
    void setType(const QString & type) { r_type = type; };
    inline QString type() { return r_type; };
    inline QString id() { return r_id; };
    inline MTDictionary * parents() { return &r_parents; };
    QSqlQuery select(const QString & = "*");
    QMap<QString, QVariant> list(const QString & = "*");
    QList<QMap<QString, QVariant> > listAll(const QString & = "*");
    bool update(const QMap<QString, QVariant> &);
    bool remove();

protected:
    QString tableForRecordType(const QString &);

private:
    QString r_type;
    QString r_id;
    MTDictionary r_parents;
};

class ModifyWarningDialogue;

class ModifyDialogue : public QDialog
{
    Q_OBJECT

protected:
    void init(const MTRecord &, const QStringList &);
    ModifyDialogue(const MTRecord &, const QStringList &, QWidget * = NULL);

public:
    ModifyDialogue(const MTRecord &, QWidget * = NULL);
    inline MTRecord record() { return md_record; };

private:
    QWidget * createInputWidget(const QStringList &, const QString &, const QString &);
    QVariant getInputFromWidget(QWidget *, const QStringList &, const QString &);

private slots:
    virtual void save();

private:
    MTRecord md_record;
    MTDictionary md_dict;
    MTDictionary md_dict_input;
    QStringList md_used_ids;
    QMap<QString, QWidget *> md_vars;
    QGridLayout * md_grid_main;

    friend class ModifyWarningDialogue;
};

QString toString(const QVariant &);

#endif // MODIFY_DIALOGUE_H
