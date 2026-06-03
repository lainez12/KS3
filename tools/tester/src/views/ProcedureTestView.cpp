#include "ui_ProcedureTestView.h"

#include <HAL/MachineStatus/sensors_labels.h>
#include <views/ProcedureTestView.h>

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

        // Wafer conveyor position (encoder)
        m_intSensorsMap.emplace(QStringLiteral(WAFER_ENCODER), ui->waferPosValue);
        // Mask conveyor position (encoder)
        m_intSensorsMap.emplace(QStringLiteral(MASK_ENCODER), ui->maskPosValue);
        // Alignment stages position (encoder)
        m_intSensorsMap.emplace(QStringLiteral(X_STAGE_ENCODER), ui->xStagePosValue);
        m_intSensorsMap.emplace(QStringLiteral(Y_STAGE_ENCODER), ui->yStagePosValue);
        m_intSensorsMap.emplace(QStringLiteral(THETA_STAGE_ENCODER), ui->thetaStagePosValue);

        // Wafer conveyor limits
        m_boolSensorsMap.emplace(QStringLiteral(CW0), ui->cw0Value);
        m_boolSensorsMap.emplace(QStringLiteral(CW1), ui->cw1Value);
        m_boolSensorsMap.emplace(QStringLiteral(CW2), ui->cw2Value);
        // Mask conveyor limits
        m_boolSensorsMap.emplace(QStringLiteral(CM0), ui->cm0Value);
        m_boolSensorsMap.emplace(QStringLiteral(CM1), ui->cm1Value);
        m_boolSensorsMap.emplace(QStringLiteral(CM2), ui->cm2Value);
        m_boolSensorsMap.emplace(QStringLiteral(CM3), ui->cm3Value);
        // Alignment stages limits
        m_boolSensorsMap.emplace(QStringLiteral(X_STAGE_LEFT_LIMIT), ui->xLeftLimitValue);
        m_boolSensorsMap.emplace(QStringLiteral(X_STAGE_RIGHT_LIMIT), ui->xRightLimitValue);
        m_boolSensorsMap.emplace(QStringLiteral(Y_STAGE_FRONT_LIMIT), ui->yFrontLimitValue);
        m_boolSensorsMap.emplace(QStringLiteral(Y_STAGE_BACK_LIMIT), ui->yBackLimitValue);
        m_boolSensorsMap.emplace(QStringLiteral(THETA_STAGE_CLOCKWISE_LIMIT), ui->thetaCWLimitValue);
        m_boolSensorsMap.emplace(QStringLiteral(THETA_STAGE_ANTI_CLOCKWISE_LIMIT), ui->thetaACWLimitValue);

        bindViewModel();
    }

    ProcedureTestView::~ProcedureTestView() = default;

    void ProcedureTestView::bindViewModel()
    {
        if (!m_procedureViewModel)
            return;

        // ===============================
        // VIEW --> VIEW MODEL (Intents)
        // ===============================

        // --- Mask Operations ---
        connect(ui->btnEjectMask, &QPushButton::clicked, this, [this]() {
            m_procedureViewModel->uiRequestDrawerOperation(DrawerTarget::Mask, true);
        });
        connect(ui->btnInsertMask, &QPushButton::clicked, this, [this]() {
            m_procedureViewModel->uiRequestDrawerOperation(DrawerTarget::Mask, false);
        });
        connect(ui->btnExposureMask, &QPushButton::clicked, this, [this]() {
            // TODO: m_procedureViewModel->uiRequestStowage(...);
        });
        connect(ui->btnInitializeMask, &QPushButton::clicked, this, [this]() {
            // TODO: m_procedureViewModel->uiRequestInitialization(...);
        });
        connect(ui->btnHomeMask, &QPushButton::clicked, this, [this]() {
            // TODO: m_procedureViewModel->uiRequestHoming(...);
        });
        // --- Wafer Operations ---
        connect(ui->btnEjectWafer, &QPushButton::clicked, this, [this]() {
            m_procedureViewModel->uiRequestDrawerOperation(DrawerTarget::Wafer, true);
        });
        connect(ui->btnInsertWafer, &QPushButton::clicked, this, [this]() {
            m_procedureViewModel->uiRequestDrawerOperation(DrawerTarget::Wafer, false);
        });
        connect(ui->btnInitializeWafer, &QPushButton::clicked, this, [this]() {
            // TODO: m_procedureViewModel->uiRequestInitialization(...);
        });
        connect(ui->btnHomeWafer, &QPushButton::clicked, this, [this]() {
            // TODO: m_procedureViewModel->uiRequestHoming(...);
        });
        // --- Alignment Stages Operations ---
        connect(ui->btnInitXStage, &QPushButton::clicked, this, [this]() {});
        connect(ui->btnCenterXStage, &QPushButton::clicked, this, [this]() {});
        connect(ui->btnInitYStage, &QPushButton::clicked, this, [this]() {});
        connect(ui->btnCenterYStage, &QPushButton::clicked, this, [this]() {});
        connect(ui->btnInitThetaStage, &QPushButton::clicked, this, [this]() {});
        connect(ui->btnCenterThetaStage, &QPushButton::clicked, this, [this]() {});
        connect(ui->btnInitAllStages, &QPushButton::clicked, this, [this]() {});
        connect(ui->btnCenterAllStages, &QPushButton::clicked, this, [this]() {});

        // =====================================
        // VIEW MODEL --> VIEW (Reactivity)
        // =====================================

        // Update Sensor UI
        connect(m_procedureViewModel.get(), &ProcedureTestViewModel::s_booleanSensorUpdate,
                this, &ProcedureTestView::ps_booleanSensorUpdate);
        connect(m_procedureViewModel.get(), &ProcedureTestViewModel::s_integerSensorUpdate,
                this, &ProcedureTestView::ps_integerSensorUpdate);

        // Update Status Message
        connect(m_procedureViewModel.get(), &ProcedureTestViewModel::s_statusMessageChanged, this, [this]() {
            ui->lblProcStatus->setText(m_procedureViewModel->lastStatusMessage());
            if (m_procedureViewModel->hasError())
            {
                ui->lblProcStatus->setStyleSheet("color: red; font-weight: bold;");
            }
            else
            {
                ui->lblProcStatus->setStyleSheet("color: #2ecc71; font-weight: bold;"); // Green
            }
        });

        // Toggle UI interactions based on running state
        connect(m_procedureViewModel.get(), &ProcedureTestViewModel::s_isRunningChanged, this, [this]() {
            const bool isRunning = m_procedureViewModel->isRunning();

            // Disable all action buttons while a procedure is running
            // --- Mask
            ui->btnEjectMask->setEnabled(!isRunning);
            ui->btnInsertMask->setEnabled(!isRunning);
            ui->btnExposureMask->setEnabled(!isRunning);
            ui->btnHomeMask->setEnabled(!isRunning);
            ui->btnInitializeMask->setEnabled(!isRunning);
            // --- Wafer
            ui->btnEjectWafer->setEnabled(!isRunning);
            ui->btnInsertWafer->setEnabled(!isRunning);
            ui->btnHomeWafer->setEnabled(!isRunning);
            ui->btnInitializeWafer->setEnabled(!isRunning);
            // --- Alignment stages
            ui->btnCenterXStage->setEnabled(!isRunning);
            ui->btnCenterYStage->setEnabled(!isRunning);
            ui->btnCenterThetaStage->setEnabled(!isRunning);
            ui->btnCenterAllStages->setEnabled(!isRunning);
            ui->btnInitXStage->setEnabled(!isRunning);
            ui->btnInitYStage->setEnabled(!isRunning);
            ui->btnInitThetaStage->setEnabled(!isRunning);
            ui->btnInitAllStages->setEnabled(!isRunning);
        });
    }

    void ProcedureTestView::ps_booleanSensorUpdate(const QString &sensorId, bool value)
    {
        if (auto it = m_boolSensorsMap.find(sensorId); it != m_boolSensorsMap.end())
            this->updateBoolSensorsText(it->second, value);
    }

    void ProcedureTestView::ps_integerSensorUpdate(const QString &sensorId, int32_t value)
    {
        if (auto it = m_intSensorsMap.find(sensorId); it != m_intSensorsMap.end())
            this->updateIntSensorsText(it->second, value);
    }

    void ProcedureTestView::updateBoolSensorsText(QLabel *label, const bool state)
    {
        label->setText(state ? "ON" : "OFF");
        label->setStyleSheet(state ? SUCCESS_QLABEL_STYLESHEET : FAILURE_QLABEL_STYLESHEET);
    }

    void ProcedureTestView::updateIntSensorsText(QLabel *label, const int32_t value)
    {
        label->setText(QString::number(value));
    }

} // namespace Kub3::Tools::Tester
