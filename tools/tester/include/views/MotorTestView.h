#pragma once

#include "ui_MotorTestView.h"

#include <memory>

#include <Views/ViewBase.h>
#include <view_models/MotorTestViewModel.h>

namespace Kub3::Tools::Tester
{

    class MotorTestView : public UI::Views::ViewBase
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
