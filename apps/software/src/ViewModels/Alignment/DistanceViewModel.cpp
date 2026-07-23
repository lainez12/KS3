#include <ViewModels/Alignment/DistanceViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{

    DistanceViewModel::DistanceViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }

    DistanceViewModel::~DistanceViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Alignment