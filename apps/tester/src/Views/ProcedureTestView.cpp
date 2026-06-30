#include "ui_ProcedureTestView.h"

#include <HAL/MachineStatus/sensors_labels.h>
#include <Views/Components/KeyboardFilter.h>
#include <Views/ProcedureTestView.h>

#define SUCCESS_QLABEL_STYLESHEET "QLabel{ background: green; color : white; font-weight: bold; padding: 2px; }"
#define FAILURE_QLABEL_STYLESHEET "QLabel{ background: red; color : white; font-weight: bold; padding: 2px; }"

namespace Kub3::Tools::Tester
{

    ProcedureTestView::ProcedureTestView(Shared<ProcedureTestViewModel> viewModel, QWidget *parent) :
        UI::Views::ViewBase(viewModel, parent),
        ui(std::make_unique<Ui::ProcedureTestView>()),
        m_procedureViewModel(std::move(viewModel))
    {
        ui->setupUi(this);
        this->setFocusPolicy(Qt::StrongFocus);

        initializeSensorMaps();
        setupButtonBindings();
        setupViewModelBindings();

        // Setup Application-wide Keyboard Filter mapped to this view
        auto *keyboardFilter = new UI::Views::KeyboardFilter(this, this);
        connect(keyboardFilter, &UI::Views::KeyboardFilter::keyPressed, this, &ProcedureTestView::handleKeyPressed);
        connect(keyboardFilter, &UI::Views::KeyboardFilter::keyHeld, this, &ProcedureTestView::handleKeyHeld);
        connect(keyboardFilter, &UI::Views::KeyboardFilter::keyReleased, this, &ProcedureTestView::handleKeyReleased);
    }

    ProcedureTestView::~ProcedureTestView() = default;

    void ProcedureTestView::showEvent(QShowEvent *event)
    {
        UI::Views::ViewBase::showEvent(event);
        this->setFocus(Qt::OtherFocusReason);
    }

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
            {QStringLiteral(RIGHT_CAMERA_Y_ENCODER), ui->lblRightCamYPos}};

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
        connect(ui->btnInitializeMask, &QPushButton::clicked, this, [this]() { /* TODO */ });
        connect(ui->btnHomeMask, &QPushButton::clicked, this, [this]() { /* TODO */ });

        // --- Wafer Operations ---
        connect(ui->btnEjectWafer, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestDrawerOperation(DrawerTarget::Wafer, true); });
        connect(ui->btnInsertWafer, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestDrawerOperation(DrawerTarget::Wafer, false); });
        connect(ui->btnInitializeWafer, &QPushButton::clicked, this, [this]() { /* TODO */ });
        connect(ui->btnHomeWafer, &QPushButton::clicked, this, [this]() { /* TODO */ });

        // --- Alignment Stages Operations ---
        connect(ui->btnInitXStage, &QPushButton::clicked, this, []() {});
        connect(ui->btnCenterXStage, &QPushButton::clicked, this, []() {});
        connect(ui->btnInitYStage, &QPushButton::clicked, this, []() {});
        connect(ui->btnCenterYStage, &QPushButton::clicked, this, []() {});
        connect(ui->btnInitThetaStage, &QPushButton::clicked, this, []() {});
        connect(ui->btnCenterThetaStage, &QPushButton::clicked, this, []() {});
        connect(ui->btnInitAllStages, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestInitStages(); });
        connect(ui->btnCenterAllStages, &QPushButton::clicked, this, []() {});

        // --- Z Elevator & Deck ---
        connect(ui->btnStartAutolevel, &QPushButton::clicked, this, [this]() { m_procedureViewModel->uiRequestAutolevel(); });
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

        connect(m_procedureViewModel.get(), &ProcedureTestViewModel::s_statusMessageChanged, this, [this]() {
            ui->lblProcStatus->setText(m_procedureViewModel->lastStatusMessage());
            ui->lblProcStatus->setStyleSheet(m_procedureViewModel->hasError() ? "color: red; font-weight: bold;" : "color: #2ecc71; font-weight: bold;");
        });

        connect(m_procedureViewModel.get(), &ProcedureTestViewModel::s_isRunningChanged, this, [this]() {
            const bool idle = !m_procedureViewModel->isRunning();

            ui->btnEjectMask->setEnabled(idle);
            ui->btnInsertMask->setEnabled(idle);
            ui->btnExposureMask->setEnabled(idle);
            ui->btnHomeMask->setEnabled(idle);
            ui->btnInitializeMask->setEnabled(idle);

            ui->btnEjectWafer->setEnabled(idle);
            ui->btnInsertWafer->setEnabled(idle);
            ui->btnHomeWafer->setEnabled(idle);
            ui->btnInitializeWafer->setEnabled(idle);

            ui->btnCenterXStage->setEnabled(idle);
            ui->btnCenterYStage->setEnabled(idle);
            ui->btnCenterThetaStage->setEnabled(idle);
            ui->btnCenterAllStages->setEnabled(idle);
            ui->btnInitXStage->setEnabled(idle);
            ui->btnInitYStage->setEnabled(idle);
            ui->btnInitThetaStage->setEnabled(idle);
            ui->btnInitAllStages->setEnabled(idle);
        });
    }

    void ProcedureTestView::booleanSensorUpdate(const QString &sensorId, bool value)
    {
        if (auto it = m_boolSensorsMap.find(sensorId); it != m_boolSensorsMap.end())
            updateBoolSensorsText(it->second, value);
    }

    void ProcedureTestView::integerSensorUpdate(const QString &sensorId, int32_t value)
    {
        if (auto it = m_intSensorsMap.find(sensorId); it != m_intSensorsMap.end())
            updateIntSensorsText(it->second, value);
    }

    void ProcedureTestView::updateBoolSensorsText(QLabel *label, const bool state)
    {
        label->setStyleSheet(state ? SUCCESS_QLABEL_STYLESHEET : FAILURE_QLABEL_STYLESHEET);
    }

    void ProcedureTestView::updateIntSensorsText(QLabel *label, const int32_t value)
    {
        label->setText(QString::number(value));
    }

    void ProcedureTestView::cameraMovement(CameraId camId, CameraMovementKind kind, CameraDirection dir)
    {
        QString camera = (camId == CameraId::LEFT) ? "Left" : "Right";
        QString move   = (kind == CameraMovementKind::CONTINUOUS) ? "continuous"
                         : (kind == CameraMovementKind::GRANULAR) ? "granular"
                                                                  : "stop";

        QString direction;
        switch (dir)
        {
        case CameraDirection::DOWN:
            direction = "DOWN";
            break;
        case CameraDirection::UP:
            direction = "UP";
            break;
        case CameraDirection::LEFT:
            direction = "LEFT";
            break;
        case CameraDirection::RIGHT:
            direction = "RIGHT";
            break;
        }

        m_procedureViewModel->uiRequestCameraMovement(camId, kind, dir);
    }

    Optional<std::pair<CameraId, CameraDirection>> ProcedureTestView::mapKeyEvtToCameraCmd(Qt::Key keyCode) const
    {
        switch (keyCode)
        {
        // Left Camera
        case Qt::Key::Key_A:
            return std::make_pair(CameraId::LEFT, CameraDirection::UP);
        case Qt::Key::Key_K:
            return std::make_pair(CameraId::LEFT, CameraDirection::LEFT);
        case Qt::Key::Key_B:
            return std::make_pair(CameraId::LEFT, CameraDirection::DOWN);
        case Qt::Key::Key_J:
            return std::make_pair(CameraId::LEFT, CameraDirection::RIGHT);

        // Right Camera
        case Qt::Key::Key_H:
            return std::make_pair(CameraId::RIGHT, CameraDirection::UP);
        case Qt::Key::Key_N:
            return std::make_pair(CameraId::RIGHT, CameraDirection::LEFT);
        case Qt::Key::Key_Z:
            return std::make_pair(CameraId::RIGHT, CameraDirection::DOWN);
        case Qt::Key::Key_O:
            return std::make_pair(CameraId::RIGHT, CameraDirection::RIGHT);

        default:
            return std::nullopt;
        }
    }

    void ProcedureTestView::handleKeyPressed(Qt::Key keyCode, Qt::KeyboardModifiers modifiers)
    {
        if (auto cmd = mapKeyEvtToCameraCmd(keyCode))
        {
            cameraMovement(cmd->first, CameraMovementKind::GRANULAR, cmd->second);
        }
    }

    void ProcedureTestView::handleKeyHeld(Qt::Key keyCode, Qt::KeyboardModifiers modifiers)
    {
        if (auto cmd = mapKeyEvtToCameraCmd(keyCode))
        {
            cameraMovement(cmd->first, CameraMovementKind::CONTINUOUS, cmd->second);
        }
    }

    void ProcedureTestView::handleKeyReleased(Qt::Key keyCode, Qt::KeyboardModifiers modifiers)
    {
        if (auto cmd = mapKeyEvtToCameraCmd(keyCode))
        {
            cameraMovement(cmd->first, CameraMovementKind::STOP, cmd->second);
        }
    }

} // namespace Kub3::Tools::Tester
