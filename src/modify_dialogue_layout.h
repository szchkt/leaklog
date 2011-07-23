#ifndef MODIFY_DIALOGUE_LAYOUT_H
#define MODIFY_DIALOGUE_LAYOUT_H

#include <QList>
#include <QGridLayout>

class QWidget;

class MDAbstractInputWidget;

class ModifyDialogueLayout
{
public:
    ModifyDialogueLayout(QList<MDAbstractInputWidget *> *, QGridLayout *);

    virtual void layout();
    void addWidget(QWidget * widget, int row, int column, Qt::Alignment alignment = 0);
    void addWidget(QWidget * widget, int fromRow, int fromColumn, int rowSpan, int columnSpan, Qt::Alignment alignment = 0);

protected:
    QGridLayout * md_grid_main;
    QList<MDAbstractInputWidget *> * md_inputwidgets;

};

class ModifyDialogueColumnLayout : public ModifyDialogueLayout
{
public:
    ModifyDialogueColumnLayout(QList<MDAbstractInputWidget *> *, QGridLayout *, int = 20);

    void layout();

private:
    int rows_in_column;
};

#endif // MODIFY_DIALOGUE_LAYOUT_H
