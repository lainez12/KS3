#include "HAL/Actuators/Motors/IPositionMotor.h"
#include "HAL/MachineStatus/MachineStatusRepo.h"
#include "Services/Drawers/tasks/MaskInsertionTask.h"
#include <catch2/catch_test_macros.hpp>

using namespace Kub3::Services;
using namespace Kub3::HAL;
using namespace Kub3::Config;

class DummyMotor : public Act::IPositionMotor
{
public:
    const std::string m_id;
    int moveDirectionCalls = 0;
    int emergencyStopCalls = 0;
    Act::MotorDirection lastDir;

    DummyMotor(const std::string id) : m_id(id) {}

    std::string_view getId(void) const noexcept override
    {
        return m_id;
    }

    void moveAbsolute(double, kinematic_profile_t) override {}

    void moveRelative(double, kinematic_profile_t) override {}

    void moveDirection(Act::MotorDirection dir, kinematic_profile_t) override
    {
        moveDirectionCalls++;
        lastDir = dir;
    }

    void emergencyStop() override
    {
        emergencyStopCalls++;
    }

    bool isMoving() const override
    {
        return false;
    }

    std::string_view getEncoderId(void) const override
    {
        return "";
    }

    double getEncoderPositionMm() const override
    {
        return 0.0;
    }

    void resetEncoder(const double offsetMm = 0.0) override {}

private:
    double getPrecisionMm(const Kub3::Config::kinematic_profile_t &profile) override
    {
        return 0.01;
    };
};

class MockMachineStatusRepo final : public MS::IMachineStatusRepo
{
public:
    explicit MockMachineStatusRepo(QObject *parent = nullptr) : MS::IMachineStatusRepo(parent) {}
    ~MockMachineStatusRepo() override = default;

    void setValueRaw(const std::string &key, const MS::MachineValue &value) override
    {
        m_sensors[key] = value;
    }

    [[nodiscard]] Optional<MS::MachineValue> getValueRaw(const std::string &key) const override
    {
        if (auto it = m_sensors.find(key); it != m_sensors.end())
            return it->second;
        return std::nullopt;
    }

    [[nodiscard]] std::vector<std::string> getRegisteredKeys() const override
    {
        return {};
    }

private:
    std::unordered_map<std::string, MS::MachineValue> m_sensors;
};

TEST_CASE("MaskInsertionTask Logic FSM", "[fsm][tasks]")
{
    auto mockMotor = std::make_shared<DummyMotor>("DUMMY_MOTOR");
    auto repo      = std::make_shared<MockMachineStatusRepo>();

    kinematic_profile_t fastProf, fineProf, contactProf;
    MaskInsertionTask task(repo, mockMotor, fastProf, fineProf, contactProf);

    repo->setValueRaw(CM0, false);
    repo->setValueRaw(CM1, false);
    repo->setValueRaw(CM2, false);
    repo->setValueRaw(CM3, false);

    SECTION("Start enters FastApproach when sensors are clear")
    {

        task.start();

        REQUIRE(mockMotor->moveDirectionCalls == 1);
        REQUIRE(mockMotor->lastDir == Act::MotorDirection::Negative);
        REQUIRE(task.tick() == false); // Not finished
    }

    SECTION("Emergency stops immediately on CM3")
    {
        repo->setValueRaw(CM3, false);
        task.start(); // Initiates movement

        REQUIRE(mockMotor->moveDirectionCalls == 1);
        REQUIRE(mockMotor->emergencyStopCalls == 0);

        // Simulate operator interference
        repo->setValueRaw(CM3, true);
        bool isFinished = task.tick();

        REQUIRE(mockMotor->emergencyStopCalls == 1);
        REQUIRE(isFinished == true);
    }

    SECTION("Transitions from Fast to Slow approach on CM1")
    {
        repo->setValueRaw(CM1, false);
        task.start(); // FastApproach

        // Trip the slow-down sensor
        repo->setValueRaw(CM1, true);
        task.tick();

        REQUIRE(mockMotor->moveDirectionCalls == 2); // Transition triggered a new command
    }
}
