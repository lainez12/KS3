#include "HAL/Actuators/Motors/IMotor.h"
#include "HAL/MachineStatus/MachineStatusRepo.h"
#include "Services/Drawers/tasks/MaskInsertionTask.h"
#include <catch2/catch_test_macros.hpp>

using namespace Kub3::Services;
using namespace Kub3::HAL;
using namespace Kub3::Config;

class DummyMotor : public Act::IMotor
{
public:
    int moveDirectionCalls = 0;
    int emergencyStopCalls = 0;
    Act::MotorDirection lastDir;

    void moveAbsolute(double, kinematic_profile_t) override
    {
    }

    void moveRelative(double, kinematic_profile_t) override
    {
    }

    void moveDirection(Act::MotorDirection dir, kinematic_profile_t) override
    {
        moveDirectionCalls++;
        lastDir = dir;
    }

    void emergencyStop() override
    {
        emergencyStopCalls++;
    }

    void enable(bool) override
    {
    }

    void home() override
    {
    }

    bool isMoving() const override
    {
        return false;
    }

    double getEncoderPositionMm() const override
    {
        return 0.0;
    }
};

TEST_CASE("MaskInsertionTask Logic FSM", "[fsm][tasks]")
{
    auto mockMotor = std::make_shared<DummyMotor>();
    auto repo      = std::make_shared<MS::MachineStatusRepo>();

    kinematic_profile_t fastProf, fineProf, contactProf;
    MaskInsertionTask task(mockMotor, repo, fastProf, fineProf, contactProf);

    SECTION("Start enters FastApproach when sensors are clear")
    {
        repo->write(MS::CM0, false);
        repo->write(MS::CM1, false);
        repo->write(MS::CM2, false);
        repo->write(MS::CM3, false);

        task.start();

        REQUIRE(mockMotor->moveDirectionCalls == 1);
        REQUIRE(mockMotor->lastDir == Act::MotorDirection::Negative);
        REQUIRE(task.tick() == false); // Not finished
    }

    SECTION("Emergency stops immediately on CM3")
    {
        repo->write(MS::CM3, false);
        task.start(); // Initiates movement

        REQUIRE(mockMotor->moveDirectionCalls == 1);

        // Simulate operator interference
        repo->write(MS::CM3, true);
        bool isFinished = task.tick();

        REQUIRE(mockMotor->emergencyStopCalls == 1);
        REQUIRE(isFinished == true);
    }

    SECTION("Transitions from Fast to Slow approach on CM1")
    {
        repo->write(MS::CM1, false);
        task.start(); // FastApproach

        // Trip the slow-down sensor
        repo->write(MS::CM1, true);
        task.tick();

        REQUIRE(mockMotor->moveDirectionCalls == 2); // Transition triggered a new command
    }
}
