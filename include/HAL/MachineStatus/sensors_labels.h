#pragma once

#if defined(KUB_MODEL_8)

// TODO: We could move the value keys like `MCU_ARDUINO2_ID` to another file `mcu_conn_labels.h`
// We should also explore the fact that holding the values corresponding to those keys is not intuitive
// and that they could be stored in a different repository. But the current repository is a `MachineStatusRepo`
// and maybe what is bothering is just the fact that these values are registered using the class `Sensor`

// Hardware Buttons
#define EMERGENCY_STOP_BUTTON "emergencyStop"
#define POWER_OFF_BUTTON      "powerOff"

// Subsystem Routing IDs
#define MCU_ARDUINO1_ID "SYS_MCU_ARDUINO1"
#define MCU_ARDUINO2_ID "SYS_MCU_ARDUINO2"
#define MCU_ARDUINO3_ID "SYS_MCU_ARDUINO3"

// Data Bus Keys
#define MCU_ARDUINO1_READY "SYS_MCU_ARDUINO1_READY"
#define MCU_ARDUINO2_READY "SYS_MCU_ARDUINO2_READY"
#define MCU_ARDUINO3_READY "SYS_MCU_ARDUINO3_READY"

// Encoders
#define WAFER_ENCODER          "waferDrawerEncoderPosition"
#define MASK_ENCODER           "maskDrawerEncoderPosition"
#define DECK_MOTOR_ENCODER     "camerasDeckEncoderPosition"
#define Z_LEFT_ENCODER         "zLeftEncoderPosition"
#define Z_RIGHT_ENCODER        "zRightEncoderPosition"
#define Z_BACK_ENCODER         "zBackEncoderPosition"
#define LEFT_CAMERA_X_ENCODER  "leftCameraXEncoder"
#define LEFT_CAMERA_Y_ENCODER  "leftCameraYEncoder"
#define RIGHT_CAMERA_X_ENCODER "rightCameraXEncoder"
#define RIGHT_CAMERA_Y_ENCODER "rightCameraYEncoder"
#define X_STAGE_ENCODER        "xStageEncoder"
#define Y_STAGE_ENCODER        "yStageEncoder"
#define THETA_STAGE_ENCODER    "thetaStageEncoder"

// Limit switches
// --- Cameras
#define LEFT_CAMERA_X_LEFT_LIMIT   "leftCameraXLeftLimit"
#define LEFT_CAMERA_X_RIGHT_LIMIT  "leftCameraXRightLimit"
#define LEFT_CAMERA_Y_FRONT_LIMIT  "leftCameraYFrontLimit"
#define LEFT_CAMERA_Y_BACK_LIMIT   "leftCameraYBackLimit"
#define RIGHT_CAMERA_X_LEFT_LIMIT  "rightCameraXLeftLimit"
#define RIGHT_CAMERA_X_RIGHT_LIMIT "rightCameraXRightLimit"
#define RIGHT_CAMERA_Y_FRONT_LIMIT "rightCameraYFrontLimit"
#define RIGHT_CAMERA_Y_BACK_LIMIT  "rightCameraYBackLimit"
// --- Z
#define Z_LEFT_HIGH_LIMIT  "zLeftHighLimit"
#define Z_LEFT_LOW_LIMIT   "zLeftLowLimit"
#define Z_RIGHT_HIGH_LIMIT "zRightHighLimit"
#define Z_RIGHT_LOW_LIMIT  "zRightLowLimit"
#define Z_BACK_HIGH_LIMIT  "zBackHighLimit"
#define Z_BACK_LOW_LIMIT   "zBackLowLimit"
#define Z1                 "z1Limit"
#define WAFER_ON           "waferOnLimit"
#define Z2                 "z2Limit"
// --- Conveyors
#define CM0 "maskCM0Limit"
#define CM1 "maskCM1Limit"
#define CM2 "maskCM2Limit"
#define CM3 "maskCM3Limit"
#define CW0 "waferCW0Limit"
#define CW1 "waferCW1Limit"
#define CW2 "waferCW2Limit"
// --- Camera's Deck
#define DECK_FRONT_LIMIT "deckFrontLimit"
#define DECK_BACK_LIMIT  "deckBackLimit"
// --- Ardko
#define ARDKO_FRONT_LEFT_LIMIT  "ardkoFrontLeftLimit"
#define ARDKO_FRONT_RIGHT_LIMIT "ardkoFrontRightLimit"
#define ARDKO_BACK_LEFT_LIMIT   "ardkoBackLeftLimit"
#define ARDKO_BACK_RIGHT_LIMIT  "ardkoBackRightLimit"
// --- X, Y, Theta
#define X_STAGE_LEFT_LIMIT               "xStageLeftLimit"
#define X_STAGE_RIGHT_LIMIT              "xStageRightLimit"
#define Y_STAGE_FRONT_LIMIT              "yStageFrontLimit"
#define Y_STAGE_BACK_LIMIT               "yStageBackLimit"
#define THETA_STAGE_CLOCKWISE_LIMIT      "thetaStageClockwiseLimit"
#define THETA_STAGE_ANTI_CLOCKWISE_LIMIT "thetaStageAntiClockwiseLimit"

// Force sensors
#define FORCE_LEFT     "leftForceSensor"
#define FORCE_RIGHT    "rightForceSensor"
#define FORCE_BACK     "backForceSensor"
#define FORCE_LEFT_EN  "leftForceSensorEnabled"
#define FORCE_RIGHT_EN "rightForceSensorEnabled"
#define FORCE_BACK_EN  "backForceSensorEnabled"

// Solenoid valves
#define MASK_VACUUM_VALVE_STATUS          "maskVacuumValveStatus"
#define WAFER_VACUUM_VALVE_STATUS         "waferVacuumValveStatus"
#define WAFER_COMPRESSED_AIR_VALVE_STATUS "waferCompressedAirValveStatus"

// Pressure sensors
#define MASK_VACUUM_ACTIVE          "maskVacuumActive"
#define WAFER_VACUUM_ACTIVE         "waferVacuumActive"
#define WAFER_COMPRESSED_AIR_ACTIVE "waferCompressedAirActive"

// Temperature sensors
#define INTERNAL_TEMPERATURE "internalTemperature"
#define EXTERNAL_TEMPERATURE "externalTemperature"

#endif // defined(KUB_MODEL_8)
