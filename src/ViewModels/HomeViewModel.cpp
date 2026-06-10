#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <ViewModels/HomeViewModel.h>

namespace Kub3::UI::ViewModels
{

    HomeViewModel::HomeViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }

    HomeViewModel::~HomeViewModel()
    {
    }

}
