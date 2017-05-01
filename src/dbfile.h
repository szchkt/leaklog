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

#ifndef DBFILE_H
#define DBFILE_H

#include "records.h"

#include <QWidget>

class QPixmap;
class QLabel;

class DBFile : public File
{
public:
    DBFile(const QString &file_uuid = QString());

    QByteArray data();
    bool saveData(const QString &);

    void setData(const QByteArray &file_data);
    void setFileName(const QString &);
    void setImage(const QString &);
    void setImage(QImage &);

    void save();

private:
    QString file_name;
    QByteArray file_data;
};

class DBFileChooser : public QWidget
{
    Q_OBJECT

public:
    DBFileChooser(const QString &file_uuid, QWidget *parent);
    ~DBFileChooser();

    virtual QVariant variantValue() const;

private slots:
    void browse();

private:
    QLabel *name_lbl;
    DBFile *db_file;
    bool changed;
};

#endif // DBFILE_H
