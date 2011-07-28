#ifndef MODIFY_INSPECTION_DIALOGUE_COMPRESSORS_H
#define MODIFY_INSPECTION_DIALOGUE_COMPRESSORS_H

#include <QWidget>
#include <QString>
#include <QVariantMap>

#include "modify_dialogue_widgets.h"
#include "tabbed_modify_dialogue.h"

class QTabWidget;

class InspectionCompressorTab;

class ModifyInspectionDialogueCompressors : public QWidget, public ModifyDialogueArea
{
    Q_OBJECT

public:
    ModifyInspectionDialogueCompressors(const QString &, const QString &, const QString &, QWidget *);

    void save(const QVariant &);

private:
    InspectionCompressorTab * addTab(int, const QString &);
    void loadTabs(const QString &);

    QString customer_id;
    QString circuit_id;
    QTabWidget * tab_w;
    QList<InspectionCompressorTab *> tabs;
    QList<int> former_ids;
};

class InspectionCompressorTab : public QWidget, public ModifyDialogueWidgets
{
    Q_OBJECT

public:
    InspectionCompressorTab(int, QWidget *);
    void init(const QVariantMap & = QVariantMap());

    void setWindowTitle(const QString &) {}
    bool save(const QString &, const QString &, const QString &);

    QWidget * widget() { return this; }

    int id() { return m_id; }

    void setRecordId(int record_id) { m_record_id = record_id; }
    int recordId() { return m_record_id; }

private:
    int m_id;
    int m_record_id;
};

#endif // MODIFY_INSPECTION_DIALOGUE_COMPRESSORS_H
