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

set(CMAKE_AUTOUIC_SEARCH_PATHS "gui;gui/Views;gui/Components;gui/Views/ViewsSettings;gui/Views/ViewsExposure;gui/Views/ViewsAlignment;tools/configurator/gui")
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Widgets SerialPort Svg)

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
    src/Services/tasks/ReachLimitTask.cpp
    src/Services/Drawers/DualConveyorDrawerService.cpp
    src/Services/Drawers/SingleConveyorDrawerService.cpp
    src/Services/Drawers/tasks/MaskEjectionTask.cpp
    src/Services/Drawers/tasks/WaferEjectionTask.cpp
    src/Services/Homing/HomingService.cpp
    src/Services/Homing/tasks/ZMotorsReachLimitTask.cpp
    src/Services/Homing/tasks/Homing/AlignmentStagesHomingTask.cpp
    src/Services/Homing/tasks/Homing/CamerasHomingTask.cpp
    src/Services/Homing/tasks/Homing/DeckHomingTask.cpp
    src/Services/Homing/tasks/Homing/MaskHomingTask.cpp
    src/Services/Homing/tasks/Homing/WaferHomingTask.cpp
    src/Services/Homing/tasks/Homing/ZMotorsHomingTask.cpp
    src/Services/Homing/tasks/Initialization/AlignmentStagesInitTask.cpp
    src/Services/Homing/tasks/Initialization/CamerasInitTask.cpp
    src/Services/Homing/tasks/Initialization/DeckInitTask.cpp
    src/Services/Homing/tasks/Initialization/MaskConveyorInitTask.cpp
    src/Services/Homing/tasks/Initialization/WaferConveyorInitTask.cpp
    src/Services/Homing/tasks/Initialization/ZMotorsInitTask.cpp
    ## HAL
    src/HAL/HardwareManager.cpp
    src/HAL/MachineStatus/MachineStatusRepo.cpp
    src/HAL/MachineStatus/utils.cpp
    src/HAL/MCUDriver.cpp
    src/HAL/Com/SerialCommunicator.cpp
    src/HAL/Com/MockCommunicator.cpp
    src/HAL/Com/LengthBasedParser.cpp
    src/HAL/Com/PacketRouter.cpp
    src/HAL/Actuators/Lights/UVExposureHead.cpp
    src/HAL/Actuators/Motors/StepperMotor.cpp
    src/HAL/Actuators/Motors/DirectCurrentMotor.cpp
    src/HAL/Actuators/Valves/SolenoidValve.cpp
    ## Algorithms
    src/Algorithms/Kinematic/utils.cpp
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
    include/Services/tasks/ReachLimitTask.h
    include/Services/Drawers/IDrawerService.h
    include/Services/Drawers/DualConveyorDrawerService.h
    include/Services/Drawers/SingleConveyorDrawerService.h
    include/Services/Drawers/tasks/MaskInsertionTask.h
    include/Services/Drawers/tasks/MaskEjectionTask.h
    include/Services/Drawers/tasks/WaferInsertionTask.h
    include/Services/Drawers/tasks/WaferEjectionTask.h
    include/Services/Homing/IHomingService.h
    include/Services/Homing/HomingService.h
    include/Services/Homing/tasks/ZMotorsReachLimitTask.h
    include/Services/Homing/tasks/Homing/AlignmentStagesHomingTask.h
    include/Services/Homing/tasks/Homing/CamerasHomingTask.h
    include/Services/Homing/tasks/Homing/DeckHomingTask.h
    include/Services/Homing/tasks/Homing/MaskHomingTask.h
    include/Services/Homing/tasks/Homing/WaferHomingTask.h
    include/Services/Homing/tasks/Homing/ZMotorsHomingTask.h
    include/Services/Homing/tasks/Initialization/AlignmentStagesInitTask.h
    include/Services/Homing/tasks/Initialization/CamerasInitTask.h
    include/Services/Homing/tasks/Initialization/DeckInitTask.h
    include/Services/Homing/tasks/Initialization/MaskConveyorInitTask.h
    include/Services/Homing/tasks/Initialization/WaferConveyorInitTask.h
    include/Services/Homing/tasks/Initialization/ZMotorsInitTask.h
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
    include/HAL/Sensors/ISensor.h
    include/HAL/Sensors/Sensor.h
    include/HAL/Actuators/ActuatorRegistry.h
    include/HAL/Actuators/IActuator.h
    include/HAL/Actuators/Lights/IExposureSystem.h
    include/HAL/Actuators/Lights/UVExposureHead.h
    include/HAL/Actuators/Motors/IMotor.h
    include/HAL/Actuators/Motors/StepperMotor.h
    include/HAL/Actuators/Motors/DirectCurrentMotor.h
    include/HAL/Actuators/Valves/IValve.h
    include/HAL/Actuators/Valves/SolenoidValve.h
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
    Qt6::Svg
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
    src/ViewModels/HomeViewModel.cpp
    src/ViewModels/DebugViewModel.cpp
    src/ViewModels/SettingsViewModel.cpp
    src/ViewModels/ViewModelsExposure/ExposureSettingsViewModel.cpp
    src/ViewModels/ViewModelsExposure/FavoriteExposureSettingsViewModel.cpp
    src/ViewModels/ViewModelsExposure/RecapExposureSettingsViewModel.cpp
    src/ViewModels/ViewModelsExposure/CompleteExposureViewModel.cpp
    src/ViewModels/ViewModelsExposure/ProgressExposureViewModel.cpp
    src/ViewModels/ViewModelsExposure/SaveExposureSettingsViewModel.cpp
    src/ViewModels/ViewModelsSettings/MachineStatusViewModel.cpp
    src/ViewModels/ViewModelsSettings/VersionViewModel.cpp
    src/ViewModels/ViewModelsSettings/TemperatureViewModel.cpp
    src/ViewModels/ViewModelsSettings/UpdateSoftwareViewModel.cpp
    src/ViewModels/ViewModelsSettings/ScreenshotExportViewModel.cpp
    src/ViewModels/ViewModelsSettings/LedTestViewModel.cpp
    src/ViewModels/ViewModelsSettings/OperatingTimesViewModel.cpp
    src/ViewModels/ViewModelsAlignment/LoadParametersViewModel.cpp
    src/ViewModels/ViewModelsAlignment/SaveParametersViewModel.cpp
    src/ViewModels/ViewModelsAlignment/VisualisationViewModel.cpp
    src/ViewModels/ViewModelsAlignment/DistanceViewModel.cpp
    src/Views/ViewsExposure/ExposureSettingsView.cpp
    src/Views/ViewsExposure/FavoriteExposureSettingsView.cpp
    src/Views/ViewsExposure/RecapExposureSettingsView.cpp
    src/Views/ViewsExposure/CompleteExposureView.cpp
    src/Views/ViewsExposure/ProgressExposureView.cpp
    src/Views/ViewsExposure/SaveExposureSettingsView.cpp
    src/Views/MainWindow.cpp
    src/Views/ViewBase.cpp
    src/Views/KeyboardConnections.cpp
    src/Views/AlignmentViewBase.cpp
    src/Views/SettingsViewBase.cpp
    src/Views/ExposureViewBase.cpp
    src/Views/DebugView.cpp
    src/Views/HomeView.cpp
    src/Views/HomeEightView.cpp
    src/Views/SettingsView.cpp
    src/Views/ViewsSettings/MachineStatusView.cpp
    src/Views/ViewsSettings/VersionView.cpp
    src/Views/ViewsSettings/TemperatureView.cpp
    src/Views/ViewsSettings/UpdateSoftwareView.cpp
    src/Views/ViewsSettings/ScreenshotExportView.cpp
    src/Views/ViewsSettings/LedTestView.cpp
    src/Views/ViewsSettings/OperatingTimesView.cpp
    src/Views/ViewsAlignment/LoadParametersView.cpp
    src/Views/ViewsAlignment/SaveParametersView.cpp
    src/Views/ViewsAlignment/VisualisationView.cpp
    src/Views/ViewsAlignment/DistanceView.cpp
    src/Views/Components/FavoriteExposureSettingButton.cpp
    src//Views/Components/MessageOverlay.cpp
    src/Views/Components/NavButtonManager.cpp
    src/Views/Components/NavButton.cpp
    src/Views/Components/UpBar.cpp
    src/Views/Components/ActionBox.cpp
    src/Views/Components/BottomCroppedCircle.cpp
    # Headers
    include/ApplicationBuilder.h
    include/ViewModels/HomeViewModel.h
    include/ViewModels/DebugViewModel.h
    include/ViewModels/SettingsViewModel.h
    include/ViewModels/ViewModelsExposure/ExposureSettingsViewModel.h
    include/ViewModels/ViewModelsExposure/FavoriteExposureSettingsViewModel.h
    include/ViewModels/ViewModelsExposure/RecapExposureSettingsViewModel.h
    include/ViewModels/ViewModelsExposure/CompleteExposureViewModel.h
    include/ViewModels/ViewModelsExposure/ProgressExposureViewModel.h
    include/ViewModels/ViewModelsExposure/SaveExposureSettingsViewModel.h
    include/ViewModels/ViewModelsSettings/MachineStatusViewModel.h
    include/ViewModels/ViewModelsSettings/VersionViewModel.h
    include/ViewModels/ViewModelsSettings/TemperatureViewModel.h
    include/ViewModels/ViewModelsSettings/UpdateSoftwareViewModel.h
    include/ViewModels/ViewModelsSettings/ScreenshotExportViewModel.h
    include/ViewModels/ViewModelsSettings/LedTestViewModel.h
    include/ViewModels/ViewModelsSettings/OperatingTimesViewModel.h
    include/ViewModels/ViewModelsAlignment/LoadParametersViewModel.h
    include/ViewModels/ViewModelsAlignment/SaveParametersViewModel.h
    include/ViewModels/ViewModelsAlignment/VisualisationViewModel.h
    include/ViewModels/ViewModelsAlignment/DistanceViewModel.h
    include/Views/ViewsExposure/ExposureSettingsView.h
    include/Views/ViewsExposure/FavoriteExposureSettingsView.h
    include/Views/ViewsExposure/RecapExposureSettingsView.h
    include/Views/ViewsExposure/CompleteExposureView.h
    include/Views/ViewsExposure/ProgressExposureView.h
    include/Views/ViewsExposure/SaveExposureSettingsView.h
    include/Views/MainWindow.h
    include/Views/ViewBase.h
    include/Views/KeyboardConnections.h
    include/Views/AlignmentViewBase.h
    include/Views/SettingsViewBase.h
    include/Views/ExposureViewBase.h
    include/Views/DebugView.h
    include/Views/HomeView.h
    include/Views/HomeEightView.h
    include/Views/SettingsView.h
    include/Views/ViewsSettings/MachineStatusView.h
    include/Views/ViewsSettings/TemperatureView.h
    include/Views/ViewsSettings/VersionView.h
    include/Views/ViewsSettings/UpdateSoftwareView.h
    include/Views/ViewsSettings/ScreenshotExportView.h
    include/Views/ViewsSettings/LedTestView.h
    include/Views/ViewsSettings/OperatingTimesView.h
    include/Views/ViewsAlignment/LoadParametersView.h
    include/Views/ViewsAlignment/SaveParametersView.h
    include/Views/ViewsAlignment/VisualisationView.h
    include/Views/ViewsAlignment/DistanceView.h
    include/Views/Components/FavoriteExposureSettingButton.h
    include/Views/Components/NavButtonManager.h
    include/Views/Components/MessageOverlay.h
    include/Views/Components/NavButton.h
    include/Views/Components/UpBar.h
    include/Views/Components/ActionBox.h
    include/Views/Components/BottomCroppedCircle.h
    # UI
    gui/resources.qrc
    gui/MainWindow.ui
    gui/Views/DebugView.ui
    gui/Views/ViewsExposure/ExposureSettingsView.ui
    gui/Views/ViewsExposure/FavoriteExposureSettingsView.ui
    gui/Views/ViewsExposure/RecapExposureSettingsView.ui
    gui/Views/ViewsExposure/CompleteExposureView.ui
    gui/Views/ViewsExposure/ProgressExposureView.ui
    gui/Views/HomeView.ui
    gui/Views/HomeEightView.ui
    gui/Views/SettingsView.ui
    gui/Views/ViewsSettings/MachineStatusView.ui
    gui/Views/ViewsSettings/VersionView.ui
    gui/Views/ViewsSettings/TemperatureView.ui
    gui/Views/ViewsSettings/UpdateSoftwareView.ui
    gui/Views/ViewsSettings/ScreenshotExportView.ui
    gui/Views/ViewsSettings/LedTestView.ui
    gui/Views/ViewsSettings/OperatingTimesView.ui
    gui/Views/ViewsAlignment/LoadParametersView.ui
    gui/Views/ViewsAlignment/SaveParametersView.ui
    gui/Views/ViewsAlignment/VisualisationView.ui
    gui/Views/ViewsAlignment/DistanceView.ui
)

qt_finalize_target(Kub3Software)

# Everything defined as PUBLIC on linked libraries is automatically inherited here
target_link_libraries(Kub3Software PRIVATE
    Kub3Core
    Qt6::Gui
    Qt6::Widgets
)

# ==============================================================================
# 6. CONFIGURATOR EXECUTABLE: Kub3Configurator
# ==============================================================================
qt_add_executable(Kub3Configurator
    MANUAL_FINALIZATION
    # Sources
    tools/configurator/src/main.cpp
    tools/configurator/src/ConfigWindow.cpp
    # Headers
    tools/configurator/include/ConfigWindow.h
    # UI
    tools/configurator/gui/ConfigWindow.ui
)

qt_finalize_target(Kub3Configurator)

target_link_libraries(Kub3Configurator PRIVATE
    Kub3Core
    Qt6::Gui
    Qt6::Widgets
)

# ==============================================================================
# 7. TESTS
# ==============================================================================
enable_testing()
add_subdirectory(tests)


# ==============================================================================
# 8. DEPLOYMENT & INSTALLATION (Portable Production Bundle)
# ==============================================================================

# ($ORIGIN is the Linux variable for "the directory the executable is in").
set_target_properties(Kub3Software PROPERTIES
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib;$ORIGIN/lib"
    BUILD_WITH_INSTALL_RPATH FALSE
)

# Install the main executable into the "bin" directory of our output folder
install(TARGETS Kub3Software    RUNTIME DESTINATION bin)
# Install the configurator executable into the "bin" directory of our output folder
install(TARGETS Kub3Configurator RUNTIME DESTINATION bin)

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
