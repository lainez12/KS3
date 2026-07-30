#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <HAL/MachineStatus/virtual_labels.h>
#include <Services/Homing/tasks/Initialization/ForceSensorsTareTask.h>

#define STABILIZING_TICKS_COUNT      40
#define MEAN_CALCULATION_TICKS_COUNT 10

namespace Kub3::Services
{

    ForceSensorsTareTask::ForceSensorsTareTask(Shared<HAL::MS::IMachineStatusRepo> repo) :
        m_repo(std::move(repo))
    {}

    void ForceSensorsTareTask::start(void)
    {
        m_ticksCount    = 0;
        m_meanValues[0] = 0;
        m_meanValues[1] = 0;
        m_meanValues[2] = 0;
    }

    bool ForceSensorsTareTask::tick(void)
    {
        if (m_ticksCount < STABILIZING_TICKS_COUNT + MEAN_CALCULATION_TICKS_COUNT)
        {
            m_ticksCount++;
            if (m_ticksCount > STABILIZING_TICKS_COUNT)
            {
                m_meanValues[0] += HAL::MS::readUInt16(m_repo, FORCE_LEFT_ADC);
                m_meanValues[1] += HAL::MS::readUInt16(m_repo, FORCE_RIGHT_ADC);
                m_meanValues[2] += HAL::MS::readUInt16(m_repo, FORCE_BACK_ADC);
            }
            return false;
        }

        m_repo->setValueRaw(V_TARE_FORCE_LEFT_ADC, static_cast<uint16_t>(m_meanValues[0] / MEAN_CALCULATION_TICKS_COUNT));
        m_repo->setValueRaw(V_TARE_FORCE_RIGHT_ADC, static_cast<uint16_t>(m_meanValues[1] / MEAN_CALCULATION_TICKS_COUNT));
        m_repo->setValueRaw(V_TARE_FORCE_BACK_ADC, static_cast<uint16_t>(m_meanValues[2] / MEAN_CALCULATION_TICKS_COUNT));

        return true;
    }

}
