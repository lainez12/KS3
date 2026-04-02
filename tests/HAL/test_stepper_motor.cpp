#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <memory>

#include "HAL/Actuators/Motors/StepperMotor.h"
#include "HAL/MCUDriver.h"

using namespace Kub3::HAL;
using namespace Kub3::HAL::Act;
using namespace Kub3::Config;

class MockCommunicator final : public Com::ICommunicator
{
public:
    int moveCommandsSent     = 0;
    bool stopCommandReceived = false;
    QByteArray lastPayload;
    QEventLoop *m_eventLoop = nullptr;

public:
    bool open() override
    {
        return true;
    };

    void close() override {};

    bool isOpen() const
    {
        return true;
    };

    bool send(QByteArray &&data) override
    {
        if (data.isEmpty())
            return false;
        lastPayload = data;

        std::cerr << "Sending: " << data.toHex(' ').toStdString() << std::endl;

        if (data[0] == '2') // '2' == Move Command
            moveCommandsSent++;
        else if (data[0] == '1') // '1' == Stop Command
        {
            stopCommandReceived = true;
            std::cerr << "\n>>> QUITTING" << std::endl;
            // Break the Qt Event Loop
            if (m_eventLoop)
                m_eventLoop->quit();
        }
        return true;
    };
};

class MockParser final : public Com::IProtocolParser
{
    void feedBytes(const QByteArray &rawData) override {};
    [[nodiscard]] Optional<Com::packet_t> tryExtractPacket(void) override
    {
        return std::nullopt;
    };
    void reset(void) override {};
};

class MockMCUDriver : public MCUDriver
{
public:
    MockMCUDriver(Unique<Com::ICommunicator> comm, Unique<Com::IProtocolParser> parser) :
        MCUDriver(std::move(comm), std::move(parser))
    {
    }
};

TEST_CASE("StepperMotor Thread-Safe Dispatch & Queued Connections", "[actuators][qt]")
{
    // 1. Initialize QCoreApplication (Required once per process for Qt QueuedConnections)
    if (!QCoreApplication::instance())
    {
        static int argc     = 1;
        static char name[]  = "test_stepper";
        static char *argv[] = {name, nullptr};
        new QCoreApplication(argc, argv); // Intentionally leaked for the test suite lifetime
    }

    // 2. Setup a local Qt Event Loop
    QEventLoop loop;

    Unique<MockCommunicator> mockCommunicator = std::make_unique<MockCommunicator>();
    MockCommunicator *commObserver            = mockCommunicator.get();
    Unique<MockParser> mockParser             = std::make_unique<MockParser>();
    Shared<MockMCUDriver> mcuDriver           = std::make_shared<MockMCUDriver>(std::move(mockCommunicator), std::move(mockParser));

    commObserver->m_eventLoop = &loop;

    // Setup physical constraints
    stepper_hw_properties_t hwConfig = {
        .stepsPerRev         = 200,
        .screwPitchMm        = 5.0,
        .maxVelocityMmS      = 100.0,
        .maxAccelerationMmS2 = 500.0,
    };

    auto dummyPosGetter = []()
    { return 0.0; };

    // Instantiate the motor
    StepperMotor motor("TEST_M1", 0x01, mcuDriver, hwConfig, dummyPosGetter);

    SECTION("Generates Move Commands and resolves with a Stop Command over the Event Loop")
    {
        kinematic_profile_t kinProfile = {
            .id                 = "dummyProfile",
            .initialVelocityMmS = 0.0,
            .targetVelocityMmS  = 10.0,
            .accelerationMmS2   = 200.0,
            .params             = stepper_kinematics_params_t{16},
        };

        // Initiate a tiny movement
        motor.moveRelative(1.0, kinProfile);

        // 3. Setup Timeout Safety Net
        QTimer timeoutTimer;

        timeoutTimer.setSingleShot(true);
        QObject::connect(
            &timeoutTimer,
            &QTimer::timeout,
            &loop,
            [&]()
            {
                FAIL("Qt Event Loop timed out. The Motor never finished the move.");
                loop.quit();
            });
        timeoutTimer.start(2000); // 2 seconds timeout

        // Blocks execution here, allowing the StepperMotor's internal 50Hz QTimer to fire and invokeMethod() to push bytes to our MockMCUDriver.
        loop.exec();
        timeoutTimer.stop();

        // 4. Architectural Verification
        REQUIRE(commObserver->moveCommandsSent > 0);
        REQUIRE(commObserver->stopCommandReceived == true);

        // The last payload pushed to the bus MUST be the stop sequence
        REQUIRE(commObserver->lastPayload.size() >= 2);
        REQUIRE(commObserver->lastPayload[0] == '1');
        REQUIRE(commObserver->lastPayload[1] == 0x01); // Asserts correct Motor Byte ID
    }

    // TODO: fix flakiness
    SECTION("On-the-fly profile updates do not interrupt motion")
    {
        kinematic_profile_t fastProfile = {
            .targetVelocityMmS = 50.0,
            .accelerationMmS2  = 200.0,
            .params            = stepper_kinematics_params_t{16},
        };

        kinematic_profile_t slowProfile = {
            .targetVelocityMmS = 5.0,
            .accelerationMmS2  = 50.0,
            .params            = stepper_kinematics_params_t{16},
        };

        // 1. Start a long move
        motor.moveAbsolute(200.0, fastProfile);

        // Let it run for just 200ms (It won't reach 200mm in 200ms)
        QTimer interruptTimer;
        interruptTimer.setSingleShot(true);
        QObject::connect(&interruptTimer, &QTimer::timeout, &loop, [&]()
                         { loop.quit(); });
        interruptTimer.start(200);
        loop.exec(); // Blocks until interruptTimer fires

        // 2. Verify we are moving and have NOT received a stop command
        REQUIRE(commObserver->moveCommandsSent > 0);
        REQUIRE(commObserver->stopCommandReceived == false);

        const int moveCommandsBeforeUpdate = commObserver->moveCommandsSent;

        // 3. ON-THE-FLY INJECTION: Slow down, but keep going to 200mm
        std::cerr << "[UPDATING KINEMATICS PROFILE] fast -> slow" << std::endl;
        motor.moveAbsolute(12.0, slowProfile);

        // 4. Setup finish trap
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);

        QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, [&]()
                         {
            FAIL("Motor failed to finish the blended move.");
            loop.quit(); });

        // The mock exits the loop automatically when it receives the '1' Stop Command
        timeoutTimer.start(3000);
        loop.exec();

        // 5. Verification
        REQUIRE(commObserver->stopCommandReceived == true);
        // Ensure more move ('2') commands were generated AFTER the update
        REQUIRE(commObserver->moveCommandsSent > moveCommandsBeforeUpdate);
    }
}
