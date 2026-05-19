#pragma once

#include <QMainWindow>
#include <memory>

#include <utils.h>

namespace Ui
{
    class MotorTestWindow;
}

namespace Kub3::Tools::MotorTester
{

    class MotorTestViewModel;

    class MotorTestWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        explicit MotorTestWindow(Shared<MotorTestViewModel> viewModel, QWidget *parent = nullptr);
        ~MotorTestWindow() override;

    private slots:
        void bindViewModel();
        void invokeJog(int direction);
        void invokeAbsoluteMove();

    private:
        Unique<Ui::MotorTestWindow> ui;
        Shared<MotorTestViewModel> m_viewModel;
    };

}