#pragma once

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels
{
    class HomeViewModel final : public QObject, public BaseViewModel
    {
        Q_OBJECT
    public:
        explicit HomeViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~HomeViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };
}