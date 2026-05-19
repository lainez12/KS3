#include "ui_MotorTestWindow.h" // Generated from .ui

#include <MotorTestViewModel.h>
#include <MotorTestWindow.h>

namespace Kub3::Tools::MotorTester
{

    MotorTestWindow::MotorTestWindow(std::shared_ptr<MotorTestViewModel> viewModel, QWidget *parent) :
        QMainWindow(parent),
        ui(std::make_unique<Ui::MotorTestWindow>()),
        m_viewModel(std::move(viewModel))
    {
        ui->setupUi(this);

        // Populate step fraction combo box
        ui->cbStepFraction->addItem("1 (Full Step)", 1);
        ui->cbStepFraction->addItem("2 (Half Step)", 2);
        ui->cbStepFraction->addItem("4", 4);
        ui->cbStepFraction->addItem("8", 8);
        ui->cbStepFraction->addItem("16", 16);
        ui->cbStepFraction->addItem("32", 32);
        // Default to 8
        ui->cbStepFraction->setCurrentIndex(3);

        // Hardware Selection
        connect(ui->cbMotorSelect, QOverload<int>::of(&QComboBox::currentIndexChanged),
                m_viewModel.get(), &MotorTestViewModel::uiSelectMotor);

        // Jogging Buttons (Press & Hold)
        connect(ui->btnJogPos, &QPushButton::pressed, this, [this]() { invokeJog(0); });
        connect(ui->btnJogPos, &QPushButton::released, m_viewModel.get(), &MotorTestViewModel::uiJogRelease);
        connect(ui->btnJogNeg, &QPushButton::pressed, this, [this]() { invokeJog(1); });
        connect(ui->btnJogNeg, &QPushButton::released, m_viewModel.get(), &MotorTestViewModel::uiJogRelease);

        // Absolute Move Button
        connect(ui->btnGoAbs, &QPushButton::clicked, this, &MotorTestWindow::invokeAbsoluteMove);

        // Emergency Stop
        connect(ui->btnEmergencyStop, &QPushButton::clicked, m_viewModel.get(), &MotorTestViewModel::uiEmergencyStop);

        bindViewModel();
    }

    MotorTestWindow::~MotorTestWindow() = default;

    void MotorTestWindow::invokeJog(int direction)
    {
        // Extract parameters from UI
        double v = ui->spinVelocity->value();
        double a = ui->spinAcceleration->value();
        int sf   = ui->cbStepFraction->currentData().toInt();

        m_viewModel->uiJogHold(direction, v, a, sf);
    }

    void MotorTestWindow::invokeAbsoluteMove()
    {
        double target = ui->spinTargetPos->value();
        double v      = ui->spinVelocity->value();
        double a      = ui->spinAcceleration->value();
        int sf        = ui->cbStepFraction->currentData().toInt();

        m_viewModel->uiMoveToAbsolute(target, v, a, sf);
    }

    void MotorTestWindow::bindViewModel()
    {
        // Populate Dropdown
        connect(m_viewModel.get(), &MotorTestViewModel::s_availableMotorsLoaded, this, [this](const QStringList &motors) {
            ui->cbMotorSelect->blockSignals(true);
            ui->cbMotorSelect->clear();
            ui->cbMotorSelect->addItems(motors);
            ui->cbMotorSelect->blockSignals(false);

            if (!motors.isEmpty()) // Trigger selection of first motor if available
                ui->cbMotorSelect->setCurrentIndex(0);
        });

        // Dynamic UI Toggles (Enable StepFraction only for Steppers)
        connect(m_viewModel.get(), &MotorTestViewModel::s_isStepperChanged, this, [this]() {
            ui->grpStepperSettings->setEnabled(m_viewModel->isStepper());
        });

        // Safeties
        connect(m_viewModel.get(), &MotorTestViewModel::s_hasValidMotorChanged, this, [this]() {
            bool valid = m_viewModel->hasValidMotor();
            ui->btnJogPos->setEnabled(valid);
            ui->btnJogNeg->setEnabled(valid);
            ui->btnGoAbs->setEnabled(valid);
        });

        // Real-Time Telemetry updates (50Hz)
        connect(m_viewModel.get(), &MotorTestViewModel::s_telemetryChanged, this, [this]() {
            ui->lblPos->setText(QString::number(m_viewModel->position(), 'f', 4) + " mm");
            ui->lblSpd->setText(QString::number(m_viewModel->speed(), 'f', 2) + " mm/s");
            ui->lblAcc->setText(QString::number(m_viewModel->acceleration(), 'f', 2) + " mm/s²");
        });
    }

} // namespace Kub3::Tools::MotorTester
