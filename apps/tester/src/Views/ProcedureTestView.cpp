#include "ui_ProcedureTestView.h"

#include <QCheckBox>

#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/virtual_labels.h>
#include <Views/Components/KeyboardFilter.h>
#include <Views/ProcedureTestView.h>
#include <utils.h>

#define SUCCESS_QLABEL_STYLESHEET "QLabel{ background: green; color : white; font-weight: bold; padding: 2px; }"
#define FAILURE_QLABEL_STYLESHEET "QLabel{ background: red; color : white; font-weight: bold; padding: 2px; }"

namespace Kub3::Tools::Tester
{
    static constexpr std::pair<std::string_view, std::string_view> _motorToEncoderMap[] = {
        {Z_LEFT_MOTOR, Z_LEFT_ENCODER},
        {Z_RIGHT_MOTOR, Z_RIGHT_ENCODER},
        {Z_BACK_MOTOR, Z_BACK_ENCODER},
        {X_STAGE_MOTOR, X_STAGE_ENCODER},
        {Y_STAGE_MOTOR, Y_STAGE_ENCODER},
        {THETA_STAGE_MOTOR, THETA_STAGE_ENCODER},
    };

    static constexpr Utils::ConstexprHashMap<std::size(_motorToEncoderMap)> MOTOR_TO_ENCODER_MAP(_motorToEncoderMap);

    ProcedureTestView::ProcedureTestView(Shared<ProcedureTestViewModel> viewModel,
                                         const Config::hardware_config_t &hwConf,
                                         QWidget *parent) :
        UI::Views::ViewBase(viewModel, parent),
        UI::Views::PadReceiverViewTrait(this),
        ui(std::make_unique<Ui::ProcedureTestView>()),
        m_procedureViewModel(std::move(viewModel))
    {
        ui->setupUi(this);

        loadConfigValues(hwConf);
        initializeSensorMaps();
        setupButtonBindings();
        setupViewModelBindings();
        setupPadInputsCallbacks();
        setupRealtimeCurves();
    }

    ProcedureTestView::~ProcedureTestView() = default;

    void ProcedureTestView::initializeSensorMaps()
    {
        // Encoders (Integers)
        m_intSensorsMap = {
            {QStringLiteral(WAFER_ENCODER), ui->waferPosValue},
            {QStringLiteral(MASK_ENCODER), ui->maskPosValue},
            {QStringLiteral(X_STAGE_ENCODER), ui->xStagePosValue},
            {QStringLiteral(Y_STAGE_ENCODER), ui->yStagePosValue},
            {QStringLiteral(THETA_STAGE_ENCODER), ui->thetaStagePosValue},
            {QStringLiteral(LEFT_CAMERA_X_ENCODER), ui->lblLeftCamXPos},
            {QStringLiteral(LEFT_CAMERA_Y_ENCODER), ui->lblLeftCamYPos},
            {QStringLiteral(RIGHT_CAMERA_X_ENCODER), ui->lblRightCamXPos},
            {QStringLiteral(RIGHT_CAMERA_Y_ENCODER), ui->lblRightCamYPos},
            {QStringLiteral(Z_LEFT_ENCODER), ui->zLeftPos},
            {QStringLiteral(Z_RIGHT_ENCODER), ui->zRightPos},
            {QStringLiteral(Z_BACK_ENCODER), ui->zBackPos},
        };

        // Force sensors (Integers)
        m_uint16SensorsMap = {
            {QStringLiteral(FORCE_LEFT_ADC), ui->lblLeftForce},
            {QStringLiteral(FORCE_BACK_ADC), ui->lblBackForce},
            {QStringLiteral(FORCE_RIGHT_ADC), ui->lblRightForce},
        };

        // Limits and Statuses (Booleans)
        m_boolSensorsMap = {
            // Wafer/Mask Limits
            {QStringLiteral(CW0), ui->cw0Value},
            {QStringLiteral(CW1), ui->cw1Value},
            {QStringLiteral(CW2), ui->cw2Value},
            {QStringLiteral(CM0), ui->cm0Value},
            {QStringLiteral(CM1), ui->cm1Value},
            {QStringLiteral(CM2), ui->cm2Value},
            {QStringLiteral(CM3), ui->cm3Value},
            // Stages
            {QStringLiteral(X_STAGE_LEFT_LIMIT), ui->xLeftLimitValue},
            {QStringLiteral(X_STAGE_RIGHT_LIMIT), ui->xRightLimitValue},
            {QStringLiteral(Y_STAGE_FRONT_LIMIT), ui->yFrontLimitValue},
            {QStringLiteral(Y_STAGE_BACK_LIMIT), ui->yBackLimitValue},
            {QStringLiteral(THETA_STAGE_CLOCKWISE_LIMIT), ui->thetaCWLimitValue},
            {QStringLiteral(THETA_STAGE_ANTI_CLOCKWISE_LIMIT), ui->thetaACWLimitValue},
            // Z-Axes (3Z)
            {QStringLiteral(Z_LEFT_HIGH_LIMIT), ui->zLeftHighLimit},
            {QStringLiteral(Z_RIGHT_HIGH_LIMIT), ui->zRightHighLimit},
            {QStringLiteral(Z_BACK_HIGH_LIMIT), ui->zBackHighLimit},
            {QStringLiteral(Z_LEFT_LOW_LIMIT), ui->zLeftLowLimit},
            {QStringLiteral(Z_RIGHT_LOW_LIMIT), ui->zRightLowLimit},
            {QStringLiteral(Z_BACK_LOW_LIMIT), ui->zBackLowLimit},
            {QStringLiteral(Z1), ui->lblZ1Limit},
            {QStringLiteral(WAFER_ON), ui->lblWaferOnLimit},
            {QStringLiteral(Z2), ui->lblZ2Limit},
            // Cameras
            {QStringLiteral(LEFT_CAMERA_X_LEFT_LIMIT), ui->lblLeftCamXLeftLimit},
            {QStringLiteral(LEFT_CAMERA_X_RIGHT_LIMIT), ui->lblLeftCamXRightLimit},
            {QStringLiteral(LEFT_CAMERA_Y_FRONT_LIMIT), ui->lblLeftCamYFrontLimit},
            {QStringLiteral(LEFT_CAMERA_Y_BACK_LIMIT), ui->lblLeftCamYBackLimit},
            {QStringLiteral(RIGHT_CAMERA_X_LEFT_LIMIT), ui->lblRightCamXLeftLimit},
            {QStringLiteral(RIGHT_CAMERA_X_RIGHT_LIMIT), ui->lblRightCamXRightLimit},
            {QStringLiteral(RIGHT_CAMERA_Y_FRONT_LIMIT), ui->lblRightCamYFrontLimit},
            {QStringLiteral(RIGHT_CAMERA_Y_BACK_LIMIT), ui->lblRightCamYBackLimit}};
    }

    void ProcedureTestView::setupButtonBindings()
    {
        if (!m_procedureViewModel)
            return;

        // --- Mask Operations ---
        connect(ui->btnEjectMask, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestDrawerOperation(DrawerTarget::Mask, true); });
        connect(ui->btnInsertMask, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestDrawerOperation(DrawerTarget::Mask, false); });
        connect(ui->btnExposureMask, &QPushButton::clicked, this, [this]() { /* TODO */ });
        connect(ui->btnInitializeMask, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestInitDrawer(DrawerTarget::Mask); });
        connect(ui->btnHomeMask, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestHomeDrawer(DrawerTarget::Mask); });

        // --- Wafer Operations ---
        connect(ui->btnEjectWafer, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestDrawerOperation(DrawerTarget::Wafer, true); });
        connect(ui->btnInsertWafer, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestDrawerOperation(DrawerTarget::Wafer, false); });
        connect(ui->btnInitializeWafer, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestInitDrawer(DrawerTarget::Wafer); });
        connect(ui->btnHomeWafer, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestHomeDrawer(DrawerTarget::Wafer); });

        // --- Alignment Stages Operations ---
        connect(ui->btnInitXStage, &QPushButton::clicked, this, []() {});
        connect(ui->btnCenterXStage, &QPushButton::clicked, this, []() {});
        connect(ui->btnInitYStage, &QPushButton::clicked, this, []() {});
        connect(ui->btnCenterYStage, &QPushButton::clicked, this, []() {});
        connect(ui->btnInitThetaStage, &QPushButton::clicked, this, []() {});
        connect(ui->btnCenterThetaStage, &QPushButton::clicked, this, []() {});
        connect(ui->btnInitAllStages, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestInitStages(); });
        connect(ui->btnCenterAllStages, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestCenterStages(); });

        // --- Z Elevator & Contact ---
        connect(ui->btnInitAllZAxes, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestInitZAxes(); });
        connect(ui->btnHomeAllZAxes, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestHomeZAxes(); });
        connect(ui->btnStartAutolevel, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestAutolevel(); });
        connect(ui->btnTareAll, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestForceSensorTare(ForceSensor::All); });
        connect(ui->btnTareLeft, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestForceSensorTare(ForceSensor::Left); });
        connect(ui->btnTareRight, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestForceSensorTare(ForceSensor::Right); });
        connect(ui->btnTareBack, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestForceSensorTare(ForceSensor::Back); });
        connect(ui->cbEnableForceSensors, &QCheckBox::toggled, this, [this](bool c) { m_procedureViewModel->uiRequestForceSensorsToggle(c); });

        // --- Vision (Cameras + Deck) ---
        connect(ui->btnInitCameras, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestInitCameras(); });
        connect(ui->btnInitDeck, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestInitDeck(); });
        connect(ui->btnInitAllVision, &QPushButton::clicked, this, [this]() { /* TODO */ });
    }

    void ProcedureTestView::setupViewModelBindings()
    {
        if (!m_procedureViewModel)
            return;

        connect(m_procedureViewModel.get(), &ProcedureTestViewModel::s_booleanSensorUpdate, this, &ProcedureTestView::booleanSensorUpdate);
        connect(m_procedureViewModel.get(), &ProcedureTestViewModel::s_integerSensorUpdate, this, &ProcedureTestView::integerSensorUpdate);
        connect(m_procedureViewModel.get(), &ProcedureTestViewModel::s_uint16SensorUpdate, this, &ProcedureTestView::uint16SensorUpdate);

        connect(m_procedureViewModel.get(), &ProcedureTestViewModel::s_statusMessageChanged, this, [this]() {
            ui->lblProcStatus->setText(m_procedureViewModel->lastStatusMessage());
            ui->lblProcStatus->setStyleSheet(m_procedureViewModel->hasError() ? "color: red; font-weight: bold;" : "color: #2ecc71; font-weight: bold;");
        });

        connect(m_procedureViewModel.get(), &ProcedureTestViewModel::s_isRunningChanged, this, [this]() {
            const bool idle = !m_procedureViewModel->isRunning();

            ui->btnEjectMask->setEnabled(idle);
            ui->btnInsertMask->setEnabled(idle);
            // ui->btnExposureMask->setEnabled(idle);
            ui->btnHomeMask->setEnabled(idle);
            ui->btnInitializeMask->setEnabled(idle);

            ui->btnEjectWafer->setEnabled(idle);
            ui->btnInsertWafer->setEnabled(idle);
            ui->btnHomeWafer->setEnabled(idle);
            ui->btnInitializeWafer->setEnabled(idle);

            ui->btnTareAll->setEnabled(idle);
            ui->btnTareLeft->setEnabled(idle);
            ui->btnTareRight->setEnabled(idle);
            ui->btnTareBack->setEnabled(idle);

            // ui->btnCenterXStage->setEnabled(idle);
            // ui->btnCenterYStage->setEnabled(idle);
            // ui->btnCenterThetaStage->setEnabled(idle);
            ui->btnCenterAllStages->setEnabled(idle);
            // ui->btnInitXStage->setEnabled(idle);
            // ui->btnInitYStage->setEnabled(idle);
            // ui->btnInitThetaStage->setEnabled(idle);
            ui->btnInitAllStages->setEnabled(idle);
        });
    }

    void ProcedureTestView::setupRealtimeCurves()
    {
        auto configureForcePlot = [&](RealTimeCurveWidget *plt, const QString &id, QColor clr) {
            plt->setUnit("gF");
            plt->configureCurve(id, "Force", clr);
            plt->setHistory(20000); // 20 seconds window
        };

        configureForcePlot(ui->pltLeftForce, "measured", QColor("#0072B2"));
        configureForcePlot(ui->pltBackForce, "measured", QColor("#D55E00"));
        configureForcePlot(ui->pltRightForce, "measured", QColor("#CC79A7"));
    }

    void ProcedureTestView::setupPadInputsCallbacks()
    {
        // Helper to map PadTrigger states directly to your MovementKind enum
        auto toMovementKind = [](UI::Views::PadTrigger trigger) -> MovementKind {
            switch (trigger)
            {
            case UI::Views::PadTrigger::Pressed:
                return MovementKind::GRANULAR;
            case UI::Views::PadTrigger::Held:
                return MovementKind::CONTINUOUS;
            case UI::Views::PadTrigger::Released:
                return MovementKind::STOP;
            }
            return MovementKind::STOP;
        };
        // High-order helper to generate camera movement actions
        auto bindCamera = [this, toMovementKind](CameraId camId, CameraDirection dir) {
            return [this, toMovementKind, camId, dir](UI::Views::PadTrigger trigger) {
                cameraMovement(camId, toMovementKind(trigger), dir);
            };
        };
        // High-order helper to generate alignment stage actions
        auto bindStage = [this, toMovementKind](AlignmentStageId stageId, AlignmentStageDirection dir) {
            return [this, toMovementKind, stageId, dir](UI::Views::PadTrigger trigger) {
                alignmentStageMovement(stageId, toMovementKind(trigger), dir);
            };
        };

        // ==========================================
        // LOGICAL PAD BINDINGS
        // ==========================================

        // Cameras (Left Camera)
        link(UI::Views::PadTarget::LeftCamera, UI::Views::PadAction::Up, bindCamera(CameraId::LEFT, CameraDirection::UP));
        link(UI::Views::PadTarget::LeftCamera, UI::Views::PadAction::Left, bindCamera(CameraId::LEFT, CameraDirection::LEFT));
        link(UI::Views::PadTarget::LeftCamera, UI::Views::PadAction::Down, bindCamera(CameraId::LEFT, CameraDirection::DOWN));
        link(UI::Views::PadTarget::LeftCamera, UI::Views::PadAction::Right, bindCamera(CameraId::LEFT, CameraDirection::RIGHT));

        // X Alignment Stage
        link(UI::Views::PadTarget::XStage, UI::Views::PadAction::Left, bindStage(AlignmentStageId::X, AlignmentStageDirection::X_LEFT));
        link(UI::Views::PadTarget::XStage, UI::Views::PadAction::Right, bindStage(AlignmentStageId::X, AlignmentStageDirection::X_RIGHT));

        // Y Alignment Stage
        link(UI::Views::PadTarget::YStage, UI::Views::PadAction::Front, bindStage(AlignmentStageId::Y, AlignmentStageDirection::Y_FRONT));
        link(UI::Views::PadTarget::YStage, UI::Views::PadAction::Back, bindStage(AlignmentStageId::Y, AlignmentStageDirection::Y_BACK));

        // Theta Alignment Stage
        link(UI::Views::PadTarget::ThetaStage, UI::Views::PadAction::CW, bindStage(AlignmentStageId::THETA, AlignmentStageDirection::THETA_CW));
        link(UI::Views::PadTarget::ThetaStage, UI::Views::PadAction::CCW, bindStage(AlignmentStageId::THETA, AlignmentStageDirection::THETA_CCW));
    }

    void ProcedureTestView::loadConfigValues(const Config::hardware_config_t &conf)
    {
        for (auto motorConf : conf.motors)
        {
            if (auto encoderId = MOTOR_TO_ENCODER_MAP.get(motorConf.first.toStdString()); encoderId)
            {
                m_motorsHwConf.emplace(QString::fromStdString(std::string(encoderId.value())), motorConf.second);
            }
        }
        auto &forceFactors = conf.adc_to_gf_factors;
        m_forceSensorIdToConfMap.emplace(
            FORCE_LEFT_ADC,
            force_sensor_conf_t{
                .adcToGFRatio = (forceFactors.find(FORCE_LEFT_ADC) != forceFactors.end()) ? forceFactors.at(FORCE_LEFT_ADC) : 1,
                .tareId       = V_TARE_FORCE_LEFT_ADC});
        m_forceSensorIdToConfMap.emplace(
            FORCE_RIGHT_ADC,
            force_sensor_conf_t{
                .adcToGFRatio = (forceFactors.find(FORCE_RIGHT_ADC) != forceFactors.end()) ? forceFactors.at(FORCE_RIGHT_ADC) : 1,
                .tareId       = V_TARE_FORCE_RIGHT_ADC});
        m_forceSensorIdToConfMap.emplace(
            FORCE_BACK_ADC,
            force_sensor_conf_t{
                .adcToGFRatio = (forceFactors.find(FORCE_BACK_ADC) != forceFactors.end()) ? forceFactors.at(FORCE_BACK_ADC) : 1,
                .tareId       = V_TARE_FORCE_BACK_ADC});
    }

    void ProcedureTestView::booleanSensorUpdate(const QString &sensorId, bool value)
    {
        if (auto it = m_boolSensorsMap.find(sensorId); it != m_boolSensorsMap.end())
            updateBoolSensorsText(it->second, value);
    }

    void ProcedureTestView::integerSensorUpdate(const QString &sensorId, int32_t value)
    {
        if (auto it = m_intSensorsMap.find(sensorId); it != m_intSensorsMap.end())
        {
            QString text;
            const Config::stepper_hw_properties_t *motorConf = nullptr;

            if (auto itMotorConf = m_motorsHwConf.find(sensorId); itMotorConf != m_motorsHwConf.end())
            {
                motorConf = std::get_if<Config::stepper_hw_properties_t>(&itMotorConf->second.hwProperties);
            }

            // Try converting encoder tops value to millimeters
            if (motorConf)
            {
                text = QString::number(static_cast<double>(value) * (motorConf->screwPitchMm / motorConf->encoderTopsPerRev)) + " mm";
            }
            else
            {
                text = QString::number(value) + " (enc. tops)";
            }
            updateLabelText(it->second, text);
        }
    }

    void ProcedureTestView::uint16SensorUpdate(const QString &sensorId, uint16_t value)
    {
        if (auto it = m_uint16SensorsMap.find(sensorId); it != m_uint16SensorsMap.end())
        {
            QString text;

            // Try converting value with corresponding force factor
            if (auto itForceConf = m_forceSensorIdToConfMap.find(sensorId); itForceConf != m_forceSensorIdToConfMap.end())
            {
                const uint16_t forceTareADC = m_procedureViewModel->getADCTareValue(itForceConf->second.tareId).value_or(0);
                const double gain           = itForceConf->second.adcToGFRatio.value_or(0);
                const double forceGF        = static_cast<double>(value - forceTareADC) * gain;

                if (sensorId == FORCE_LEFT_ADC)
                    ui->pltLeftForce->addDataPoint("measured", forceGF);
                else if (sensorId == FORCE_BACK_ADC)
                    ui->pltBackForce->addDataPoint("measured", forceGF);
                else if (sensorId == FORCE_RIGHT_ADC)
                    ui->pltRightForce->addDataPoint("measured", forceGF);
                text = QString::number(forceGF) + " gF (bin.: " + QString::number(value) + ", tare bin.: " + QString::number(forceTareADC) + ")";
            }
            else
                text = QString::number(value);
            updateLabelText(it->second, text); // Update UI label using raw value
        }
    }

    void ProcedureTestView::updateBoolSensorsText(QLabel *label, const bool state)
    {
        constexpr QStringView nd = u"N.D.";

        if (label->text() == nd)
            label->setText(state ? "ON" : "OFF");
        label->setStyleSheet(state ? SUCCESS_QLABEL_STYLESHEET : FAILURE_QLABEL_STYLESHEET);
    }

    void ProcedureTestView::updateLabelText(QLabel *label, const QString &text)
    {
        label->setText(text);
    }

    void ProcedureTestView::cameraMovement(CameraId camId, MovementKind kind, CameraDirection dir)
    {
        m_procedureViewModel->uiRequestCameraMovement(camId, kind, dir);
    }

    void ProcedureTestView::alignmentStageMovement(AlignmentStageId stageId, MovementKind kind, AlignmentStageDirection dir)
    {
        m_procedureViewModel->uiRequestAlignmentStageMovement(stageId, kind, dir);
    }

} // namespace Kub3::Tools::Tester
