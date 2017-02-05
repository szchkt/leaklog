/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2017 Matus & Michal Tomlein

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

#ifndef EDIT_INSPECTION_DIALOGUE_COMPRESSORS_H
#define EDIT_INSPECTION_DIALOGUE_COMPRESSORS_H

#include <QWidget>
#include <QString>
#include <QVariantMap>

#include "editdialoguewidgets.h"
#include "tabbededitdialogue.h"

class QTabWidget;

class InspectionCompressorTab;

class EditInspectionDialogueCompressors : public QWidget, public EditDialogueArea
{
    Q_OBJECT

public:
    EditInspectionDialogueCompressors(const QString &, const QString &, const QString &, QWidget *);

    void clearOriginalInspectionDate() { original_inspection_date.clear(); }

    void save(const QVariant &);

private:
    InspectionCompressorTab *addTab(int, const QString &);
    void loadTabs(const QString &);

    QString customer_id;
    QString circuit_id;
    QString original_inspection_date;
    QTabWidget *tab_w;
    QList<InspectionCompressorTab *> tabs;
    QList<int> former_ids;
};

class InspectionCompressorTab : public QWidget, public EditDialogueWidgets
{
    Q_OBJECT

public:
    InspectionCompressorTab(int, QWidget *);
    void init(const QVariantMap & = QVariantMap());

    void setWindowTitle(const QString &) {}
    bool save(const QString &, const QString &, const QString &, const QString &);

    QWidget *widget() { return this; }

    int id() { return m_id; }

    void setRecordId(int record_id) { m_record_id = record_id; }
    int recordId() { return m_record_id; }

private:
    int m_id;
    int m_record_id;
};

#endif // EDIT_INSPECTION_DIALOGUE_COMPRESSORS_H
