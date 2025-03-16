/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2025 Matus & Michal Tomlein

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
#include <QSettings>
#include <QWebEngineUrlRequestJob>

#define IMAGE_MAX_SIZE 1600

class DBFileBuffer : public QBuffer
{
public:
    DBFileBuffer(QByteArray *buf, QObject *parent): QBuffer(buf, parent), _buffer(buf) {}

    ~DBFileBuffer() {
        delete _buffer;
    }

private:
    QByteArray *_buffer;
};

void DBFileUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *job)
{
    QByteArray *byte_array = new QByteArray(DBFile(job->requestUrl().host()).data());
    if (byte_array->isNull()) {
        delete byte_array;
        job->fail(QWebEngineUrlRequestJob::UrlNotFound);
    } else {
        DBFileBuffer *buffer = new DBFileBuffer(byte_array, job);
        job->reply(QString("image/jpeg").toUtf8(), buffer);
    }
}

DBFile::DBFile(const QString &file_uuid):
    File(file_uuid)
{
}

void DBFile::setData(const QByteArray &file_data)
{
    this->file_data = file_data;
}

void DBFile::setFileName(const QString &file_name)
{
    this->file_name = file_name;
    if (file_name.isEmpty())
        return;

    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly))
        return;

    setData(file.readAll());
}

void DBFile::setImage(const QString &file_name)
{
    this->file_name = file_name;
    if (file_name.isEmpty())
        return;

    QImage image(file_name);
    if (image.isNull())
        return;

    setImage(image);
}

void DBFile::setImage(QImage &image)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    if (image.height() > IMAGE_MAX_SIZE || image.width() > IMAGE_MAX_SIZE)
        image = image.scaled(QSize(IMAGE_MAX_SIZE, IMAGE_MAX_SIZE), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    image.save(&buffer, "JPG", JPEG_QUALITY);
    buffer.close();
    setData(buffer.data());
}

bool DBFile::saveData(const QString &file_name)
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

void DBFile::save()
{
    QVariantMap update_map;
    update_map.insert("data", file_data);
    update_map.insert("name", file_name);

    update(update_map);
}

QByteArray DBFile::data()
{
    if (!file_data.isNull() || uuid().isEmpty())
        return file_data;

    file_data = list("data").value("data").toByteArray();
    return file_data;
}

DBFileChooser::DBFileChooser(const QString &file_uuid, QWidget *parent):
QWidget(parent)
{
    db_file = new DBFile(file_uuid);
    changed = false;

    QHBoxLayout *layout = new QHBoxLayout(this);

    if (!file_uuid.isEmpty()) {
        name_lbl = new QLabel(this);
        QPixmap pixmap;
        pixmap.loadFromData(db_file->data());
        name_lbl->setPixmap(pixmap.scaled(215, 160, Qt::KeepAspectRatio));
    } else {
        name_lbl = new QLabel(tr("Select an image"), this);
    }
    layout->addWidget(name_lbl);

    QPushButton *browse_btn = new QPushButton(file_uuid.isEmpty() ? tr("Browse") : tr("Replace"), this);
    QObject::connect(browse_btn, SIGNAL(clicked()), this, SLOT(browse()));
    layout->addWidget(browse_btn);

    setLayout(layout);
}

DBFileChooser::~DBFileChooser()
{
    delete db_file;
}

void DBFileChooser::browse()
{
    QSettings settings("SZCHKT", "Leaklog");
    QDir dir(settings.value("file_dialog_dir", QDir::homePath()).toString());
    QString file_name = QFileDialog::getOpenFileName(parentWidget(), tr("Open File"),
                                                     dir.absolutePath(),
                                                     "Images (*.png *.jpg)");
    if (!file_name.isNull()) {
        dir.setPath(file_name);
        dir.cdUp();
        settings.setValue("file_dialog_dir", dir.absolutePath());
        name_lbl->setPixmap(QPixmap(file_name).scaled(215, 160, Qt::KeepAspectRatio));
        db_file->setImage(file_name);
        changed = true;
    }
}

QVariant DBFileChooser::variantValue() const
{
    if (changed)
        db_file->save();

    return db_file->uuid();
}
