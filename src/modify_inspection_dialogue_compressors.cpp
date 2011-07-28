#include "modify_inspection_dialogue_compressors.h"

#include "records.h"
#include "variables.h"
#include "modify_dialogue_layout.h"
#include "input_widgets.h"

#include <QVBoxLayout>
#include <QTabWidget>
#include <QVariantMap>

ModifyInspectionDialogueCompressors::ModifyInspectionDialogueCompressors(const QString & customer_id, const QString & circuit_id, const QString & inspection_date, QWidget * parent)
    : QWidget(parent),
      customer_id(customer_id),
      circuit_id(circuit_id)
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    tab_w = new QTabWidget(this);
    layout->addWidget(tab_w);

    loadTabs(inspection_date);
}

void ModifyInspectionDialogueCompressors::loadTabs(const QString & inspection_date)
{
    Compressor compressors_rec(QString(), MTDictionary(QStringList() << "customer_id" << "circuit_id",
                                                       QStringList() << customer_id << circuit_id));
    ListOfVariantMaps compressors = compressors_rec.listAll();

    for (int i = 0; i < compressors.count(); ++i) {
        InspectionCompressorTab * tab = addTab(compressors.at(i).value("id").toInt(), compressors.at(i).value("name").toString());

        if (!inspection_date.isEmpty()) {
            InspectionsCompressor inspection_compressor_rec(QString(), MTDictionary(QStringList() << "customer_id" << "circuit_id" << "date" << "compressor_id",
                QStringList() << customer_id << circuit_id << inspection_date << compressors.at(i).value("id").toString()));
            QVariantMap inspection_compressor = inspection_compressor_rec.list();
            if (!inspection_compressor.isEmpty()) {
                tab->setRecordId(inspection_compressor.value("id").toInt());
                former_ids.append(inspection_compressor.value("id").toInt());
                tab->init(inspection_compressor);
            } else {
                tab->init();
            }
        } else {
            tab->init();
        }
    }
}

InspectionCompressorTab * ModifyInspectionDialogueCompressors::addTab(int tab_id, const QString & tab_name)
{
    InspectionCompressorTab * tab = new InspectionCompressorTab(tab_id, this);
    tabs.append(tab);

    tab_w->addTab(tab, tab_name);

    return tab;
}

void ModifyInspectionDialogueCompressors::save(const QVariant & inspection_date)
{
    bool success = false;
    for (int i = 0; i < tabs.count(); ++i) {
        success = tabs.at(i)->save(customer_id, circuit_id, inspection_date.toString()) && success;
        if (tabs.at(i)->recordId() >= 0)
            former_ids.removeAll(tabs.at(i)->recordId());
    }
    for (int i = 0; i < former_ids.count(); ++i) {
        InspectionsCompressor record(QString::number(former_ids.at(i)));
        record.remove();
    }
    //return success;
}

InspectionCompressorTab::InspectionCompressorTab(int id, QWidget * parent)
    : QWidget(parent),
      m_id(id),
      m_record_id(-1)
{
}

void InspectionCompressorTab::init(const QVariantMap & var_values)
{
    Variables vars(QSqlDatabase(), Variable::Compressor);
    vars.initModifyDialogueWidgets(this, var_values);

    QGridLayout * layout = new QGridLayout(this);

    ModifyDialogueColumnLayout(&md_inputwidgets, layout, 5).layout();
}

bool InspectionCompressorTab::save(const QString & customer_id, const QString & circuit_id, const QString & inspection_date)
{
    MTDictionary parents(QStringList() << "customer_id" << "circuit_id" << "date" << "compressor_id",
                         QStringList() << customer_id << circuit_id << inspection_date << QString::number(m_id));
    InspectionsCompressor record;
    if (m_record_id >= 0)
        record = InspectionsCompressor(QString::number(m_record_id), parents);
    else
        record = InspectionsCompressor(QString(), parents);

    QVariantMap values;
    for (QList<MDAbstractInputWidget *>::const_iterator i = md_inputwidgets.constBegin(); i != md_inputwidgets.constEnd(); ++i) {
        if ((*i)->skipSave()) continue;
        values.insert((*i)->id(), (*i)->variantValue());
    }
    return record.update(values, true);
}
