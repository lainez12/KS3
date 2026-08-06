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

public slots:
    void ps_setMaximum(double maximumGF);
    void ps_setTolerance(double toleranceGF);
    void ps_setLock(bool lock);

signals:
    void s_startContactRoutine(double targetForceGrams);

private:
    Ui::HardForceContactForm *ui;
    Kub3::UI::Views::KeyboardConnections m_keyboard;
};
