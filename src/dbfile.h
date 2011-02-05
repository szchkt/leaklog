#ifndef DBFILE_H
#define DBFILE_H

class QPixmap;

#include "records.h"

class DBFile : public File
{
public:
    DBFile(int = -1);

    const QByteArray & data();
    bool saveData(const QString &);

    void setData(const QByteArray & file_data) { this->file_data = file_data;}
    void setFileName(const QString &);
    void setPixmap(const QPixmap &);

    int save();

private:
    int file_id;
    QString file_name;
    QByteArray file_data;
};

#endif // DBFILE_H
