/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2018 Matus & Michal Tomlein

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

#ifndef REFRIGERANTRECORD_H
#define REFRIGERANTRECORD_H

#include "dbrecord.h"

class RefrigerantRecord : public DBRecord
{
    Q_OBJECT

public:
    RefrigerantRecord(const QString &uuid = QString());

    void initEditDialogue(EditDialogueWidgets *);
    bool checkValues(QWidget * = 0);

    inline QString date() { return stringValue("date"); }
    inline QString partner() { return stringValue("partner"); }
    inline QString partnerID() { return stringValue("partner_id"); }
    inline QString refrigerant() { return stringValue("refrigerant"); }
    inline QString batchNumber() { return stringValue("batch_number"); }
    inline void setBatchNumber(const QString &value) { setValue("batch_number", value); }
    inline double purchased() { return doubleValue("purchased"); }
    inline double purchasedRecovered() { return doubleValue("purchased_reco"); }
    inline double sold() { return doubleValue("sold"); }
    inline double soldRecovered() { return doubleValue("sold_reco"); }
    inline double regenerated() { return doubleValue("refr_rege"); }
    inline double disposedOf() { return doubleValue("refr_disp"); }
    inline double leaked() { return doubleValue("leaked"); }
    inline double leakedRecovered() { return doubleValue("leaked_reco"); }
    inline QString notes() { return stringValue("notes"); }
    inline void setNotes(const QString &value) { setValue("notes", value); }

    static QString tableName();
    static inline MTRecordQuery<RefrigerantRecord> query(const QVariantMap &parents = QVariantMap()) { return MTRecordQuery<RefrigerantRecord>(tableName(), parents); }
    static const ColumnList &columns();
    static const MTDictionary &attributes();
};

#endif // REFRIGERANTRECORD_H
