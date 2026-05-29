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
        // Wafer conveyor limits
        m_boolSensorsMap.emplace(QStringLiteral(CW0), ui->cw0Value);
        m_boolSensorsMap.emplace(QStringLiteral(CW1), ui->cw1Value);
        m_boolSensorsMap.emplace(QStringLiteral(CW2), ui->cw2Value);
        // Mask conveyor limits
        m_boolSensorsMap.emplace(QStringLiteral(CM0), ui->cm0Value);
        m_boolSensorsMap.emplace(QStringLiteral(CM1), ui->cm1Value);
        m_boolSensorsMap.emplace(QStringLiteral(CM2), ui->cm2Value);
        m_boolSensorsMap.emplace(QStringLiteral(CM3), ui->cm3Value);

        bindViewModel();
    }

    ProcedureTestView::~ProcedureTestView() = default;

    void ProcedureTestView::bindViewModel()
    {
        if (!m_procedureViewModel)
            return;

        // ============================
        // VIEW --> VIEW MODEL (Intents)
        // ============================

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
            ui->btnEjectMask->setEnabled(!isRunning);
            ui->btnInsertMask->setEnabled(!isRunning);
            ui->btnExposureMask->setEnabled(!isRunning);
            ui->btnHomeMask->setEnabled(!isRunning);
            ui->btnInitializeMask->setEnabled(!isRunning);

            ui->btnEjectWafer->setEnabled(!isRunning);
            ui->btnInsertWafer->setEnabled(!isRunning);
            ui->btnHomeWafer->setEnabled(!isRunning);
            ui->btnInitializeWafer->setEnabled(!isRunning);
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
