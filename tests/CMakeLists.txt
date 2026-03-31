include(FetchContent)

find_package(Qt6 REQUIRED COMPONENTS Test)

# Fetch Catch2 v3
fetchcontent_declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.13.0
)
fetchcontent_makeavailable(Catch2)

# Create a Test Executable for your Logic
add_executable(unit_tests
    test_main.cpp
    test_dummy.cpp
    test_machine_status_repo.cpp
    algorithms/test_trapezoidal_generator.cpp
    HAL/test_stepper_motor.cpp
    tasks/Drawers/test_mask_insertion_task.cpp
)

target_link_libraries(unit_tests
    PRIVATE
    Kub3Core
    Catch2::Catch2
    Qt6::Test
)

include(CTest)
include(Catch)
catch_discover_tests(unit_tests)
