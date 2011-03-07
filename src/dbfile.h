#ifndef DBFILE_H
#define DBFILE_H

class QPixmap;
class QLabel;

#include "records.h"

#include <QWidget>

class DBFile : public File
{
public:
    DBFile(int = -1);

    const QByteArray & data();
    bool saveData(const QString &);

    void setData(const QByteArray & file_data) { this->file_data = file_data;}
    void setFileName(const QString &);
    void setPixmap(const QString &);
    void setPixmap(QPixmap &);

    int save();

private:
    int file_id;
    QString file_name;
    QByteArray file_data;
};

class DBFileChooser : public QWidget
{
    Q_OBJECT

public:
    DBFileChooser(QWidget *, int);

    virtual QVariant variantValue();

private slots:
    void browse();

private:
    QLabel * name_lbl;
    DBFile * db_file;
    bool changed;
};

#endif // DBFILE_H
