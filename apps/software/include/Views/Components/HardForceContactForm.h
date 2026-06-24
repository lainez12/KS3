#pragma once

#include <QEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWidget>
#include <Views/KeyboardConnections.h>

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
public slots:

private slots:

signals:

private:
    Ui::HardForceContactForm *ui;
    Kub3::UI::Views::KeyboardConnections m_keyboard;
};
