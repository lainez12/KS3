#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <HAL/MachineStatus/virtual_labels.h>
#include <Services/Homing/tasks/Initialization/ForceSensorsTareTask.h>

namespace Kub3::Services
{

    ForceSensorsTareTask::ForceSensorsTareTask(Shared<HAL::MS::IMachineStatusRepo> repo) :
        m_repo(std::move(repo))
    {}

    void ForceSensorsTareTask::start(void)
    {
        m_ticksCount = 0;
    }

    // TODO: average value on N ticks ?
    bool ForceSensorsTareTask::tick(void)
    {
        if (m_ticksCount < 50)
        {
            m_ticksCount++;
            return false;
        }

        m_repo->setValueRaw(V_TARE_FORCE_LEFT_ADC, HAL::MS::readUInt16(m_repo, FORCE_LEFT_ADC));
        m_repo->setValueRaw(V_TARE_FORCE_RIGHT_ADC, HAL::MS::readUInt16(m_repo, FORCE_RIGHT_ADC));
        m_repo->setValueRaw(V_TARE_FORCE_BACK_ADC, HAL::MS::readUInt16(m_repo, FORCE_BACK_ADC));

        return true;
    }

}
