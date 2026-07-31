#ifndef PROGRESSEXPOSUREVIEWMODEL_H
#define PROGRESSEXPOSUREVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <Services/Exposure/IExposureService.h>
#include <ViewModels/Exposure/ExposureBaseViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    class ProgressExposureViewModel final : public ExposureBaseViewModel
    {
        Q_OBJECT
    public:
        explicit ProgressExposureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~ProgressExposureViewModel() override;

    public:
        PresetExposure getCurrentPreset() const;
        double getTemperature() const;
        void ui_launchExposure();

    signals:
        void s_launchExposure(const Services::ExposurePayload &payload);

    public slots:
        void ps_launchExposure(const PresetExposure &preset);

    private:
        void launchExposureToMasterFSM();

    private:
        PresetExposure m_currentPreset;
    };

} // namespace Kub3::UI::ViewModels

#endif