#include "dbfile.h"
#include <QBuffer>
#include <QFile>
#include <QDataStream>
#include <QPixmap>

DBFile::DBFile(int file_id):
File(QString::number(file_id))
{
    this->file_id = file_id;
}

void DBFile::setFileName(const QString & file_name)
{
    this->file_name = file_name;
    if (file_name.isEmpty())
        return;

    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly))
        return;

    file_data = file.readAll();
}

void DBFile::setPixmap(const QPixmap & pixmap)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    file_data = buffer.data();
}

bool DBFile::saveData(const QString & file_name)
{
    if (file_data.isEmpty()){
        return false;
    }

    QFile file(file_name);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream.writeRawData(file_data.constData(), file_data.size());
    return true;
}

int DBFile::save()
{
    if (file_id <= 0) {
        QVariantMap max_id_map = list("MAX(id) AS max");
        file_id = max_id_map.value("max").toInt() + 1;
        File::setId(QString::number(file_id));
    }

    QVariantMap update_map;
    update_map.insert("data", file_data);
    update_map.insert("name", file_name);

    update(update_map);

    return file_id;
}

const QByteArray & DBFile::data()
{
    if (!file_data.isNull() || file_id < 0) return file_data;

    file_data = list().value("data").toByteArray();
    return file_data;
}
