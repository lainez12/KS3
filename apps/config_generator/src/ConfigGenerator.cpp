#include "ConfigGenerator.h"

#include <QDebug>
#include <QSettings>
#include <QVariant>

// Include your macro definitions
#include <Config/keys/admin.h>
#include <Config/keys/hardware.h>
#include <Config/keys/process.h>

namespace Kub3::Config
{

    // Helper: Only writes the value if the key does not already exist and logs it
    static void setDefault(QSettings &settings, const QString &key, const QVariant &value, int &filledCount)
    {
        if (!settings.contains(key))
        {
            settings.setValue(key, value);

            // Construct the full configuration path for logging (e.g., "cameras/upperLeftCamera/max_exposure_us")
            QString fullPath = settings.group();
            if (!fullPath.isEmpty())
            {
                fullPath += "/";
            }
            fullPath += key;

            qInfo().noquote() << QString("  [FILLED] %1 = %2").arg(fullPath).arg(value.toString());
            filledCount++;
        }
    }

    // Helper: Populates a Stepper motor
    static void setDefaultStepper(QSettings &s, const QString &id, double maxAcc, double maxVel, double pitch, int stepsPerRev, int encoderTops, int &filledCount)
    {
        s.beginGroup(id);
        setDefault(s, CONF_HW_MOTOR_TYPE, CONF_HW_MOTOR_TYPE_STEPPER, filledCount);
        setDefault(s, CONF_HW_MOTOR_STEPS_PER_REV, stepsPerRev, filledCount);
        setDefault(s, CONF_HW_SCREW_PITCH_MM, pitch, filledCount);
        setDefault(s, CONF_HW_MAX_VELOCITY_MM_S, maxVel, filledCount);
        setDefault(s, CONF_HW_MAX_ACCELERATION_MM_S2, maxAcc, filledCount);
        setDefault(s, CONF_HW_ENCODER_TOPS_PER_REV, encoderTops, filledCount);
        s.endGroup();
    }

    // Helper: Populates a Kinematic profile
    static void setDefaultKinematic(QSettings &s, const QString &motorId, const QString &profileId, double acc, double targetVel, int stepFrac, int &filledCount)
    {
        s.beginGroup(motorId);
        s.beginGroup(profileId);
        setDefault(s, CONF_PROCESS_PARAMS_TYPE, CONF_PROCESS_PARAMS_TYPE_STEPPER, filledCount);
        setDefault(s, CONF_PROCESS_TARGET_VELOCITY_MM_S, targetVel, filledCount);
        setDefault(s, CONF_PROCESS_ACCELERATION_MM_S, acc, filledCount);
        setDefault(s, CONF_PROCESS_STEP_FRACTION, stepFrac, filledCount);
        s.endGroup();
        s.endGroup();
    }

    // Helper: Populates a Camera
    static void setDefaultCamera(QSettings &s, const QString &id, const QString &serial, double defExp, double defFramerate, double defGain, double maxExp, double maxGain, int &filledCount)
    {
        s.beginGroup(id);
        setDefault(s, CONF_HW_SERIAL_NUMBER, serial, filledCount);
        setDefault(s, CONF_HW_DEFAULT_EXPOSURE_US, defExp, filledCount);
        setDefault(s, CONF_HW_FRAMERATE, defFramerate, filledCount);
        setDefault(s, CONF_HW_DEFAULT_GAIN_DB, defGain, filledCount);
        setDefault(s, CONF_HW_MAX_EXPOSURE_US, maxExp, filledCount);
        setDefault(s, CONF_HW_MAX_GAIN_DB, maxGain, filledCount);
        s.endGroup();
    }

    // Helper: Populates a Focal device
    static void setDefaultFocal(QSettings &s, const QString &id, int defVal, int minVal, int maxVal, int &filledCount)
    {
        s.beginGroup(id);
        setDefault(s, CONF_PROCESS_FOCAL_DEFAULT_VALUE, defVal, filledCount);
        setDefault(s, CONF_PROCESS_FOCAL_MIN_VALUE, minVal, filledCount);
        setDefault(s, CONF_PROCESS_FOCAL_MAX_VALUE, maxVal, filledCount);
        s.endGroup();
    }

    void ConfigGenerator::generateDefaults(const QString &hwPath, const QString &procPath, const QString &adminPath)
    {
        qInfo() << "[ConfigGenerator] Scanning and filling missing default values...";

#if defined(KUB_MODEL_8)
        generateModel8(hwPath, procPath, adminPath);
#else
        qWarning() << "[ConfigGenerator] Compilation flag defining the machine (e.g. KUB_MODEL_8) is missing.";
        qWarning() << "[ConfigGenerator] Generation skipped. Nothing was altered.";
#endif
    }

    void ConfigGenerator::generateModel8(const QString &hwPath, const QString &procPath, const QString &adminPath)
    {
        int filledCount = 0;

        // =========================================================
        // ADMIN CONFIGURATION
        // =========================================================
        {
            QSettings admin(adminPath, QSettings::IniFormat);
            setDefault(admin, CONF_ADMIN_KLOE_MODE, false, filledCount);
            admin.sync();
        }

        // =========================================================
        // HARDWARE CONFIGURATION
        // =========================================================
        {
            QSettings hw(hwPath, QSettings::IniFormat);

            // -- MCUs --
            hw.beginGroup(CONF_HW_MCUS);
            setDefault(hw, CONF_HW_MCU1_BAUDRATE, 115200, filledCount);
            setDefault(hw, CONF_HW_MCU1_PORT, "/dev/ttyACM0", filledCount);
            setDefault(hw, CONF_HW_MCU2_BAUDRATE, 115200, filledCount);
            setDefault(hw, CONF_HW_MCU2_PORT, "/dev/ttyACM1", filledCount);
            setDefault(hw, CONF_HW_MCU3_BAUDRATE, 115200, filledCount);
            setDefault(hw, CONF_HW_MCU3_PORT, "/dev/ttyACM2", filledCount);
            setDefault(hw, CONF_HW_MCU4_BAUDRATE, 115200, filledCount);
            setDefault(hw, CONF_HW_MCU4_PORT, "/dev/ttyACM3", filledCount);
            hw.endGroup();

            // -- Motors --
            hw.beginGroup(CONF_HW_MOTORS);
            setDefaultStepper(hw, "deckMotor", 400.0, 40.0, 3.175, 400, 400, filledCount);
            setDefaultStepper(hw, "leftCameraXMotor", 400.0, 40.0, 3.175, 400, 400, filledCount);
            setDefaultStepper(hw, "leftCameraYMotor", 400.0, 40.0, 3.175, 400, 400, filledCount);
            setDefaultStepper(hw, "maskDrawerMotor", 400.0, 40.0, 3.175, 400, 400, filledCount);
            setDefaultStepper(hw, "rightCameraXMotor", 400.0, 40.0, 3.175, 400, 400, filledCount);
            setDefaultStepper(hw, "rightCameraYMotor", 400.0, 40.0, 3.175, 400, 400, filledCount);
            setDefaultStepper(hw, "thetaStageMotor", 400.0, 40.0, 3.175, 400, 400, filledCount);
            setDefaultStepper(hw, "waferDrawerMotor", 400.0, 40.0, 3.175, 400, 400, filledCount);
            setDefaultStepper(hw, "xStageMotor", 400.0, 40.0, 3.175, 400, 400, filledCount);
            setDefaultStepper(hw, "yStageMotor", 400.0, 40.0, 3.175, 400, 400, filledCount);

            // Z-Motors
            setDefaultStepper(hw, "zBackMotor", 400.0, 40.0, 0.5, 400, 2000, filledCount);
            setDefaultStepper(hw, "zLeftMotor", 400.0, 40.0, 0.5, 400, 2000, filledCount);
            setDefaultStepper(hw, "zRightMotor", 400.0, 40.0, 0.5, 400, 2000, filledCount);
            hw.endGroup();

            // -- Cameras --
            hw.beginGroup(CONF_HW_CAMERAS);
            setDefaultCamera(hw, "upperLeftCamera", "DA1051980", 10000.0, 30.0, 0.0, 50000.0, 24.0, filledCount);
            setDefaultCamera(hw, "upperRightCamera", "INVALID_CAMERA_ID", 10000.0, 59.97, 0.0, 50000.0, 24.0, filledCount);
            hw.endGroup();

            // -- Force Sensors --
            hw.beginGroup(CONF_HW_FORCE_SENSORS);
            hw.beginGroup("backForceSensor");
            setDefault(hw, CONF_HW_ADC_TO_GRAM_FORCE_FACTOR, 1.0, filledCount);
            hw.endGroup();
            hw.beginGroup("leftForceSensor");
            setDefault(hw, CONF_HW_ADC_TO_GRAM_FORCE_FACTOR, 1.0, filledCount);
            hw.endGroup();
            hw.beginGroup("rightForceSensor");
            setDefault(hw, CONF_HW_ADC_TO_GRAM_FORCE_FACTOR, 1.0, filledCount);
            hw.endGroup();
            hw.endGroup();

            hw.sync();
        }

        // =========================================================
        // PROCESS CONFIGURATION
        // =========================================================
        {
            QSettings proc(procPath, QSettings::IniFormat);

            // -- Admittance --
            proc.beginGroup(CONF_PROCESS_ADMITTANCE_TUNING);
            setDefault(proc, CONF_PROCESS_ADMITTANCE_MAX_STEP_MM_PER_TICK, 0.25, filledCount);
            setDefault(proc, CONF_PROCESS_ADMITTANCE_DEADBAND_VELOCITY_MM_S, 0.1, filledCount);
            setDefault(proc, CONF_PROCESS_ADMITTANCE_ROTATION_GAIN_HIGH_FORCE, 0.00075, filledCount);
            setDefault(proc, CONF_PROCESS_ADMITTANCE_ROTATION_GAIN_LOW_FORCE, 0.015, filledCount);
            setDefault(proc, CONF_PROCESS_ADMITTANCE_TRANSLATION_GAIN_HIGH_FORCE, 0.00075, filledCount);
            setDefault(proc, CONF_PROCESS_ADMITTANCE_TRANSLATION_GAIN_LOW_FORCE, 0.015, filledCount);
            proc.endGroup();

            // -- Alignment Positions --
            proc.beginGroup(CONF_PROCESS_ALIGNMENT_POSITIONS);
            setDefault(proc, CONF_PROCESS_THETA_STAGE_CENTER_POS_MM, 3501.2031, filledCount);
            setDefault(proc, CONF_PROCESS_X_STAGE_CENTER_POS_MM, 5001.0, filledCount);
            setDefault(proc, CONF_PROCESS_Y_STAGE_CENTER_POS_MM, 5002.0, filledCount);
            proc.endGroup();

            // -- Cameras --
            proc.beginGroup(CONF_PROCESS_CAMERAS);
            setDefault(proc, CONF_PROCESS_LEFT_CAM_X_HOME_POS_MM, 0.0, filledCount);
            setDefault(proc, CONF_PROCESS_LEFT_CAM_X_RESET_POS_MM, 30.0, filledCount);
            setDefault(proc, CONF_PROCESS_LEFT_CAM_Y_HOME_POS_MM, 0.0, filledCount);
            setDefault(proc, CONF_PROCESS_LEFT_CAM_Y_RESET_POS_MM, 30.0, filledCount);
            setDefault(proc, CONF_PROCESS_RIGHT_CAM_X_HOME_POS_MM, 0.0, filledCount);
            setDefault(proc, CONF_PROCESS_RIGHT_CAM_X_RESET_POS_MM, 30.0, filledCount);
            setDefault(proc, CONF_PROCESS_RIGHT_CAM_Y_HOME_POS_MM, 0.0, filledCount);
            setDefault(proc, CONF_PROCESS_RIGHT_CAM_Y_RESET_POS_MM, 30.0, filledCount);
            setDefault(proc, CONF_PROCESS_MIN_CAMERA_DISTANCE_MM, 50.0, filledCount);

            setDefaultFocal(proc, CONF_PROCESS_LEFT_FOCAL, 149, 0, 4095, filledCount);
            setDefaultFocal(proc, CONF_PROCESS_RIGHT_FOCAL, 1492, 0, 4095, filledCount);
            proc.endGroup();

            // -- Drawers Positions --
            proc.beginGroup(CONF_PROCESS_DRAWERS_POSITIONS);
            setDefault(proc, CONF_PROCESS_CM3_RESET_POS_MM, 12345.0, filledCount);
            proc.endGroup();

            // -- Force Limits --
            proc.beginGroup(CONF_PROCESS_FORCE_LIMITS);
            setDefault(proc, CONF_PROCESS_AUTOLEVEL_FORCE_GF, 700.0, filledCount);
            setDefault(proc, CONF_PROCESS_AUTOLEVEL_FORCE_TOLERANCE_GF, 5.0, filledCount);
            setDefault(proc, CONF_PROCESS_CONTACT_THRESHOLD_GF, 10.0, filledCount);
            setDefault(proc, CONF_PROCESS_HW_CRASH_FORCE_LIMIT_GF, 8000.0, filledCount);
            setDefault(proc, CONF_PROCESS_MAX_FORCE_GF, 4000.0, filledCount);
            proc.endGroup();

            // -- Z Elevator --
            proc.beginGroup(CONF_PROCESS_ELEVATOR_POSITIONS);
            setDefault(proc, CONF_PROCESS_MAX_Z_RELATIVE_DISTANCE_MM, 10.0, filledCount);
            proc.endGroup();

            // -- Kinematics --
            proc.beginGroup(CONF_PROCESS_KINEMATICS);
            setDefaultKinematic(proc, "deckMotor", "normal", 400.0, 100.0, 1, filledCount);

            setDefaultKinematic(proc, "leftCameraXMotor", "fine", 400.0, 100.0, 8, filledCount);
            setDefaultKinematic(proc, "leftCameraXMotor", "normal", 400.0, 100.0, 1, filledCount);

            setDefaultKinematic(proc, "leftCameraYMotor", "fine", 400.0, 100.0, 8, filledCount);
            setDefaultKinematic(proc, "leftCameraYMotor", "normal", 400.0, 100.0, 1, filledCount);

            setDefaultKinematic(proc, "maskDrawerMotor", "contact", 0.0, 1.0, 16, filledCount);
            setDefaultKinematic(proc, "maskDrawerMotor", "fine", 400.0, 25.0, 8, filledCount);
            setDefaultKinematic(proc, "maskDrawerMotor", "normal", 400.0, 100.0, 1, filledCount);

            setDefaultKinematic(proc, "rightCameraXMotor", "fine", 400.0, 100.0, 1, filledCount);
            setDefaultKinematic(proc, "rightCameraXMotor", "normal", 410.0, 100.0, 1, filledCount);

            setDefaultKinematic(proc, "rightCameraYMotor", "fine", 400.0, 100.0, 8, filledCount);
            setDefaultKinematic(proc, "rightCameraYMotor", "normal", 400.0, 100.0, 1, filledCount);

            setDefaultKinematic(proc, "thetaStageMotor", "fine", 400.0, 100.0, 8, filledCount);
            setDefaultKinematic(proc, "thetaStageMotor", "normal", 400.0, 100.0, 1, filledCount);

            setDefaultKinematic(proc, "waferDrawerMotor", "fine", 400.0, 25.0, 1, filledCount);
            setDefaultKinematic(proc, "waferDrawerMotor", "normal", 400.0, 100.0, 16, filledCount);

            setDefaultKinematic(proc, "xStageMotor", "fine", 400.0, 100.0, 1, filledCount);
            setDefaultKinematic(proc, "xStageMotor", "normal", 110.0, 110.0, 1, filledCount);

            setDefaultKinematic(proc, "yStageMotor", "fine", 400.0, 100.0, 8, filledCount);
            setDefaultKinematic(proc, "yStageMotor", "normal", 400.0, 100.0, 1, filledCount);

            setDefaultKinematic(proc, "zBackMotor", "fine", 1.0, 0.5, 16, filledCount);
            setDefaultKinematic(proc, "zBackMotor", "normal", 1.0, 1.0, 8, filledCount);

            setDefaultKinematic(proc, "zLeftMotor", "fine", 2.0, 0.5, 16, filledCount);
            setDefaultKinematic(proc, "zLeftMotor", "normal", 1.0, 1.0, 8, filledCount);

            setDefaultKinematic(proc, "zRightMotor", "fine", 1.0, 0.5, 16, filledCount);
            setDefaultKinematic(proc, "zRightMotor", "normal", 1.0, 1.0, 8, filledCount);
            proc.endGroup(); // kinematics

            proc.sync();
        }

        // =========================================================
        // SUMMARY
        // =========================================================
        if (filledCount > 0)
        {
            qInfo() << "[ConfigGenerator] Successfully filled" << filledCount << "missing fields.";
        }
        else
        {
            qInfo() << "[ConfigGenerator] All configurations are already up-to-date. No fields were added.";
        }
    }

} // namespace Kub3::Config
