#pragma once

#include "ui_MotorTestView.h"

#include <memory>

#include <ViewModels/MotorTestViewModel.h>
#include <Views/ViewBase.h>

namespace Kub3::Tools::Tester
{

    class MotorTestView final : public UI::Views::ViewBase
    {
        Q_OBJECT
    public:
        explicit MotorTestView(Shared<MotorTestViewModel> vm, QWidget *parent = nullptr);
        ~MotorTestView() override;

    private slots:
        void invokeJog(int direction);
        void invokeAbsoluteMove();

    private:
        Unique<Ui::MotorTestView> ui;
        Shared<MotorTestViewModel> m_vm;
    };

}
