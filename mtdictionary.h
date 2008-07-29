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

#include <QStringList>

class MTDictionary
{
public:
    MTDictionary() {};
    MTDictionary(const MTDictionary & other) {
        dict_keys = other.dict_keys;
        dict_values = other.dict_values;
    };

    int count() { return dict_keys.count(); };
    void insert(const QString & key, const QString & value) {
        while (dict_keys.contains(key)) {
            int i = dict_keys.indexOf(key);
            dict_keys.removeAt(i);
            dict_values.removeAt(i);
        }
        dict_keys << key; dict_values << value;
    };
    QString key(int i) { if (i >= 0 && i < dict_keys.count()) return dict_keys.at(i); else return QString(); };
    QString firstKey(const QString & value) {
        return dict_values.indexOf(value) < 0 ? value : dict_keys.at(dict_values.indexOf(value));
    };
    bool contains(const QString & key) {
        return dict_keys.contains(key, Qt::CaseSensitive);
    };
    QString value(int i) { if (i >= 0 && i < dict_keys.count()) return dict_values.at(i); else return QString(); };
    QString value(const QString & key) {
        return dict_keys.indexOf(key) < 0 ? key : dict_values.at(dict_keys.indexOf(key));
    };
    void removeAt(int i) { if (i >= 0 && i < count()) { dict_keys.removeAt(i); dict_values.removeAt(i); } };
    void remove(const QString & key) {
        while (dict_keys.contains(key)) {
            int i = dict_keys.indexOf(key);
            dict_keys.removeAt(i);
            dict_values.removeAt(i);
        }
    };

private:
    QStringList dict_keys;
    QStringList dict_values;
};
