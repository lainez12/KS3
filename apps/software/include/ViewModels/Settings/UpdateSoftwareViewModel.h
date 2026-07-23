#ifndef UPDATESOFTWAREVIEWMODEL_H
#define UPDATESOFTWAREVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Settings
{
    class UpdateSoftwareViewModel final : public BaseViewModel
    {
    public:
        explicit UpdateSoftwareViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~UpdateSoftwareViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Settings

#endif