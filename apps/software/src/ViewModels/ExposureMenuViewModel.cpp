#include <ViewModels/ExposureMenuViewModel.h>

namespace Kub3::UI::ViewModels
{

    ExposureMenuViewModel::ExposureMenuViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }

    ExposureMenuViewModel::~ExposureMenuViewModel()
    {
    }

}