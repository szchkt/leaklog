#ifndef MODIFY_INSPECTION_DIALOGUE_COMPRESSORS_H
#define MODIFY_INSPECTION_DIALOGUE_COMPRESSORS_H

#include <QWidget>
#include <QString>

#include "modify_dialogue_widgets.h"

class QTabWidget;

class InspectionCompressorTab;

class ModifyInspectionDialogueCompressors : public QWidget
{
    Q_OBJECT

public:
    ModifyInspectionDialogueCompressors(const QString &, const QString &, const QString &, QWidget *);

private:
    InspectionCompressorTab * addTab(int, const QString &);
    void loadTabs(const QString &);

    QString customer_id;
    QString circuit_id;
    QTabWidget * tab_w;
    QList<InspectionCompressorTab *> tabs;
};

class InspectionCompressorTab : public QWidget, public ModifyDialogueWidgets
{
    Q_OBJECT

public:
    InspectionCompressorTab(int, QWidget *);

    void setWindowTitle(const QString &) {}

    QWidget * widget() { return this; }

    int id() { return m_id; }

private:
    int m_id;
};

#endif // MODIFY_INSPECTION_DIALOGUE_COMPRESSORS_H
