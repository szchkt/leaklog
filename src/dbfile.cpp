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

#include "dbfile.h"
#include "global.h"

#include <QBuffer>
#include <QFile>
#include <QDataStream>
#include <QPixmap>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>

#define IMAGE_MAX_SIZE 800

DBFile::DBFile(int file_id):
File(QString::number(file_id))
{
    this->file_id = file_id;
}

void DBFile::setData(const QByteArray & file_data)
{
    if (Global::isDatabaseRemote())
        this->file_data = file_data.toBase64();
    else
        this->file_data = file_data;
}

void DBFile::setFileName(const QString & file_name)
{
    this->file_name = file_name;
    if (file_name.isEmpty())
        return;

    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly))
        return;

    setData(file.readAll());
}

void DBFile::setPixmap(const QString & file_name)
{
    this->file_name = file_name;
    if (file_name.isEmpty())
        return;

    QPixmap pixmap(file_name);
    if (pixmap.isNull())
        return;

    setPixmap(pixmap);
}

void DBFile::setPixmap(QPixmap & pixmap)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    if (pixmap.height() > IMAGE_MAX_SIZE || pixmap.width() > IMAGE_MAX_SIZE)
        pixmap = pixmap.scaled(QSize(IMAGE_MAX_SIZE, IMAGE_MAX_SIZE), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixmap.save(&buffer, "JPG", JPEG_QUALITY);
    buffer.close();
    setData(buffer.data());
}

bool DBFile::saveData(const QString & file_name)
{
    QByteArray data = this->data();
    if (data.isEmpty())
        return false;

    QFile file(file_name);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream.writeRawData(data.constData(), data.size());
    return true;
}

int DBFile::save()
{
    if (file_id <= 0) {
        file_id = File("").max("id") + 1;
        File::setId(QString::number(file_id));
    }

    QVariantMap update_map;
    update_map.insert("data", file_data);
    update_map.insert("name", file_name);

    update(update_map);

    return file_id;
}

QByteArray DBFile::data()
{
    if (!file_data.isNull() || file_id < 0)
        return Global::isDatabaseRemote() ? QByteArray::fromBase64(file_data) : file_data;

    if (Global::isDatabaseRemote())
        file_data = QByteArray::fromHex(list("data").value("data").toByteArray());
    else
        return file_data = list("data").value("data").toByteArray();

    return QByteArray::fromBase64(file_data);
}

QByteArray DBFile::dataToBase64()
{
    if (!file_data.isNull() || file_id < 0)
        return Global::isDatabaseRemote() ? file_data : file_data.toBase64();

    if (Global::isDatabaseRemote())
        return file_data = QByteArray::fromHex(list("data").value("data").toByteArray());
    else
        file_data = list("data").value("data").toByteArray();

    return file_data.toBase64();
}

DBFileChooser::DBFileChooser(QWidget * parent, int file_id):
QWidget(parent)
{
    db_file = new DBFile(file_id);
    changed = false;

    QHBoxLayout * layout = new QHBoxLayout(this);

    name_lbl = new QLabel(tr("Select an image"), this);
    layout->addWidget(name_lbl);

    QPushButton * browse_btn = new QPushButton(tr("Browse"), this);
    QObject::connect(browse_btn, SIGNAL(clicked()), this, SLOT(browse()));
    layout->addWidget(browse_btn);

    setLayout(layout);
}

void DBFileChooser::browse()
{
    QString file_name = QFileDialog::getOpenFileName(parentWidget(), tr("Open File"),
                                                     QDir::homePath(),
                                                     "Images (*.png *.jpg)");
    if (!file_name.isNull()) {
        name_lbl->setText(QFileInfo(file_name).fileName());
        db_file->setPixmap(file_name);
        changed = true;
    }
}

QVariant DBFileChooser::variantValue() const
{
    if (changed)
        return db_file->save();
    else
        return db_file->id().toInt();
}
