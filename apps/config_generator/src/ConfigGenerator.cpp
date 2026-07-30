#include "ConfigGenerator.h"

#include <QDebug>
#include <QSettings>
#include <QVariant>

#include <Config/ConfigLoader.h>
#include <Config/ConfigSaver.h>
#include <Config/conf.h>
#include <Config/keys/admin.h>
#include <Config/keys/hardware.h>
#include <Config/keys/process.h>
#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/Vision/identifiers.h>

namespace Kub3::Config
{

    void ConfigGenerator::generateDefaults(const std::string &hwPath, const std::string &procPath, const std::string &adminPath)
    {
        qInfo() << "[ConfigGenerator] Scanning and filling missing default values...";

#if defined(KUB_MODEL_8)
        generateModel8(hwPath, procPath, adminPath);
#else
        qWarning() << "[ConfigGenerator] Compilation flag defining the machine (e.g. KUB_MODEL_8) is missing.";
        qWarning() << "[ConfigGenerator] Generation skipped. Nothing was altered.";
#endif
    }

    void ConfigGenerator::generateModel8(const std::string &hwPath, const std::string &procPath, const std::string &adminPath)
    {
        uint32_t filledCount = 0u;

        ConfigGenerator::generateModel8Admin(adminPath, filledCount);
        ConfigGenerator::generateModel8Hardware(hwPath, filledCount);
        ConfigGenerator::generateModel8Process(procPath, filledCount);

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

    // =========================================================
    // ADMIN CONFIGURATION FILL-UP / GENERATION
    // =========================================================
    void ConfigGenerator::generateModel8Admin(const std::string &path, uint32_t &filledCount)
    {
        std::vector<std::string> changes;
        admin_config_t currentConfig = ConfigLoader::loadAdminConfig(path, false, &changes);

        for (const auto &change : changes)
        {
            qInfo().noquote() << "\t[FILLED]" << QString::fromStdString(change);
        }

        if (!changes.empty())
        {
            ConfigSaver::saveAdminConfig(currentConfig, path);
            filledCount += changes.size();
        }
    }

    // =========================================================
    // HARDWARE CONFIGURATION FILL-UP / GENERATION
    // =========================================================
    void ConfigGenerator::generateModel8Hardware(const std::string &path, uint32_t &filledCount)
    {
        std::vector<std::string> changes;
        hardware_config_t currentConfig       = ConfigLoader::loadHardwareConfig(path, false, &changes);
        const hardware_config_t defaultConfig = hardware_config_t{
            .motors = {
                // Cameras' deck
                {
                    DECK_MOTOR,
                    {
                        .id           = DECK_MOTOR,
                        .hwProperties = dc_motor_hw_properties_t{
                            .screwPitchMm        = 12.0,
                            .maxVelocityMmS      = 36.0,
                            .maxAccelerationMmS2 = 36.0,
                        },
                    },
                },
                // Cameras
                {
                    LEFT_CAMERA_X_MOTOR,
                    {
                        .id           = LEFT_CAMERA_X_MOTOR,
                        .hwProperties = stepper_hw_properties_t{
                            .stepsPerRev         = 200u,
                            .screwPitchMm        = 0.635,
                            .maxVelocityMmS      = 20.0,
                            .maxAccelerationMmS2 = 400.0,
                            .encoderTopsPerRev   = 1000u,
                        },
                    },
                },
                {
                    LEFT_CAMERA_Y_MOTOR,
                    {
                        .id           = LEFT_CAMERA_Y_MOTOR,
                        .hwProperties = stepper_hw_properties_t{
                            .stepsPerRev         = 200u,
                            .screwPitchMm        = 0.635,
                            .maxVelocityMmS      = 20.0,
                            .maxAccelerationMmS2 = 400.0,
                            .encoderTopsPerRev   = 1000u,
                        },
                    },
                },
                {
                    RIGHT_CAMERA_X_MOTOR,
                    {
                        .id           = RIGHT_CAMERA_X_MOTOR,
                        .hwProperties = stepper_hw_properties_t{
                            .stepsPerRev         = 200u,
                            .screwPitchMm        = 0.635,
                            .maxVelocityMmS      = 20.0,
                            .maxAccelerationMmS2 = 400.0,
                            .encoderTopsPerRev   = 1000u,
                        },
                    },
                },
                {
                    RIGHT_CAMERA_Y_MOTOR,
                    {
                        .id           = RIGHT_CAMERA_Y_MOTOR,
                        .hwProperties = stepper_hw_properties_t{
                            .stepsPerRev         = 200u,
                            .screwPitchMm        = 0.635,
                            .maxVelocityMmS      = 20.0,
                            .maxAccelerationMmS2 = 400.0,
                            .encoderTopsPerRev   = 1000u,
                        },
                    },
                },
                // Alignment stages
                {
                    X_STAGE_MOTOR,
                    {
                        .id           = X_STAGE_MOTOR,
                        .hwProperties = stepper_hw_properties_t{
                            .stepsPerRev         = 200u,
                            .screwPitchMm        = 0.3,
                            .maxVelocityMmS      = 16.0,
                            .maxAccelerationMmS2 = 50.0,
                            .encoderTopsPerRev   = 1000u,
                        },
                    },
                },
                {
                    Y_STAGE_MOTOR,
                    {
                        .id           = Y_STAGE_MOTOR,
                        .hwProperties = stepper_hw_properties_t{
                            .stepsPerRev         = 200u,
                            .screwPitchMm        = 0.3,
                            .maxVelocityMmS      = 16.0,
                            .maxAccelerationMmS2 = 50.0,
                            .encoderTopsPerRev   = 1000u,
                        },
                    },
                },
                {
                    THETA_STAGE_MOTOR,
                    {
                        .id           = THETA_STAGE_MOTOR,
                        .hwProperties = stepper_hw_properties_t{
                            .stepsPerRev         = 200u,
                            .screwPitchMm        = 0.3,
                            .maxVelocityMmS      = 16.0,
                            .maxAccelerationMmS2 = 50.0,
                            .encoderTopsPerRev   = 1000u,
                        },
                    },
                },
                // Z Elevators
                {
                    Z_LEFT_MOTOR,
                    {
                        .id           = Z_LEFT_MOTOR,
                        .hwProperties = stepper_hw_properties_t{
                            .stepsPerRev         = 400u,
                            .screwPitchMm        = 0.5,
                            .maxVelocityMmS      = 12.0,
                            .maxAccelerationMmS2 = 999.999,
                            .encoderTopsPerRev   = 2000u,
                        },
                    },
                },
                {
                    Z_RIGHT_MOTOR,
                    {
                        .id           = Z_RIGHT_MOTOR,
                        .hwProperties = stepper_hw_properties_t{
                            .stepsPerRev         = 400u,
                            .screwPitchMm        = 0.5,
                            .maxVelocityMmS      = 12.0,
                            .maxAccelerationMmS2 = 999.999,
                            .encoderTopsPerRev   = 2000u,
                        },
                    },
                },
                {
                    Z_BACK_MOTOR,
                    {
                        .id           = Z_BACK_MOTOR,
                        .hwProperties = stepper_hw_properties_t{
                            .stepsPerRev         = 400u,
                            .screwPitchMm        = 0.5,
                            .maxVelocityMmS      = 12.0,
                            .maxAccelerationMmS2 = 999.999,
                            .encoderTopsPerRev   = 2000u,
                        },
                    },
                },
                // Conveyors
                {
                    WAFER_DRAWER_MOTOR,
                    {
                        .id           = WAFER_DRAWER_MOTOR,
                        .hwProperties = stepper_hw_properties_t{
                            .stepsPerRev         = 400u,
                            .screwPitchMm        = 50.0,
                            .maxVelocityMmS      = 100.0,
                            .maxAccelerationMmS2 = 999.999,
                            .encoderTopsPerRev   = 2000u,
                        },
                    },
                },
                {
                    MASK_DRAWER_MOTOR,
                    {
                        .id           = MASK_DRAWER_MOTOR,
                        .hwProperties = stepper_hw_properties_t{
                            .stepsPerRev         = 400u,
                            .screwPitchMm        = 50.0,
                            .maxVelocityMmS      = 100.0,
                            .maxAccelerationMmS2 = 999.999,
                            .encoderTopsPerRev   = 2000u,
                        },
                    },
                },
            },
            .cameras = {
                {
                    UPPER_LEFT_CAMERA,
                    {
                        .id                = UPPER_LEFT_CAMERA,
                        .serialNumber      = "DUMMY",
                        .maxExposureUs     = 5e4,
                        .defaultExposureUs = 1e4,
                        .maxGainDb         = 24.0,
                        .defaultGainDb     = 0.0,
                        .framerate         = 30.0,
                        .associatedFocalId = LEFT_CAMERA_FOCAL,
                        .associatedLightId = LEFT_CAMERA_LIGHT,
                    },
                },
                {
                    UPPER_RIGHT_CAMERA,
                    {
                        .id                = UPPER_RIGHT_CAMERA,
                        .serialNumber      = "DUMMY",
                        .maxExposureUs     = 5e4,
                        .defaultExposureUs = 1e4,
                        .maxGainDb         = 24.0,
                        .defaultGainDb     = 0.0,
                        .framerate         = 30.0,
                        .associatedFocalId = RIGHT_CAMERA_FOCAL,
                        .associatedLightId = RIGHT_CAMERA_LIGHT,
                    },
                },
            },
            .adc_to_gf_factors = {
                {FORCE_LEFT_ADC, 1.0},
                {FORCE_RIGHT_ADC, 1.0},
                {FORCE_BACK_ADC, 1.0},
            },
        };

        currentConfig.mergeMissingFieldsFrom(defaultConfig, "hardware", changes);

        for (const auto &change : changes)
        {
            qInfo().noquote() << "\t[FILLED]" << QString::fromStdString(change);
        }

        if (!changes.empty())
        {
            ConfigSaver::saveHardwareConfig(currentConfig, path);
            filledCount += changes.size();
        }
    }

    // =========================================================
    // PROCESS CONFIGURATION FILL-UP / GENERATION
    // =========================================================
    void ConfigGenerator::generateModel8Process(const std::string &path, uint32_t &filledCount)
    {
        std::vector<std::string> changes;
        process_config_t currentConfig       = ConfigLoader::loadProcessConfig(path, false, &changes);
        const process_config_t defaultConfig = process_config_t{
            .kinematic_profiles = {
                // Conveyors
                {
                    MASK_DRAWER_MOTOR,
                    {
                        {"normal", buildStepperKinematicProfile("normal", 50.0, 50.0, 8)},
                        {"fine", buildStepperKinematicProfile("fine", 5.0, 5.0, 16)},
                        {"contact", buildStepperKinematicProfile("contact", 1.0, 1.0, 16)},
                    },
                },
                {
                    WAFER_DRAWER_MOTOR,
                    {
                        {"normal", buildStepperKinematicProfile("normal", 50.0, 50.0, 8)},
                        {"fine", buildStepperKinematicProfile("fine", 5.0, 5.0, 16)},
                    },
                },
                // Alignment stages
                {
                    X_STAGE_MOTOR,
                    {
                        {"normal", buildStepperKinematicProfile("normal", 4.5, 4.0, 16)},
                        {"fine", buildStepperKinematicProfile("fine", 0.25, 1.0, 16)},
                    },
                },
                {
                    Y_STAGE_MOTOR,
                    {
                        {"normal", buildStepperKinematicProfile("normal", 4.5, 4.0, 16)},
                        {"fine", buildStepperKinematicProfile("fine", 0.25, 1.0, 16)},
                    },
                },
                {
                    THETA_STAGE_MOTOR,
                    {
                        {"normal", buildStepperKinematicProfile("normal", 3.0, 2.0, 16)},
                        {"fine", buildStepperKinematicProfile("fine", 0.25, 1.0, 16)},
                    },
                },
                // Z Elevators
                {
                    Z_LEFT_MOTOR,
                    {
                        {"normal", buildStepperKinematicProfile("normal", 1.0, 1.0, 16)},
                        {"fine", buildStepperKinematicProfile("fine", 0.1, 100.0, 16)},
                    },
                },
                {
                    Z_RIGHT_MOTOR,
                    {
                        {"normal", buildStepperKinematicProfile("normal", 1.0, 1.0, 16)},
                        {"fine", buildStepperKinematicProfile("fine", 0.1, 100.0, 16)},
                    },
                },
                {
                    Z_BACK_MOTOR,
                    {
                        {"normal", buildStepperKinematicProfile("normal", 1.0, 1.0, 16)},
                        {"fine", buildStepperKinematicProfile("fine", 0.1, 100.0, 16)},
                    },
                },
                // Cameras
                {
                    LEFT_CAMERA_X_MOTOR,
                    {
                        {"normal", buildStepperKinematicProfile("normal", 6.0, 6.0, 16)},
                        {"fine", buildStepperKinematicProfile("fine", 1.0, 400.0, 8)},
                    },
                },
                {
                    LEFT_CAMERA_Y_MOTOR,
                    {
                        {"normal", buildStepperKinematicProfile("normal", 6.0, 6.0, 16)},
                        {"fine", buildStepperKinematicProfile("fine", 1.0, 400.0, 8)},
                    },
                },
                {
                    RIGHT_CAMERA_X_MOTOR,
                    {
                        {"normal", buildStepperKinematicProfile("normal", 6.0, 6.0, 16)},
                        {"fine", buildStepperKinematicProfile("fine", 1.0, 400.0, 8)},
                    },
                },
                {
                    RIGHT_CAMERA_Y_MOTOR,
                    {
                        {"normal", buildStepperKinematicProfile("normal", 6.0, 6.0, 16)},
                        {"fine", buildStepperKinematicProfile("fine", 1.0, 400.0, 8)},
                    },
                },
                // Cameras' deck
                {
                    DECK_MOTOR,
                    {
                        {
                            "normal",
                            kinematic_profile_t{
                                .id                = "normal",
                                .targetVelocityMmS = 50.0,
                                .accelerationMmS2  = 50.0,
                                .params            = std::monostate{},
                            },
                        },
                    },
                },
            },
        };

        currentConfig.mergeMissingFieldsFrom(defaultConfig, "process", changes);

        for (const auto &change : changes)
        {
            qInfo().noquote() << "\t[FILLED]" << QString::fromStdString(change);
        }

        if (!changes.empty())
        {
            ConfigSaver::saveProcessConfig(currentConfig, path);
            filledCount += changes.size();
        }
    }

    // =========================================================
    // HELPER FUNCTIONS
    // =========================================================
    kinematic_profile_t ConfigGenerator::buildStepperKinematicProfile(
        const std::string &profileId, double targetVelMmS, double accelMmS2, uint8_t stepFraction)
    {
        return kinematic_profile_t{
            .id                = profileId,
            .targetVelocityMmS = targetVelMmS,
            .accelerationMmS2  = accelMmS2,
            .params            = stepper_kinematics_params_t{.stepFraction = stepFraction},
        };
    }

} // namespace Kub3::Config
