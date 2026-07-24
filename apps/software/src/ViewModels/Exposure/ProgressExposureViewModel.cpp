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

} // namespace Kub3::UI::ViewModels::Exposure