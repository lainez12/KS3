#ifndef SAVEEXPOSURESETTINGSVIEWMODEL_H
#define SAVEEXPOSURESETTINGSVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    class SaveExposureSettingsViewModel final : public BaseViewModel
    {
    public:
        explicit SaveExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~SaveExposureSettingsViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels

#endif