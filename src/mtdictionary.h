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

#ifndef MTDICTIONARY_H
#define MTDICTIONARY_H

#include <QStringList>

class MTDictionary
{
public:
    MTDictionary() { allow_duplicate_keys = false; }
    MTDictionary(const QString &key, const QString &value) {
        allow_duplicate_keys = false;
        dict_keys << key; dict_values << value;
    }
    MTDictionary(bool allow_duplicate_keys) { this->allow_duplicate_keys = allow_duplicate_keys; }
    MTDictionary(const QStringList &keys, const QStringList &values = QStringList()) {
        dict_keys = keys;
        if (values.isEmpty()) {
            dict_values = keys;
        } else {
            dict_values = values; int i;
            for (i = dict_values.count(); i < dict_keys.count(); ++i) {
                dict_values << dict_keys.at(i);
            }
            for (i = dict_keys.count(); i < dict_values.count(); ++i) {
                dict_keys << dict_values.at(i);
            }
        }
    }
    MTDictionary(const MTDictionary &other) {
        allow_duplicate_keys = other.allow_duplicate_keys;
        dict_keys = other.dict_keys;
        dict_values = other.dict_values;
    }

    inline void allowDuplicateKeys() { allow_duplicate_keys = true; }
    inline int count() const { return dict_keys.count(); }
    inline bool isEmpty() const { return dict_keys.isEmpty(); }
    void insert(const QString &key, const QString &value) {
        while (dict_keys.contains(key) && !allow_duplicate_keys) {
            int i = dict_keys.indexOf(key);
            dict_keys.removeAt(i);
            dict_values.removeAt(i);
        }
        dict_keys << key; dict_values << value;
    }
    void setValue(const QString &key, const QString &value) {
        if (contains(key)) { dict_values.replace(indexOfKey(key), value); }
        else { dict_keys << key; dict_values << value; }
    }
    inline QString key(int i) const {
        if (i >= 0 && i < dict_keys.count()) return dict_keys.at(i);
        else return QString();
    }
    inline QString firstKey(const QString &value) const {
        return dict_values.indexOf(value) < 0 ? value : dict_keys.at(dict_values.indexOf(value));
    }
    inline const QString &lastKey() const {
        return dict_keys.last();
    }
    inline QStringList keys() const { return dict_keys; }
    inline int indexOfKey(const QString &key) const { return dict_keys.indexOf(key); }
    inline bool contains(const QString &key) const {
        return dict_keys.contains(key, Qt::CaseSensitive);
    }
    inline QString value(int i) const {
        if (i >= 0 && i < dict_keys.count()) return dict_values.at(i);
        else return QString();
    }
    inline const QString &value(int i, const QString &default_value) const {
        if (i >= 0 && i < dict_keys.count()) return dict_values.at(i);
        else return default_value;
    }
    inline const QString &value(const QString &key) const {
        return dict_keys.indexOf(key) < 0 ? key : dict_values.at(dict_keys.indexOf(key));
    }
    inline const QString &value(const QString &key, const QString &default_value) const {
        return dict_keys.indexOf(key) < 0 ? default_value : dict_values.at(dict_keys.indexOf(key));
    }
    inline const QString &lastValue() const {
        return dict_values.last();
    }

    inline QStringList values() const { return dict_values; }
    inline int indexOfValue(const QString &value, int from = 0) const { return dict_values.indexOf(value, from); }
    inline int lastIndexOfValue(const QString &value, int from = -1) const { return dict_values.lastIndexOf(value, from); }
    void removeAt(int i) { if (i >= 0 && i < count()) { dict_keys.removeAt(i); dict_values.removeAt(i); } }
    void remove(const QString &key) {
        while (dict_keys.contains(key)) {
            int i = dict_keys.indexOf(key);
            dict_keys.removeAt(i);
            dict_values.removeAt(i);
        }
    }
    void clear() { dict_keys.clear(); dict_values.clear(); }
    MTDictionary &operator=(const MTDictionary &other) {
        allow_duplicate_keys = other.allow_duplicate_keys;
        dict_keys = other.dict_keys;
        dict_values = other.dict_values;
        return *this;
    }
    MTDictionary &swapKeysAndValues() {
        QStringList tmp = dict_keys;
        dict_keys = dict_values;
        dict_values = tmp;
        return *this;
    }

private:
    bool allow_duplicate_keys;
    QStringList dict_keys;
    QStringList dict_values;
};

#endif // MTDICTIONARY_H
