#pragma once

#include <QDialog>
#include <memory>

#include <utils.h>

namespace Ui
{
    class CamerasTestView;
}

namespace Kub3::UI::ViewModels
{
    class MachineStatusViewModel;
}

namespace Kub3::UI::Modules
{

    class CamerasTestView : public QDialog
    {
        Q_OBJECT
    public:
        explicit CamerasTestView(Shared<ViewModels::MachineStatusViewModel> viewModel, QWidget *parent = nullptr);
        ~CamerasTestView() override;

    private:
        Ui::CamerasTestView *ui;
        Shared<ViewModels::MachineStatusViewModel> m_viewModel;
    };

}
