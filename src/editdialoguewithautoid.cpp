#include "editdialoguewithautoid.h"

#include "records.h"

#include "inputwidgets.h"

void EditDialogueWithAutoId::save()
{
    MDAbstractInputWidget *id_iw = inputWidget(record()->idField());
    QString id = id_iw->variantValue().toString();
    if (id.isEmpty()) {
        MTRecord rec(record()->table(), record()->idField(), "", MTDictionary());
        if (m_max_id > 0)
            rec.setCustomWhere(QString("id < %1").arg(m_max_id));

        qlonglong next_id = rec.max("id") + (qint64)1;
        if (m_max_id > 0 && next_id > m_max_id)
            return;

        id_iw->setVariantValue(next_id);
    }

    EditDialogue::save();
}
