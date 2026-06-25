#pragma once

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels
{
    class ExposureMenuViewModel final : public QObject, public BaseViewModel
    {
        Q_OBJECT
    public:
        explicit ExposureMenuViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~ExposureMenuViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };
}