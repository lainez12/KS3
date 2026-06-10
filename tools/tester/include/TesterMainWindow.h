#pragma once

#include <QMainWindow>
#include <memory>

#include <utils.h>

namespace Ui
{
    class TesterMainWindow;
}

namespace Kub3::Tools::Tester
{
    class TesterMainWindow : public QMainWindow
    {
        Q_OBJECT
    public:
        explicit TesterMainWindow(QWidget *parent = nullptr);
        ~TesterMainWindow() override;

        // Uses QTabWidget under the hood
        void addView(const QString &tabTitle, QWidget *view);

    signals:
        void s_globalEmergencyStopRequested();

    private:
        Unique<Ui::TesterMainWindow> ui;
    };
}