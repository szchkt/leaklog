#ifndef MODIFY_DIALOGUE_WIDGETS_H
#define MODIFY_DIALOGUE_WIDGETS_H

#include <QStringList>

class MDAbstractInputWidget;

class ModifyDialogueWidgets
{
public:
    void addInputWidget(MDAbstractInputWidget * iw) { md_inputwidgets << iw; }
    void addGroupedInputWidgets(const QString &, const QList<MDAbstractInputWidget *> &);

    void setUsedIds(const QStringList & ids) { md_used_ids = ids; }

    virtual void setWindowTitle(const QString &) = 0;

    virtual QWidget * widget() = 0;

protected:
    QStringList md_used_ids;
    QList<MDAbstractInputWidget *> md_inputwidgets;
};

#endif // MODIFY_DIALOGUE_WIDGETS_H
