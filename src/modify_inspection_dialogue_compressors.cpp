#include "modify_inspection_dialogue_compressors.h"

#include "records.h"
#include "variables.h"
#include "modify_dialogue_layout.h"

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
        addTab(compressors.at(i).value("id").toInt(), compressors.at(i).value("name").toString());
    }
}

InspectionCompressorTab * ModifyInspectionDialogueCompressors::addTab(int tab_id, const QString & tab_name)
{
    InspectionCompressorTab * tab = new InspectionCompressorTab(tab_id, this);
    tabs.append(tab);

    tab_w->addTab(tab, tab_name);

    return tab;
}

InspectionCompressorTab::InspectionCompressorTab(int id, QWidget * parent)
    : QWidget(parent),
      m_id(id)
{
    Variables vars(QSqlDatabase(), true, Variable::Compressor);
    vars.initModifyDialogueWidgets(this, QVariantMap());

    QGridLayout * layout = new QGridLayout(this);

    ModifyDialogueColumnLayout(&md_inputwidgets, layout, 5).layout();
}
