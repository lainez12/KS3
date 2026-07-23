#ifndef LEDTESTVIEWMODEL_H
#define LEDTESTVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Settings
{
    class LedTestViewModel final : public BaseViewModel
    {
    public:
        explicit LedTestViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~LedTestViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Settings

#endif