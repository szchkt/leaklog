/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2023 Matus & Michal Tomlein

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
#include "inspectioncompressor.h"

class QTabWidget;

class InspectionCompressorTab;

class EditInspectionDialogueCompressors : public QWidget, public EditDialogueArea
{
    Q_OBJECT

public:
    EditInspectionDialogueCompressors(const QString &customer_uuid, const QString &circuit_uuid, const QString &inspection_uuid, bool duplicate, QWidget *parent);

    void save(const QString &inspection_uuid);

private:
    InspectionCompressorTab *addTab(const InspectionCompressor &inspection_compressor, const QString &name);
    void loadTabs(const QString &inspection_uuid, bool duplicate);

    QString customer_uuid;
    QString circuit_uuid;
    QTabWidget *tab_w;
    QList<InspectionCompressorTab *> tabs;
    QList<int> former_ids;
};

class InspectionCompressorTab : public QWidget, public EditDialogueWidgets
{
    Q_OBJECT

public:
    InspectionCompressorTab(const InspectionCompressor &inspection_compressor, QWidget *);
    void init(const QVariantMap & = QVariantMap());

    void setWindowTitle(const QString &) {}
    bool save(const QString &inspection_uuid);

    QWidget *widget() { return this; }

    QWidget *parentWidget() { return QWidget::parentWidget(); }

private:
    InspectionCompressor inspection_compressor;
};

#endif // EDIT_INSPECTION_DIALOGUE_COMPRESSORS_H
