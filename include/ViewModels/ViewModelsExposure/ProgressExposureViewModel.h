#ifndef PROGRESSEXPOSUREVIEWMODEL_H
#define PROGRESSEXPOSUREVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsExposure
{
    class ProgressExposureViewModel final : public QObject, public BaseViewModel
    {
        Q_OBJECT
    public:
        explicit ProgressExposureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~ProgressExposureViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels

#endif