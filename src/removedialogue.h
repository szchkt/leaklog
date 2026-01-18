/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2026 Matus & Michal Tomlein

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

#ifndef REMOVEDIALOGUE_H
#define REMOVEDIALOGUE_H

#include <QInputDialog>
#include <QDialogButtonBox>
#include <QPushButton>

class RemoveDialogue : public QInputDialog
{
    Q_OBJECT

public:
    RemoveDialogue(QWidget *parent = 0):
    QInputDialog(parent), ok(NULL) {
        QObject::connect(this, SIGNAL(textValueChanged(QString)), this, SLOT(setAcceptEnabled(QString)));
    }

    static DialogCode confirm(QWidget *parent, const QString &title, const QString &label) {
        RemoveDialogue d(parent);
        d.setWindowTitle(title);
        d.setLabelText(label);
        d.setTextEchoMode(QLineEdit::Normal);
        d.setWindowModality(Qt::WindowModal);
        d.setWindowFlags(Qt::Sheet);

        d.show();
        d.findOkButton();

        return (DialogCode)d.exec();
    }

protected slots:
    void findOkButton() {
        QList<QDialogButtonBox *> boxes = findChildren<QDialogButtonBox *>();
        if (!boxes.isEmpty()) {
            ok = boxes.first()->button(QDialogButtonBox::Ok);
            ok->setEnabled(false);
        }
    }

    void setAcceptEnabled(const QString &text) {
        if (ok)
            ok->setEnabled(text == tr("REMOVE"));
    }

protected:
    QPushButton *ok;
};

#endif // REMOVEDIALOGUE_H
