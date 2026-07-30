#pragma once

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/ITask.h>

namespace Kub3::Services
{
    class StowageWaitForMaskVacuumTask : public ITask
    {
    public:
        StowageWaitForMaskVacuumTask(Shared<HAL::MS::IMachineStatusRepo> repo) :
            m_repo(std::move(repo)) {}

        void start() override
        {
            postInfo("Waiting for vacuum to secure the mask...");
        }

        bool tick() override
        {
            // Task completes as soon as vacuum is physically confirmed
            return HAL::MS::readBool(m_repo, MASK_VACUUM_ACTIVE);
        }

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };
}
