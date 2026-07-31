#pragma once

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

#include "BottomCroppedCircle.h"

class ActionBox : public BottomCroppedCircle
{
    Q_OBJECT
public:
    ActionBox(QWidget *parent = nullptr);

    void setup(const QString &text, const QColor &color, const QString &path = "");
    void setIcon(const QString &path);

signals:
    void clicked(void);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    QVBoxLayout *m_layout;
    QLabel *m_icon;
    QLabel *m_text;
    QString m_iconPath;
    QColor m_enabledColor;
    const QColor m_disabledColor = QColor(220, 220, 220);
};
