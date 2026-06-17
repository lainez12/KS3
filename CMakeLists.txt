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
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS
    # Main app
    "gui;gui/Components"
    # --- Views
    "gui/Views;gui/Views/Settings;gui/Views/Exposure;gui/Views/Alignment"
    # Configurator tool
    "tools/configurator/gui"
    # Motor tester tool
    "tools/tester/gui"
)
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
    # ===============
    # Sources
    # ===============

    ## Utils & miscellaneous
    src/utils.cpp
    src/Config/ConfigLoader.cpp
    src/Config/ConfigSaver.cpp

    ## Views / View Models Boilerplate
    src/ViewModels/BaseViewModel.cpp

    ## Master FSM
    src/MFSM/MasterFSM.actions.cpp
    src/MFSM/MasterFSM.core.cpp
    src/MFSM/MasterFSM.heartbeat.cpp
    src/MFSM/MasterFSM.transitions.macro.cpp
    src/MFSM/MasterFSM.transitions.operational.cpp
    src/MFSM/interlocks.cpp

    ## Services
    # --- Generic tasks
    src/Services/tasks/ReachLimitTask.cpp
    src/Services/tasks/ToggleValveTask.cpp
    # --- Homing service and tasks
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
    src/Services/Homing/tasks/Initialization/MaskConveyorInitTask.cpp
    src/Services/Homing/tasks/Initialization/WaferConveyorInitTask.cpp
    src/Services/Homing/tasks/Initialization/ZMotorsInitTask.cpp
    # --- Drawers service and tasks
    src/Services/Drawers/DualConveyorDrawerService.cpp
    src/Services/Drawers/SingleConveyorDrawerService.cpp
    src/Services/Drawers/tasks/MaskEjectionTask.cpp
    src/Services/Drawers/tasks/WaferEjectionTask.cpp
    # --- Stowage service & tasks
    src/Services/Stowage/StowageService.cpp
    src/Services/Stowage/tasks/StowageMoveZToLimitTask.cpp
    # --- Alignment service & tasks
    src/Services/Alignment/AlignmentService.cpp
    # --- Vision service & tasks
    src/Services/Vision/VisionService.cpp
    # --- Contact service & tasks
    src/Services/Contact/ContactService.cpp
    src/Services/Contact/tasks/AdmittanceControlTask.cpp
    src/Services/Contact/tasks/FastApproachTask.cpp
    src/Services/Contact/tasks/SaveCurrentPlanTask.cpp
    # --- Exposure service & tasks
    src/Services/Exposure/ExposureService.cpp

    ## HAL
    src/HAL/HardwareManager.cpp
    src/HAL/MachineStatus/MachineStatusRepo.cpp
    src/HAL/MachineStatus/utils.cpp
    src/HAL/MCUDriver.cpp
    src/HAL/Com/SerialCommunicator.cpp
    src/HAL/Com/MockCommunicator.cpp
    src/HAL/Com/LengthBasedParser.cpp
    src/HAL/Com/PacketRouter.cpp
    src/HAL/Vision/HikrobotCamera/HikrobotCamera.cpp
    src/HAL/Actuators/Lights/CameraLightingLed.cpp
    src/HAL/Actuators/Lights/UVExposureHead.cpp
    src/HAL/Actuators/Motors/StepperMotor.cpp
    src/HAL/Actuators/Motors/DirectCurrentMotor.cpp
    src/HAL/Actuators/Valves/SolenoidValve.cpp
    src/HAL/Actuators/Focal/Focal.cpp
    ## Algorithms
    src/Algorithms/Kinematic/utils.cpp
    src/Algorithms/Kinematic/TrapezoidalGenerator.cpp
    src/Algorithms/Admittance/AdmittanceController.cpp
    src/Algorithms/Filters/TimeAwareEMAFilter.cpp


    # ===============
    # Headers
    # ===============

    ## Utils, Config & miscellaneous
    include/utils.h
    include/Common/Enums.h
    include/Config/ConfigLoader.h
    include/Config/ConfigSaver.h
    include/Config/kinematics.h
    include/Config/conf.h
    include/Config/hardware.h
    include/Config/process.h
    include/Config/admin.h
    include/Config/helper.h
    include/Config/keys/hardware.h
    include/Config/keys/process.h

    ## Views / View Models Boilerplate
    include/ViewModels/BaseViewModel.h

    ## Master FSM
    include/MFSM/events.h
    include/MFSM/interlocks.h
    include/MFSM/MasterFSM.h
    include/MFSM/posture.h
    include/MFSM/states.macro.h
    include/MFSM/states.operational.h

    ## Services
    include/Services/ITask.h
    include/Services/IService.h
    include/Services/BaseTaskService.h
    # --- Generic tasks
    include/Services/tasks/ReachLimitTask.h
    include/Services/tasks/ToggleValveTask.h
    # --- Homing service and tasks
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
    # --- Drawers service and tasks
    include/Services/Drawers/IDrawerService.h
    include/Services/Drawers/DualConveyorDrawerService.h
    include/Services/Drawers/SingleConveyorDrawerService.h
    include/Services/Drawers/tasks/MaskInsertionTask.h
    include/Services/Drawers/tasks/MaskEjectionTask.h
    include/Services/Drawers/tasks/WaferInsertionTask.h
    include/Services/Drawers/tasks/WaferEjectionTask.h
    # --- Stowage service & tasks
    include/Services/Stowage/IStowageService.h
    include/Services/Stowage/StowageService.h
    include/Services/Stowage/tasks/StowageCenterStagesTask.h
    include/Services/Stowage/tasks/StowageMoveZToLimitTask.h
    # --- Alignment service & tasks
    include/Services/Alignment/IAlignmentService.h
    include/Services/Alignment/AlignmentService.h
    # --- Vision service & tasks
    include/Services/Vision/IVisionService.h
    include/Services/Vision/VisionService.h
    # --- Contact service & tasks
    include/Services/Contact/IContactService.h
    include/Services/Contact/ContactService.h
    include/Services/Contact/tasks/AdmittanceControlTask.h
    include/Services/Contact/tasks/FastApproachTask.h
    include/Services/Contact/tasks/SaveCurrentPlanTask.h
    # --- Exposure service & tasks
    include/Services/Exposure/ExposureService.h
    include/Services/Exposure/IExposureService.h

    ## HAL
    include/HAL/HardwareManager.h
    include/HAL/MachineStatus/IMachineStatusRepo.h
    include/HAL/MachineStatus/MachineStatusRepo.h
    include/HAL/MachineStatus/utils.h
    include/HAL/MachineStatus/actuators_labels.h
    include/HAL/MachineStatus/sensors_labels.h
    include/HAL/MachineStatus/virtual_labels.h
    include/HAL/MCUDriver.h
    include/HAL/Com/ICommunicator.h
    include/HAL/Com/SerialCommunicator.h
    include/HAL/Com/MockCommunicator.h
    include/HAL/Com/IProtocolParser.h
    include/HAL/Com/LengthBasedParser.h
    include/HAL/Com/PacketRouter.h
    include/HAL/Sensors/ISensor.h
    include/HAL/Sensors/Sensor.h
    include/HAL/Vision/ICamera.h
    include/HAL/Vision/identifiers.h
    include/HAL/Vision/Hikrobot/HikrobotCamera.h
    include/HAL/Actuators/ActuatorRegistry.h
    include/HAL/Actuators/IActuator.h
    include/HAL/Actuators/Lights/ILightingSystem.h
    include/HAL/Actuators/Lights/CameraLightingLed.h
    include/HAL/Actuators/Lights/IExposureSystem.h
    include/HAL/Actuators/Lights/UVExposureHead.h
    include/HAL/Actuators/Motors/IMotor.h
    include/HAL/Actuators/Motors/StepperMotor.h
    include/HAL/Actuators/Motors/DirectCurrentMotor.h
    include/HAL/Actuators/Valves/IValve.h
    include/HAL/Actuators/Valves/SolenoidValve.h
    include/HAL/Actuators/Focal/IFocal.h
    include/HAL/Actuators/Focal/Focal.h

    ## Algorithms
    include/Algorithms/Kinematic/utils.h
    include/Algorithms/Kinematic/IKinematicGenerator.h
    include/Algorithms/Kinematic/TrapezoidalGenerator.h
    include/Algorithms/Admittance/AdmittanceController.h
    include/Algorithms/Filters/TimeAwareEMAFilter.h
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
    Qt6::Gui
    Qt6::Widgets
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
    target_compile_definitions(Kub3Core PUBLIC
        KUB3_HARDWARE_CONFIG_PATH="${CMAKE_SOURCE_DIR}/tests/hardware.ini"
        KUB3_PROCESS_CONFIG_PATH="${CMAKE_SOURCE_DIR}/tests/process.ini"
        KUB3_ADMIN_CONFIG_PATH="${CMAKE_SOURCE_DIR}/tests/admin.ini"
        KUB3_ADMIN_VAULT_PATH="${CMAKE_SOURCE_DIR}/tests/admin_vault.bin"
    )
else()
    target_compile_definitions(Kub3Core PUBLIC BUILD_RELEASE)
    target_compile_definitions(Kub3Core PUBLIC
        KUB3_HARDWARE_CONFIG_PATH="/etc/kub3/hardware.ini"
        KUB3_PROCESS_CONFIG_PATH="/etc/kub3/process.ini"
        KUB3_ADMIN_CONFIG_PATH="/etc/kub3/admin.ini"
        KUB3_ADMIN_VAULT_PATH="/etc/kub3/admin_vault.bin"
    )
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
    src/ViewModels/SettingsViewModel.cpp
    src/ViewModels/Exposure/ExposureSettingsViewModel.cpp
    src/ViewModels/Exposure/FavoriteExposureSettingsViewModel.cpp
    src/ViewModels/Exposure/RecapExposureSettingsViewModel.cpp
    src/ViewModels/Exposure/CompleteExposureViewModel.cpp
    src/ViewModels/Exposure/ProgressExposureViewModel.cpp
    src/ViewModels/Exposure/SaveExposureSettingsViewModel.cpp
    src/ViewModels/Settings/MachineStatusViewModel.cpp
    src/ViewModels/Settings/VersionViewModel.cpp
    src/ViewModels/Settings/TemperatureViewModel.cpp
    src/ViewModels/Settings/UpdateSoftwareViewModel.cpp
    src/ViewModels/Settings/ScreenshotExportViewModel.cpp
    src/ViewModels/Settings/LedTestViewModel.cpp
    src/ViewModels/Settings/OperatingTimesViewModel.cpp
    src/ViewModels/Alignment/LoadParametersViewModel.cpp
    src/ViewModels/Alignment/SaveParametersViewModel.cpp
    src/ViewModels/Alignment/VisualisationViewModel.cpp
    src/ViewModels/Alignment/DistanceViewModel.cpp
    src/Views/Exposure/ExposureSettingsView.cpp
    src/Views/Exposure/FavoriteExposureSettingsView.cpp
    src/Views/Exposure/RecapExposureSettingsView.cpp
    src/Views/Exposure/CompleteExposureView.cpp
    src/Views/Exposure/ProgressExposureView.cpp
    src/Views/Exposure/SaveExposureSettingsView.cpp
    src/Views/MainWindow.cpp
    src/Views/ViewBase.cpp
    src/Views/KeyboardConnections.cpp
    src/Views/AlignmentViewBase.cpp
    src/Views/SettingsViewBase.cpp
    src/Views/ExposureViewBase.cpp
    src/Views/HomeView.cpp
    src/Views/HomeEightView.cpp
    src/Views/SettingsView.cpp
    src/Views/Settings/MachineStatusView.cpp
    src/Views/Settings/VersionView.cpp
    src/Views/Settings/TemperatureView.cpp
    src/Views/Settings/UpdateSoftwareView.cpp
    src/Views/Settings/ScreenshotExportView.cpp
    src/Views/Settings/LedTestView.cpp
    src/Views/Settings/OperatingTimesView.cpp
    src/Views/Alignment/LoadParametersView.cpp
    src/Views/Alignment/SaveParametersView.cpp
    src/Views/Alignment/VisualisationView.cpp
    src/Views/Alignment/DistanceView.cpp
    src/Views/Components/FavoriteExposureSettingButton.cpp
    src//Views/Components/MessageOverlay.cpp
    src/Views/Components/NavButtonManager.cpp
    src/ViewModels/BaseVisionViewModel.cpp
    src/Views/Modules/CamerasTestView.cpp
    src/Views/Components/CameraStreamWidget.cpp
    src/Views/Components/NavButton.cpp
    src/Views/Components/UpBar.cpp
    src/Views/Components/ActionBox.cpp
    src/Views/Components/BottomCroppedCircle.cpp
    src/Views/Components/DoubleRatioSlider.cpp
    # Headers
    include/ApplicationBuilder.h
    include/ViewModels/HomeViewModel.h
    include/ViewModels/SettingsViewModel.h
    include/ViewModels/Exposure/ExposureSettingsViewModel.h
    include/ViewModels/Exposure/FavoriteExposureSettingsViewModel.h
    include/ViewModels/Exposure/RecapExposureSettingsViewModel.h
    include/ViewModels/Exposure/CompleteExposureViewModel.h
    include/ViewModels/Exposure/ProgressExposureViewModel.h
    include/ViewModels/Exposure/SaveExposureSettingsViewModel.h
    include/ViewModels/Settings/MachineStatusViewModel.h
    include/ViewModels/Settings/VersionViewModel.h
    include/ViewModels/Settings/TemperatureViewModel.h
    include/ViewModels/Settings/UpdateSoftwareViewModel.h
    include/ViewModels/Settings/ScreenshotExportViewModel.h
    include/ViewModels/Settings/LedTestViewModel.h
    include/ViewModels/Settings/OperatingTimesViewModel.h
    include/ViewModels/Alignment/LoadParametersViewModel.h
    include/ViewModels/Alignment/SaveParametersViewModel.h
    include/ViewModels/Alignment/VisualisationViewModel.h
    include/ViewModels/Alignment/DistanceViewModel.h
    include/Views/Exposure/ExposureSettingsView.h
    include/Views/Exposure/FavoriteExposureSettingsView.h
    include/Views/Exposure/RecapExposureSettingsView.h
    include/Views/Exposure/CompleteExposureView.h
    include/Views/Exposure/ProgressExposureView.h
    include/Views/Exposure/SaveExposureSettingsView.h
    include/Views/MainWindow.h
    include/Views/ViewBase.h
    include/Views/KeyboardConnections.h
    include/Views/AlignmentViewBase.h
    include/Views/SettingsViewBase.h
    include/Views/ExposureViewBase.h
    include/Views/HomeView.h
    include/Views/HomeEightView.h
    include/Views/SettingsView.h
    include/Views/Settings/MachineStatusView.h
    include/Views/Settings/TemperatureView.h
    include/Views/Settings/VersionView.h
    include/Views/Settings/UpdateSoftwareView.h
    include/Views/Settings/ScreenshotExportView.h
    include/Views/Settings/LedTestView.h
    include/Views/Settings/OperatingTimesView.h
    include/Views/Alignment/LoadParametersView.h
    include/Views/Alignment/SaveParametersView.h
    include/Views/Alignment/VisualisationView.h
    include/Views/Alignment/DistanceView.h
    include/Views/Components/FavoriteExposureSettingButton.h
    include/Views/Components/NavButtonManager.h
    include/Views/Components/MessageOverlay.h
    include/ViewModels/BaseVisionViewModel.h
    include/Views/Modules/CamerasTestView.h
    include/Views/Components/CameraStreamWidget.h
    include/Views/Components/NavButton.h
    include/Views/Components/UpBar.h
    include/Views/Components/ActionBox.h
    include/Views/Components/BottomCroppedCircle.h
    include/Views/Components/DoubleRatioSlider.h
    # UI
    gui/resources.qrc
    gui/MainWindow.ui
    gui/Views/Exposure/ExposureSettingsView.ui
    gui/Views/Exposure/FavoriteExposureSettingsView.ui
    gui/Views/Exposure/RecapExposureSettingsView.ui
    gui/Views/Exposure/CompleteExposureView.ui
    gui/Views/Exposure/ProgressExposureView.ui
    gui/Views/HomeView.ui
    gui/Views/HomeEightView.ui
    gui/Views/SettingsView.ui
    gui/Views/Settings/MachineStatusView.ui
    gui/Views/Settings/VersionView.ui
    gui/Views/Settings/TemperatureView.ui
    gui/Views/Settings/UpdateSoftwareView.ui
    gui/Views/Settings/ScreenshotExportView.ui
    gui/Views/Settings/LedTestView.ui
    gui/Views/Settings/OperatingTimesView.ui
    gui/Views/Alignment/LoadParametersView.ui
    gui/Views/Alignment/SaveParametersView.ui
    gui/Views/Alignment/VisualisationView.ui
    gui/Views/Alignment/DistanceView.ui
)

qt_finalize_target(Kub3Software)

# Everything defined as PUBLIC on linked libraries is automatically inherited here
target_link_libraries(Kub3Software PRIVATE
    Kub3Core
    Qt6::Gui
    Qt6::Widgets
)

# ==============================================================================
# 6. STANDALONE TOOLS
# ==============================================================================
add_subdirectory(tools/configurator)
add_subdirectory(tools/tester)
add_subdirectory(tools/config_generator)

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

# Install the main executable into the "bin" dir of the output folder
install(TARGETS Kub3Software    RUNTIME DESTINATION bin)

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
