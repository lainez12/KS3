# Generate version.h
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/version.h"
)

# Core does not need UI features enabled
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)

qt_add_library(Kub3Core STATIC
    # Sources
    src/utils.cpp
    src/Config/ConfigLoader.cpp
    src/Config/ConfigSaver.cpp
    src/Views/Components/CameraStreamWidget.cpp
    src/Views/Components/DoubleRatioSlider.cpp
    src/Views/Components/KeyboardFilter.cpp
    src/Views/Traits/PadReceiverViewTrait.cpp
    src/ViewModels/BaseViewModel.cpp
    src/ViewModels/BaseVisionViewModel.cpp
    src/MFSM/MasterFSM.actions.cpp
    src/MFSM/MasterFSM.core.cpp
    src/MFSM/MasterFSM.heartbeat.cpp
    src/MFSM/MasterFSM.transitions.macro.cpp
    src/MFSM/MasterFSM.transitions.operational.cpp
    src/MFSM/interlocks.cpp
    src/Services/tasks/ReachLimitTask.cpp
    src/Services/tasks/ToggleValveTask.cpp
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
    src/Services/Homing/tasks/Initialization/ForceSensorsTareTask.cpp
    src/Services/Drawers/DualConveyorDrawerService.cpp
    src/Services/Drawers/SingleConveyorDrawerService.cpp
    src/Services/Drawers/tasks/MaskEjectionTask.cpp
    src/Services/Drawers/tasks/WaferEjectionTask.cpp
    src/Services/Stowage/StowageService.cpp
    src/Services/Stowage/tasks/StowageMoveZToLimitTask.cpp
    src/Services/Alignment/AlignmentService.cpp
    src/Services/Vision/VisionService.cpp
    src/Services/Contact/ContactService.cpp
    src/Services/Contact/tasks/AdmittanceControlTask.cpp
    src/Services/Contact/tasks/FastApproachTask.cpp
    src/Services/Contact/tasks/SaveCurrentPlanTask.cpp
    src/Services/Exposure/ExposureService.cpp
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
    src/HAL/Actuators/Switches/LogicSwitch.cpp
    src/Algorithms/Kinematic/utils.cpp
    src/Algorithms/Kinematic/TrapezoidalGenerator.cpp
    src/Algorithms/Admittance/AdmittanceController.cpp
    src/Algorithms/Filters/TimeAwareEMAFilter.cpp

    # Headers
    include/utils.h
    include/Common/Enums.h
    include/Common/TestToken.h
    include/Config/ConfigLoader.h
    include/Config/ConfigSaver.h
    include/Config/kinematics.h
    include/Config/conf.h
    include/Config/hardware.h
    include/Config/process.h
    include/Config/admin.h
    include/Config/helper.h
    include/Config/keys/admin.h
    include/Config/keys/hardware.h
    include/Config/keys/process.h
    include/Config/default/admin.h
    include/Config/default/hardware.h
    include/Config/default/process.h
    include/Views/Components/CameraStreamWidget.h
    include/Views/Components/DoubleRatioSlider.h
    include/Views/Components/KeyboardFilter.h
    include/Views/Traits/PadReceiverViewTrait.h
    include/ViewModels/BaseViewModel.h
    include/ViewModels/BaseVisionViewModel.h
    include/MFSM/events.h
    include/MFSM/interlocks.h
    include/MFSM/MasterFSM.h
    include/MFSM/posture.h
    include/MFSM/states.macro.h
    include/MFSM/states.operational.h
    include/Services/ITask.h
    include/Services/IService.h
    include/Services/BaseTaskService.h
    include/Services/tasks/ReachLimitTask.h
    include/Services/tasks/ToggleValveTask.h
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
    include/Services/Homing/tasks/Initialization/ForceSensorsTareTask.h
    include/Services/Drawers/IDrawerService.h
    include/Services/Drawers/DualConveyorDrawerService.h
    include/Services/Drawers/SingleConveyorDrawerService.h
    include/Services/Drawers/tasks/MaskInsertionTask.h
    include/Services/Drawers/tasks/MaskEjectionTask.h
    include/Services/Drawers/tasks/WaferInsertionTask.h
    include/Services/Drawers/tasks/WaferEjectionTask.h
    include/Services/Stowage/IStowageService.h
    include/Services/Stowage/StowageService.h
    include/Services/Stowage/tasks/StowageCenterStagesTask.h
    include/Services/Stowage/tasks/StowageMoveZToLimitTask.h
    include/Services/Alignment/IAlignmentService.h
    include/Services/Alignment/AlignmentService.h
    include/Services/Vision/IVisionService.h
    include/Services/Vision/VisionService.h
    include/Services/Contact/IContactService.h
    include/Services/Contact/ContactService.h
    include/Services/Contact/tasks/AdmittanceControlTask.h
    include/Services/Contact/tasks/FastApproachTask.h
    include/Services/Contact/tasks/SaveCurrentPlanTask.h
    include/Services/Exposure/ExposureService.h
    include/Services/Exposure/IExposureService.h
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
    include/HAL/Actuators/Switches/ISwitch.h
    include/HAL/Actuators/Switches/LogicSwitch.h
    include/Algorithms/Kinematic/utils.h
    include/Algorithms/Kinematic/IKinematicGenerator.h
    include/Algorithms/Kinematic/TrapezoidalGenerator.h
    include/Algorithms/Admittance/AdmittanceController.h
    include/Algorithms/Filters/TimeAwareEMAFilter.h
)

target_include_directories(Kub3Core PUBLIC
    include
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

# Core dependencies (Shared downward)
target_link_libraries(Kub3Core PUBLIC Qt6::Core Qt6::SerialPort Qt6::Gui Qt6::Widgets)

if(MVS_LIBRARY)
    target_link_libraries(Kub3Core PUBLIC ${MVS_LIBRARY})
    target_include_directories(Kub3Core PUBLIC ${MVS_INCLUDE_DIR})
endif()

# Global build definitions applied to everyone interacting with Core
target_compile_definitions(Kub3Core PUBLIC KUB_MODEL_${KUB_MODEL} KUB_MODEL_STR="${KUB_MODEL}")

if (CMAKE_PRODUCTION_BUILD)
    target_compile_definitions(Kub3Core PUBLIC BUILD_PRODUCTION)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT CMAKE_PRODUCTION_BUILD)
    target_compile_definitions(Kub3Core PUBLIC BUILD_DEBUG)
    target_compile_definitions(Kub3Core PUBLIC
        KUB3_HARDWARE_CONFIG_PATH="${CMAKE_SOURCE_DIR}/tests/hardware.ini"
        KUB3_PROCESS_CONFIG_PATH="${CMAKE_SOURCE_DIR}/tests/process.ini"
        KUB3_ADMIN_CONFIG_PATH="${CMAKE_SOURCE_DIR}/tests/admin.ini"
        KUB3_ADMIN_VAULT_PATH="${CMAKE_SOURCE_DIR}/tests/admin_vault.bin"
    )
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(Kub3Core PUBLIC BUILD_DEBUG)
    else()
        target_compile_definitions(Kub3Core PUBLIC BUILD_RELEASE)
    endif()
    target_compile_definitions(Kub3Core PUBLIC
        KUB3_HARDWARE_CONFIG_PATH="/etc/kub3/hardware.ini"
        KUB3_PROCESS_CONFIG_PATH="/etc/kub3/process.ini"
        KUB3_ADMIN_CONFIG_PATH="/etc/kub3/admin.ini"
        KUB3_ADMIN_VAULT_PATH="/etc/kub3/admin_vault.bin"
    )
endif()
