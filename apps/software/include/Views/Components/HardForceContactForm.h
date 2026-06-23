#pragma once

#include <QEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWidget>

namespace Ui
{
    class HardForceContactForm;
} // namespace Ui

class HardForceContactForm : public QWidget
{
    Q_OBJECT
public:
    explicit HardForceContactForm(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

public slots:

private slots:

signals:

private:
    Ui::HardForceContactForm *ui;
    QString m_text;
};
