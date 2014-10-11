#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QLabel>
#include <QLineEdit>
#include <QResizeEvent>
#include <QToolButton>
#include <QStyle>

class SearchLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    SearchLineEdit(QWidget *parent = 0, bool show_icon = false, QString stylesheet = QString()):
    QLineEdit(parent) {
        if (show_icon) {
            setAttribute(Qt::WA_MacShowFocusRect, false);

            lbl_icon = new QLabel(this);
            lbl_icon->setPixmap(QIcon(":/images/images/find16.png").pixmap(16));
            lbl_icon->setMaximumSize(16, 16);
            lbl_icon->setMinimumSize(16, 16);
        } else {
            lbl_icon = NULL;
        }

        btn_clear = new QToolButton(this);
        if (layoutDirection() == Qt::LeftToRight) {
            btn_clear->setIcon(QIcon(":/images/images/clear.png"));
        } else {
            btn_clear->setIcon(QIcon(":/images/images/clear_rtl.png"));
        }
        btn_clear->setIconSize(QSize(13, 13));
        btn_clear->setCursor(Qt::ArrowCursor);
        btn_clear->setStyleSheet("QToolButton { border: none; padding: 0px; }");
        btn_clear->hide();

        QObject::connect(btn_clear, SIGNAL(clicked()), this, SLOT(clear()));
        QObject::connect(btn_clear, SIGNAL(clicked()), this, SIGNAL(returnPressed()));
        QObject::connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(setClearButtonVisible(const QString &)));
        QObject::connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(emitTextChanged(const QString &)));

        int frameWidth = lbl_icon ? 5 : style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        stylesheet.append(QString("QLineEdit { padding-left: %1px; padding-right: %2px; }")
                          .arg(lbl_icon ? 16 + frameWidth + 1 : 0)
                          .arg(btn_clear->sizeHint().width() + frameWidth + 1));
        setStyleSheet(stylesheet);

        frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        QSize minsize = minimumSizeHint();
        setMinimumSize(qMax(minsize.width(), btn_clear->sizeHint().height() + frameWidth * 2 + 2),
                        qMax(minsize.height(), btn_clear->sizeHint().height() + frameWidth * 2 + 2));
    }

protected:
    void resizeEvent(QResizeEvent *) {
        QSize btn_clear_size_hint = btn_clear->sizeHint();
        int frameWidth = lbl_icon ? 5 : style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        if (layoutDirection() == Qt::LeftToRight) {
            btn_clear->move(rect().right() - frameWidth - btn_clear_size_hint.width(),
                            (rect().bottom() + 1 - btn_clear_size_hint.height()) / 2);
            if (lbl_icon) {
                lbl_icon->move(frameWidth + 1, (rect().bottom() + 1 - 16) / 2);
            }
        } else {
            btn_clear->move(frameWidth, (rect().bottom() + 1 - btn_clear_size_hint.height()) / 2);
            if (lbl_icon) {
                lbl_icon->move(rect().right() - frameWidth - 16 - 1, (rect().bottom() + 1 - 16) / 2);
            }
        }
    }

private slots:
    void setClearButtonVisible(const QString &text) {
        btn_clear->setVisible(!text.isEmpty());
    }
    void emitTextChanged(const QString &text) {
        emit textChanged(this, text);
    }

signals:
    void textChanged(QLineEdit *, const QString &);

private:
    QToolButton *btn_clear;
    QLabel *lbl_icon;
};

#endif // SEARCHLINEEDIT_H
