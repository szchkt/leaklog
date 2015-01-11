/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2015 Matus & Michal Tomlein

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

#ifndef MTCHECKBOXGROUP_H
#define MTCHECKBOXGROUP_H

#include <QCheckBox>

class MTCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    MTCheckBox(QWidget *parent = NULL):
    QCheckBox(parent) {
        QObject::connect(this, SIGNAL(toggled(bool)), this, SLOT(emitToggled(bool)));
    }
    MTCheckBox(const QString &text, QWidget *parent = NULL):
    QCheckBox(text, parent) {
        QObject::connect(this, SIGNAL(toggled(bool)), this, SLOT(emitToggled(bool)));
    }

signals:
    void toggled(MTCheckBox *, bool);

private slots:
    void emitToggled(bool checked) {
        emit toggled(this, checked);
    }
};

class MTCheckBoxGroup : public QObject
{
    Q_OBJECT

public:
    MTCheckBoxGroup(QObject *parent = NULL):
    QObject(parent) {}

    void addCheckBox(MTCheckBox *chb) {
        QObject::connect(chb, SIGNAL(toggled(MTCheckBox *, bool)), this, SLOT(updateCheckStates(MTCheckBox *, bool)));
        checkboxes << chb;
    }

private slots:
    void updateCheckStates(MTCheckBox *chb, bool checked) {
        if (!checked) return;
        for (QList<MTCheckBox *>::const_iterator i = checkboxes.constBegin(); i != checkboxes.constEnd(); ++i) {
            if (*i != chb)
                (*i)->setChecked(false);
        }
    }

private:
    QList<MTCheckBox *> checkboxes;
};

#endif // MTCHECKBOXGROUP_H
