#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <ViewModels/Exposure/ProgressExposureViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    ProgressExposureViewModel::ProgressExposureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        ExposureBaseViewModel(repo, parent)
    {
    }

    ProgressExposureViewModel::~ProgressExposureViewModel()
    {
    }

    void ProgressExposureViewModel::ps_launchExposure(const PresetExposure &preset)
    {
        m_currentPreset = preset;
    }

    PresetExposure ProgressExposureViewModel::getCurrentPreset() const
    {
        return m_currentPreset;
    }

    uint32_t ProgressExposureViewModel::getTemperature() const
    {
        return HAL::MS::readInt32(m_repo, INTERNAL_TEMPERATURE); // TODO Choose or decide what temp to show, indoor or outdoor
    }

} // namespace Kub3::UI::ViewModels::Exposure