cmake_minimum_required(VERSION 3.20)

# ==============================================================================
# 1. PROJECT SETUP & VERSIONING
# ==============================================================================
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.version" RAW_VERSION)
string(STRIP "${RAW_VERSION}" CLEAN_VERSION)

project(Kub3Software VERSION ${CLEAN_VERSION} LANGUAGES CXX)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/version.h"
)

# ==============================================================================
# 2. COMPILER & QT SETUP
# ==============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOUIC_SEARCH_PATHS "gui;gui/Views;gui/Components")
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Widgets SerialPort)

# ==============================================================================
# 3. EXTERNAL DEPENDENCIES (MVS Library)
# ==============================================================================
find_path(MVS_INCLUDE_DIR NAMES MvCameraControl.h PATHS /opt/MVS/include $ENV{MVS_ROOT}/include)
find_library(MVS_LIBRARY NAMES MvCameraControl PATHS /opt/MVS/lib/64 /opt/MVS/lib/32 $ENV{MVS_ROOT}/lib/64)

if(NOT MVS_INCLUDE_DIR OR NOT MVS_LIBRARY)
    message(WARNING "MVS Hikrobot Library NOT FOUND. Vision features disabled.")
endif()

# ==============================================================================
# 4. PROJECT'S CORE LIBRARY
# ==============================================================================
qt_add_library(Kub3Core STATIC
    # Sources
    ## Utils & miscellaneous
    src/utils.cpp
    src/Config/ConfigLoader.cpp
    ## Master FSM
    src/MFSM/MasterFSM.cpp
    ## Services
    src/Services/Drawers/DualConveyorDrawerService.cpp
    src/Services/Drawers/SingleConveyorDrawerService.cpp
    src/Services/Drawers/tasks/MaskInsertionTask.cpp
    src/Services/Drawers/tasks/MaskEjectionTask.cpp
    src/Services/Drawers/tasks/WaferInsertionTask.cpp
    src/Services/Drawers/tasks/WaferEjectionTask.cpp
    ## HAL
    src/HAL/HardwareManager.cpp
    src/HAL/MachineStatus/MachineStatusRepo.cpp
    src/HAL/MachineStatus/utils.cpp
    src/HAL/MCUDriver.cpp
    src/HAL/Com/SerialCommunicator.cpp
    src/HAL/Com/MockCommunicator.cpp
    src/HAL/Com/LengthBasedParser.cpp
    src/HAL/Com/PacketRouter.cpp
    src/HAL/Actuators/Motors/StepperMotor.cpp
    ## Algorithms
    src/Algorithms/Kinematic/TrapezoidalGenerator.cpp

    # Headers
    ## Utils & miscellaneous
    include/utils.h
    include/Config/ConfigLoader.h
    include/Config/kinematics.h
    include/Config/machine_config.h
    ## Master FSM
    include/MFSM/MasterFSM.h
    include/MFSM/events.h
    include/MFSM/states.h
    ## Services
    include/Services/ITask.h
    include/Services/IService.h
    include/Services/BaseTaskService.h
    include/Services/Drawers/IDrawerService.h
    include/Services/Drawers/DualConveyorDrawerService.h
    include/Services/Drawers/SingleConveyorDrawerService.h
    include/Services/Drawers/tasks/MaskInsertionTask.h
    include/Services/Drawers/tasks/MaskEjectionTask.h
    include/Services/Drawers/tasks/WaferInsertionTask.h
    include/Services/Drawers/tasks/WaferEjectionTask.h
    ## HAL
    include/HAL/HardwareManager.h
    include/HAL/MachineStatus/IMachineStatusRepo.h
    include/HAL/MachineStatus/MachineStatusRepo.h
    include/HAL/MachineStatus/utils.h
    include/HAL/MachineStatus/sensors_labels.h
    include/HAL/MCUDriver.h
    include/HAL/Com/ICommunicator.h
    include/HAL/Com/SerialCommunicator.h
    include/HAL/Com/MockCommunicator.h
    include/HAL/Com/IProtocolParser.h
    include/HAL/Com/LengthBasedParser.h
    include/HAL/Com/PacketRouter.h
    include/HAL/Actuators/ActuatorRegistry.h
    include/HAL/Actuators/IActuator.h
    include/HAL/Actuators/Motors/IMotor.h
    include/HAL/Actuators/Motors/StepperMotor.h
    include/HAL/Sensors/ISensor.h
    include/HAL/Sensors/Sensor.h
    ## Algorithms
    include/Algorithms/Kinematic/utils.h
    include/Algorithms/Kinematic/IKinematicGenerator.h
    include/Algorithms/Kinematic/TrapezoidalGenerator.h
)

# PUBLIC: Everything that links to Kub3Core automatically gets these headers and macros
target_include_directories(Kub3Core PUBLIC
    include
    include/Views/Components/
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

target_link_libraries(Kub3Core PUBLIC
    Qt6::Core
    Qt6::SerialPort
)

if(MVS_LIBRARY)
    target_link_libraries(Kub3Core PUBLIC ${MVS_LIBRARY})
    target_include_directories(Kub3Core PUBLIC ${MVS_INCLUDE_DIR})
endif()

# Define Machine Model and Build Type globally
set(KUB_MODEL "8" CACHE STRING "KUB3 Machine Model Target (4, 6, or 8)")
target_compile_definitions(Kub3Core PUBLIC KUB_MODEL_${KUB_MODEL} KUB_MODEL_STR="${KUB_MODEL}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Kub3Core PUBLIC BUILD_DEBUG)
else()
    target_compile_definitions(Kub3Core PUBLIC BUILD_RELEASE)
endif()

# ==============================================================================
# 5. MAIN EXECUTABLE: Kub3Software
# ==============================================================================
qt_add_executable(Kub3Software
    MANUAL_FINALIZATION
    # Sources
    src/main.cpp
    src/Logger.h
    src/ApplicationBuilder.cpp
    src/Views/MainWindow.cpp
    src/Views/DebugView.cpp
    src/Views/HomeView.cpp
    src/Views/MachineStatusView.cpp
    src/Views/Components/NavButton.cpp
    src/Views/Components/ActionBox.cpp
    src/Views/Components/BottomCroppedCircle.cpp
    # Headers
    include/ApplicationBuilder.h
    include/Views/MainWindow.h
    include/Views/DebugView.h
    include/Views/HomeView.h
    include/Views/MachineStatusView.h
    include/Views/Components/NavButton.h
    include/Views/Components/ActionBox.h
    include/Views/Components/BottomCroppedCircle.h
    # UI
    gui/resources.qrc
    gui/MainWindow.ui
    gui/Views/DebugView.ui
    gui/Views/HomeView.ui
    gui/Views/MachineStatusView.ui
)

qt_finalize_target(Kub3Software)

# Everything defined as PUBLIC on linked libraries is automatically inherited here
target_link_libraries(Kub3Software PRIVATE
    Kub3Core
    Qt6::Gui
    Qt6::Widgets
)

# ==============================================================================
# 6. TESTS
# ==============================================================================
enable_testing()
add_subdirectory(tests)


# ==============================================================================
# 7. DEPLOYMENT & INSTALLATION (Portable Production Bundle)
# ==============================================================================

# ($ORIGIN is the Linux variable for "the directory the executable is in").
set_target_properties(Kub3Software PROPERTIES
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib;$ORIGIN/lib"
    BUILD_WITH_INSTALL_RPATH FALSE
)

# Install the main executable into the "bin" directory of our output folder
install(TARGETS Kub3Software
    RUNTIME DESTINATION bin
)

# Explicitly deploy the MVS Camera libraries
if(MVS_LIBRARY)
    get_filename_component(MVS_LIB_DIR "${MVS_LIBRARY}" DIRECTORY)
    # Copies all Hikrobot .so files into the "lib" folder of our distribution
    install(DIRECTORY "${MVS_LIB_DIR}/"
        DESTINATION lib
        FILES_MATCHING PATTERN "*.so*"
    )
endif()

# Automatically deploy all necessary Qt libraries, plugins, and system dependencies
if(TARGET Qt6::Core)
    qt_generate_deploy_app_script(
        TARGET Kub3Software
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()
