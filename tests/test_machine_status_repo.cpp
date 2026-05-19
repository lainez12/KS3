#include <QSignalSpy>
#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <thread>
#include <vector>

#include "HAL/MachineStatus/MachineStatusRepo.h"

using namespace Kub3::HAL::MS;

TEST_CASE("MachineStatusRepo: Basic Type-Safe CRUD", "[HAL][MachineStatus]")
{
    MachineStatusRepo repo;

    SECTION("Can set and get valid types correctly")
    {
        repo.setSensor<bool>("valve", true);
        repo.setSensor<int32_t>("motorPosition", -155);
        repo.setSensor<uint32_t>("forceSensor", 990);

        auto valve = repo.getSensor<bool>("valve");
        REQUIRE(valve.has_value());
        REQUIRE(valve.value() == true);

        auto motorPos = repo.getSensor<int32_t>("motorPosition");
        REQUIRE(motorPos.has_value());
        REQUIRE(motorPos.value() == -155);

        auto force = repo.getSensor<uint32_t>("forceSensor");
        REQUIRE(force.has_value());
        REQUIRE(force.value() == 990);
    }

    SECTION("Returns nullopt for unregistered keys")
    {
        auto missing = repo.getSensor<int>("invalidKey");
        REQUIRE_FALSE(missing.has_value());
    }

    SECTION("Returns nullopt when requesting the wrong type (Type Safety)")
    {
        repo.setSensor<int32_t>("position", 200);

        // We stored a int32_t. Asking for an bool should safely fail, not crash.
        auto wrongType = repo.getSensor<bool>("position");
        REQUIRE_FALSE(wrongType.has_value());
    }

    SECTION("Can retrieve the list of registered keys")
    {
        repo.setSensor<int32_t>("Sens1", 1);
        repo.setSensor<int32_t>("Sens2", 2);

        auto keys = repo.getRegisteredKeys();

        REQUIRE(keys.size() == 2);
        // Ensure both keys exist in the returned vector
        bool hasSens1 = std::find(keys.begin(), keys.end(), "Sens1") != keys.end();
        bool hasSens2 = std::find(keys.begin(), keys.end(), "Sens2") != keys.end();
        REQUIRE(hasSens1);
        REQUIRE(hasSens2);
    }
}

TEST_CASE("MachineStatusRepo: Signal Anti-Spam Logic", "[HAL][MachineStatus]")
{
    MachineStatusRepo repo;

    // QSignalSpy listens to the signals emitted by our repo
    QSignalSpy spy(&repo, &IMachineStatusRepo::s_sensorValueChanged);
    REQUIRE(spy.isValid());

    SECTION("Emits exactly once on new insertion")
    {
        repo.setSensor<bool>("InterlockSafe", true);
        REQUIRE(spy.count() == 1);

        // Verify the signal argument was "InterlockSafe"
        QList<QVariant> arguments = spy.takeFirst();
        REQUIRE(arguments.at(0).value<std::string>() == "InterlockSafe");
    }

    SECTION("Ignores duplicate updates (Anti-Spam)")
    {
        repo.setSensor<int32_t>("temperature", 25); // Emits (count = 1)
        REQUIRE(spy.count() == 1);

        // Simulating MCU sending the same value rapidly
        repo.setSensor<int32_t>("temperature", 25);
        repo.setSensor<int32_t>("temperature", 25);

        // Count must remain 1
        REQUIRE(spy.count() == 1);

        // Change the value slightly
        repo.setSensor<int32_t>("temperature", 26);
        REQUIRE(spy.count() == 2); // Now it should emit
    }
}

TEST_CASE("MachineStatusRepo: High-Concurrency Thread Safety", "[HAL][MachineStatus]")
{
    MachineStatusRepo repo;
    repo.setSensor<int>("stressCounter", 0);

    std::atomic<bool> startFlag{false};

    // A writer lambda that updates the counter 10,000 times
    auto writerTask = [&]() {
        while (!startFlag)
        {
            std::this_thread::yield();
        } // Wait for all threads to be ready
        for (int i = 1; i <= 10000; ++i)
        {
            repo.setSensor<int>("stressCounter", i);
        }
    };

    // A reader lambda that hammers the getter 10,000 times
    auto readerTask = [&]() {
        while (!startFlag)
        {
            std::this_thread::yield();
        } // Wait for all threads to be ready
        for (int i = 0; i < 10000; ++i)
        {
            auto val = repo.getSensor<int>("stressCounter");
            REQUIRE(val.has_value()); // Must never be nullopt during execution
        }
    };

    // Spin up 4 Writers and 10 Readers simultaneously
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i)
        threads.emplace_back(writerTask);
    for (int i = 0; i < 10; ++i)
        threads.emplace_back(readerTask);

    // Unleash the threads at the exact same moment
    startFlag = true;

    // Wait for all threads to finish
    for (auto &t : threads)
    {
        if (t.joinable())
            t.join();
    }

    // If we reach here without a segfault, deadlock, or assertion failure,
    // the std::shared_mutex is working perfectly.
    SUCCEED("Completed heavy concurrent read/write operations successfully.");
}
