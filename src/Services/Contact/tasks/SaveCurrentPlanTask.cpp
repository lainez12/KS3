#include <QDebug>
#include <QString>

#include <Config/helper.h>
#include <Services/Contact/tasks/SaveCurrentPlanTask.h>

namespace Kub3::Services
{

    SaveCurrentPlanTask::SaveCurrentPlanTask(std::array<Shared<HAL::Act::IMotor>, 3> motors,
                                             Optional<plan_deltas_t> &outPlanDeltas) :
        m_motors(std::move(motors)),
        m_outPlanDeltas(outPlanDeltas)
    {
    }

    void SaveCurrentPlanTask::start(void)
    {
        const double leftPos    = m_motors[0]->getEncoderPositionMm();
        const double rightPos   = m_motors[1]->getEncoderPositionMm();
        const double backPos    = m_motors[2]->getEncoderPositionMm();
        const double highestPos = std::max({leftPos, rightPos, backPos});

        m_outPlanDeltas = plan_deltas_t{
            .left  = highestPos - leftPos,
            .right = highestPos - rightPos,
            .back  = highestPos - backPos,
        };
    }

    bool SaveCurrentPlanTask::tick(void)
    {
        return true; /* NO-OP */
    }

} // namespace Kub3::Services
