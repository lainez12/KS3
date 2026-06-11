#ifndef COMPLETEEXPOSUREVIEWMODEL_H
#define COMPLETEEXPOSUREVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{

    class CompleteExposureViewModel final : public QObject, public BaseViewModel
    {
        Q_OBJECT

    public:
        explicit CompleteExposureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~CompleteExposureViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Exposure

#endif