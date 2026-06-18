#include <views/MotorTestView.h>

namespace Kub3::Tools::Tester
{

    MotorTestView::MotorTestView(Shared<MotorTestViewModel> vm, QWidget *parent) :
        UI::Views::ViewBase(vm, parent),
        ui(std::make_unique<Ui::MotorTestView>()),
        m_vm(std::move(vm))
    {
        ui->setupUi(this);

        // --- Configure the Plots ---
        // Position Plot: Reference (Blue) vs Real Position (Green)
        ui->plotPosition->setUnit("mm");
        // ui->plotPosition->configureCurve("ref", "Reference", QColor("#2ecc71")); // Deep Blue
        ui->plotPosition->configureCurve("real", "Value", QColor("#0055ff")); // Emerald Green
        ui->plotPosition->setHistory(30000);                                  // 30-second window
        // Speed Plot: Reference (Orange) vs Real Speed (Purple)
        ui->plotSpeed->setUnit("mm/s");
        // ui->plotSpeed->configureCurve("ref", "Reference", QColor("#9b59b6")); // Burnt Orange
        ui->plotSpeed->configureCurve("real", "Value", QColor("#e67e22")); // Purple
        ui->plotSpeed->setHistory(30000);
        // Acceleration Plot: Reference (Dark Red) vs Real Acceleration (Gray)
        ui->plotAcceleration->setUnit("mm/s²");
        // ui->plotAcceleration->configureCurve("ref", "Reference", QColor("#7f8c8d")); // Dark Red
        ui->plotAcceleration->configureCurve("real", "Value", QColor("#c0392b")); // Gray
        ui->plotAcceleration->setHistory(30000);

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
        connect(ui->cbMotorSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), m_vm.get(), &MotorTestViewModel::uiSelectMotor);
        // Jogging Buttons (Press & Hold)
        connect(ui->btnJogPos, &QPushButton::pressed, this, [this]() { invokeJog(0); });
        connect(ui->btnJogPos, &QPushButton::released, m_vm.get(), &MotorTestViewModel::uiJogRelease);
        connect(ui->btnJogNeg, &QPushButton::pressed, this, [this]() { invokeJog(1); });
        connect(ui->btnJogNeg, &QPushButton::released, m_vm.get(), &MotorTestViewModel::uiJogRelease);
        // Absolute Move Button
        connect(ui->btnGoAbs, &QPushButton::clicked, this, &MotorTestView::invokeAbsoluteMove);

        // Populate Dropdown
        connect(m_vm.get(), &MotorTestViewModel::s_availableMotorsLoaded, this, [this](const QStringList &motors) {
            ui->cbMotorSelect->blockSignals(true);
            ui->cbMotorSelect->clear();
            ui->cbMotorSelect->addItems(motors);
            ui->cbMotorSelect->blockSignals(false);
            if (!motors.isEmpty())
                ui->cbMotorSelect->setCurrentIndex(0);
        });
        // Dynamic UI Toggles (Enable StepFraction only for Steppers)
        connect(m_vm.get(), &MotorTestViewModel::s_isStepperChanged, this, [this]() {
            ui->grpStepperSettings->setEnabled(m_vm->isStepper());
        });
        // Real-Time Telemetry updates
        connect(m_vm.get(), &MotorTestViewModel::s_telemetryChanged, this, [this]() {
            ui->lblPos->setText(QString::number(m_vm->position(), 'f', 4) + " mm");
            ui->lblSpd->setText(QString::number(m_vm->speed(), 'f', 2) + " mm/s");
            ui->lblAcc->setText(QString::number(m_vm->acceleration(), 'f', 2) + " mm/s²");
        });
        // Push data to the plots
        connect(m_vm.get(), &MotorTestViewModel::s_telemetryChanged, this, [this]() {
            ui->lblPos->setText(QString::number(m_vm->position(), 'f', 4) + " mm");
            ui->plotPosition->addDataPoint("real", m_vm->position());
            ui->plotSpeed->addDataPoint("real", m_vm->speed());
            ui->plotAcceleration->addDataPoint("real", m_vm->acceleration());
        });
        // Clear the plots + define buttons state when the motor changes
        connect(m_vm.get(), &MotorTestViewModel::s_hasValidMotorChanged, this, [this]() {
            bool valid = m_vm->hasValidMotor();

            ui->plotPosition->clear();
            ui->plotSpeed->clear();
            ui->plotAcceleration->clear();
            ui->grpKinematics->setEnabled(valid);
            ui->btnJogPos->setEnabled(valid);
            ui->btnJogNeg->setEnabled(valid);
            ui->btnGoAbs->setEnabled(valid);
        });
    }

    MotorTestView::~MotorTestView() = default;

    void MotorTestView::invokeJog(int direction)
    {
        // Extract parameters from UI
        double v = ui->spinVelocity->value();
        double a = ui->spinAcceleration->value();
        int sf   = ui->cbStepFraction->currentData().toInt();

        m_vm->uiJogHold(direction, v, a, sf);
    }

    void MotorTestView::invokeAbsoluteMove()
    {
        double target = ui->spinTargetPos->value();
        double v      = ui->spinVelocity->value();
        double a      = ui->spinAcceleration->value();
        int sf        = ui->cbStepFraction->currentData().toInt();

        m_vm->uiMoveToAbsolute(target, v, a, sf);
    }

}
