#ifndef PROGRESSEXPOSUREVIEWMODEL_H
#define PROGRESSEXPOSUREVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/Exposure/ExposureBaseViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    class ProgressExposureViewModel final : public ExposureBaseViewModel
    {
    public:
        explicit ProgressExposureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~ProgressExposureViewModel() override;

    public slots:
        void ps_launchExposure(const PresetExposure &preset);

    private:
    };

} // namespace Kub3::UI::ViewModels

#endif