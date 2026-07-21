#pragma once

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    class ExposureBaseViewModel : public QObject, public BaseViewModel
    {
        Q_OBJECT

    public:
        enum class ExposureMode
        {
            Continuous,
            Flashing,
        };

        struct Duration {
            int minutes = 0;
            int seconds = 0;
        };

        struct ContinuousSettings {
            Duration duration;
            int power = 0;
        };

        struct FlashingSettings {
            int numberOfCycles = 0;
            Duration durationOn;
            Duration durationOff;
            int power = 0;
        };

        struct PresetExposure {
            QString name;
            ExposureMode mode = ExposureMode::Continuous;
            ContinuousSettings continuous;
            FlashingSettings flashing;
        };

    public:
        explicit ExposureBaseViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        virtual ~ExposureBaseViewModel() = default;

    protected:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Exposure