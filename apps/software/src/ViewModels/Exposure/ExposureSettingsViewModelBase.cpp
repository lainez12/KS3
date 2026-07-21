#include <ViewModels/Exposure/ExposureSettingsViewModelBase.h>

namespace Kub3::UI::ViewModels::Exposure
{
    ExposureSettingsViewModelBase::ExposureSettingsViewModelBase(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }

} // namespace Kub3::UI::ViewModels::Exposure