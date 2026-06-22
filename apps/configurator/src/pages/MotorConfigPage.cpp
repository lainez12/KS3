#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <format>

#include <Config/keys/hardware.h>
#include <pages/MotorConfigPage.h>
#include <utils.h>

namespace Kub3::Components
{
    static QDoubleSpinBox *createDoubleSpinBox(double min = 0.0, double max = 10000.0)
    {
        auto *sb = new QDoubleSpinBox();
        sb->setRange(min, max);
        sb->setDecimals(4);
        return sb;
    }

    static QSpinBox *createSpinBox(int min = 0, int max = 1000000)
    {
        auto *sb = new QSpinBox();
        sb->setRange(min, max);
        return sb;
    }

    MotorConfigPage::MotorConfigPage(const Kub3::Config::motor_config_t &hwConf,
                                     const Kub3::Config::KinematicProfiles &kinConf,
                                     const QStringList &allMotorIds,
                                     QWidget *parent) : QWidget(parent),
                                                        m_profiles(kinConf),
                                                        m_allMotorIds(allMotorIds),
                                                        m_currentMotorId(QString::fromStdString(hwConf.id))
    {
        setupUI(m_currentMotorId);
        loadInitialData(hwConf);
        refreshProfileList();
    }

    void MotorConfigPage::setupUI(const QString &motorId)
    {
        m_layout     = new QVBoxLayout(this);
        auto *header = new QLabel(QString("<b>Configuration for Motor: %1</b>").arg(camelToNormal(motorId)));
        header->setStyleSheet("font-size: 14px; margin-bottom: 5px;");
        m_layout->addWidget(header);

        m_tabs = new QTabWidget();
        setupHardwareTab();
        setupKinematicsTab();
        onHardwareTypeChanged(m_typeSelector->currentIndex());
        m_layout->addWidget(m_tabs);
    }

    void MotorConfigPage::setupHardwareTab()
    {
        auto *page   = new QWidget();
        auto *layout = new QVBoxLayout(page);

        auto *typeLayout = new QFormLayout();
        m_typeSelector   = new QComboBox();
        m_typeSelector->addItem("Stepper Motor", CONF_HW_MOTOR_TYPE_STEPPER);
        m_typeSelector->addItem("DC Motor", CONF_HW_MOTOR_TYPE_DC);
        typeLayout->addRow("Motor Type:", m_typeSelector);
        layout->addLayout(typeLayout);

        m_typeStack = new QStackedWidget();

        // 1. Stepper Form
        auto *stepPage   = new QWidget();
        auto *stepForm   = new QFormLayout(stepPage);
        m_stepPerRev     = createSpinBox();
        m_stepScrewPitch = createDoubleSpinBox(0.01);
        m_stepMaxVel     = createDoubleSpinBox();
        m_stepMaxAcc     = createDoubleSpinBox();
        m_stepEncTops    = createSpinBox();
        stepForm->addRow("Steps Per Revolution:", m_stepPerRev);
        stepForm->addRow("Screw Pitch (mm):", m_stepScrewPitch);
        stepForm->addRow("Max Velocity (mm/s):", m_stepMaxVel);
        stepForm->addRow("Max Acceleration (mm/s²):", m_stepMaxAcc);
        stepForm->addRow("Encoder Tops Per Rev:", m_stepEncTops);
        m_typeStack->addWidget(stepPage);

        // 2. DC Form
        auto *dcPage   = new QWidget();
        auto *dcForm   = new QFormLayout(dcPage);
        m_dcScrewPitch = createDoubleSpinBox(0.01);
        m_dcMaxVel     = createDoubleSpinBox();
        m_dcMaxAcc     = createDoubleSpinBox();
        m_dcEncTops    = createSpinBox();
        dcForm->addRow("Screw Pitch (mm):", m_dcScrewPitch);
        dcForm->addRow("Max Velocity (mm/s):", m_dcMaxVel);
        dcForm->addRow("Max Acceleration (mm/s²):", m_dcMaxAcc);
        dcForm->addRow("Encoder Tops Per Rev:", m_dcEncTops);
        m_typeStack->addWidget(dcPage);

        layout->addWidget(m_typeStack);
        layout->addStretch();

        connect(m_typeSelector, &QComboBox::currentIndexChanged, m_typeStack, &QStackedWidget::setCurrentIndex);
        connect(m_typeSelector, &QComboBox::currentIndexChanged, this, &MotorConfigPage::onHardwareTypeChanged);
        m_tabs->addTab(page, "Hardware");
    }

    void MotorConfigPage::setupKinematicsTab()
    {
        auto *page       = new QWidget();
        auto *mainLayout = new QHBoxLayout(page);

        // LEFT: List and Action Buttons
        auto *listLayout = new QVBoxLayout();
        m_profileList    = new QListWidget();
        listLayout->addWidget(m_profileList);

        auto *btnLayout = new QHBoxLayout();
        auto *btnAdd    = new QPushButton("+");
        auto *btnRem    = new QPushButton("-");
        auto *btnExp    = new QPushButton("Export...");
        btnLayout->addWidget(btnAdd);
        btnLayout->addWidget(btnRem);
        btnLayout->addWidget(btnExp);
        listLayout->addLayout(btnLayout);

        mainLayout->addLayout(listLayout, 1);

        // RIGHT: Profile Form
        m_profileFormWidget = new QWidget();
        auto *formLayout    = new QFormLayout(m_profileFormWidget);

        m_kinInitVel   = createDoubleSpinBox();
        m_kinTargetVel = createDoubleSpinBox();
        m_kinAcc       = createDoubleSpinBox();

        formLayout->addRow("Initial Velocity (mm/s):", m_kinInitVel);
        formLayout->addRow("Target Velocity (mm/s):", m_kinTargetVel);
        formLayout->addRow("Acceleration (mm/s²):", m_kinAcc);

        // Contextual Params Title
        m_kinParamsTitle = new QLabel("");
        m_kinParamsTitle->setStyleSheet("font-weight: bold; margin-top: 15px; margin-bottom: 5px;");
        formLayout->addRow(m_kinParamsTitle);

        // Contextual Params Stack (No ComboBox anymore)
        m_kinParamsStack = new QStackedWidget();
        m_kinParamsStack->addWidget(new QWidget()); // Index 0: None / Generic (Used for DC)

        auto *stepParamPage = new QWidget();
        auto *stepParamForm = new QFormLayout(stepParamPage);
        stepParamForm->setContentsMargins(0, 0, 0, 0);

        m_kinStepFraction           = new QComboBox();
        const QList<int> stepValues = {1, 2, 4, 8, 16, 32};
        for (int val : stepValues)
            m_kinStepFraction->addItem(QString::number(val), val);
        stepParamForm->addRow("Step Fraction (1/x):", m_kinStepFraction);
        m_kinParamsStack->addWidget(stepParamPage); // Index 1: Stepper

        formLayout->addRow("", m_kinParamsStack);

        mainLayout->addWidget(m_profileFormWidget, 2);

        connect(m_profileList, &QListWidget::currentItemChanged, this, &MotorConfigPage::onProfileSelectionChanged);
        connect(btnAdd, &QPushButton::clicked, this, &MotorConfigPage::onAddProfile);
        connect(btnRem, &QPushButton::clicked, this, &MotorConfigPage::onRemoveProfile);
        connect(btnExp, &QPushButton::clicked, this, &MotorConfigPage::onExportProfile);

        m_tabs->addTab(page, "Process / Kinematics");
    }

    void MotorConfigPage::loadInitialData(const Kub3::Config::motor_config_t &hwConf)
    {
        // (Hardware load logic unchanged from previous step... omitted here for brevity but keep it exactly as it was)
        if (std::holds_alternative<Kub3::Config::stepper_hw_properties_t>(hwConf.hwProperties))
        {
            m_typeSelector->setCurrentIndex(0);
            const auto &s = std::get<Kub3::Config::stepper_hw_properties_t>(hwConf.hwProperties);
            m_stepPerRev->setValue(s.stepsPerRev);
            m_stepScrewPitch->setValue(s.screwPitchMm);
            m_stepMaxVel->setValue(s.maxVelocityMmS);
            m_stepMaxAcc->setValue(s.maxAccelerationMmS2);
            m_stepEncTops->setValue(s.encoderTopsPerRev);
        }
        else if (std::holds_alternative<Kub3::Config::dc_motor_hw_properties_t>(hwConf.hwProperties))
        {
            m_typeSelector->setCurrentIndex(1);
            const auto &dc = std::get<Kub3::Config::dc_motor_hw_properties_t>(hwConf.hwProperties);
            m_dcScrewPitch->setValue(dc.screwPitchMm);
            m_dcMaxVel->setValue(dc.maxVelocityMmS);
            m_dcMaxAcc->setValue(dc.maxAccelerationMmS2);
        }
    }

    void MotorConfigPage::onHardwareTypeChanged(int index)
    {
        // 1. Save currently viewed profile so we don't lose its basic fields
        if (auto *currentItem = m_profileList->currentItem())
        {
            saveCurrentProfileForm(currentItem->text());
        }

        QString hwType = m_typeSelector->itemData(index).toString();

        if (hwType == CONF_HW_MOTOR_TYPE_STEPPER)
        {
            m_kinParamsTitle->setText("Stepper Specific Settings:");
            m_kinParamsTitle->show();
            m_kinParamsStack->setCurrentIndex(1); // Show Stepper options

            // Enforce default params on ALL existing RAM profiles immediately
            for (auto &[id, prof] : m_profiles)
            {
                if (!std::holds_alternative<Kub3::Config::stepper_kinematics_params_t>(prof.params))
                {
                    prof.params = Kub3::Config::stepper_kinematics_params_t{.stepFraction = 1}; // Sensible default
                }
            }
        }
        else if (hwType == CONF_HW_MOTOR_TYPE_DC)
        {
            m_kinParamsTitle->hide();
            m_kinParamsStack->setCurrentIndex(0); // Show empty/generic options

            // Strip specific params from ALL existing RAM profiles
            for (auto &[id, prof] : m_profiles)
            {
                prof.params = std::monostate{};
            }
        }

        // 2. Reload the UI to reflect the newly enforced params
        if (auto *currentItem = m_profileList->currentItem())
        {
            loadProfileForm(currentItem->text());
        }
    }

    // --- KINEMATICS LOGIC ---

    void MotorConfigPage::refreshProfileList()
    {
        m_profileList->blockSignals(true);
        m_profileList->clear();
        for (const auto &[id, prof] : m_profiles)
        {
            m_profileList->addItem(QString::fromStdString(id));
        }
        m_profileList->blockSignals(false);

        if (m_profileList->count() > 0)
        {
            m_profileFormWidget->setEnabled(true);
            m_profileList->setCurrentRow(0);
        }
        else
        {
            m_profileFormWidget->setEnabled(false);
        }
    }

    void MotorConfigPage::onProfileSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous)
    {
        if (previous)
            saveCurrentProfileForm(previous->text());
        if (current)
        {
            m_profileFormWidget->setEnabled(true);
            loadProfileForm(current->text());
        }
        else
        {
            m_profileFormWidget->setEnabled(false);
        }
    }

    void MotorConfigPage::loadProfileForm(const QString &profileId)
    {
        auto it = m_profiles.find(profileId.toStdString());
        if (it == m_profiles.end())
            return;
        const auto &p = it->second;

        m_kinInitVel->setValue(p.initialVelocityMmS);
        m_kinTargetVel->setValue(p.targetVelocityMmS);
        m_kinAcc->setValue(p.accelerationMmS2);

        if (std::holds_alternative<Kub3::Config::stepper_kinematics_params_t>(p.params))
        {
            m_kinParamsStack->setCurrentIndex(1); // Stepper

            uint8_t fraction = std::get<Kub3::Config::stepper_kinematics_params_t>(p.params).stepFraction;

            // Find the value in the combobox. If not found (e.g. invalid ini value), index will be -1.
            int index = m_kinStepFraction->findData(static_cast<int>(fraction));
            if (index != -1)
                m_kinStepFraction->setCurrentIndex(index);
            else
                // Fallback to Full Step (1) if the file contains an illegal value
                m_kinStepFraction->setCurrentIndex(0);
        }
    }

    void MotorConfigPage::saveCurrentProfileForm(const QString &profileId)
    {
        auto it = m_profiles.find(profileId.toStdString());
        if (it == m_profiles.end())
            return;
        auto &p = it->second;

        p.initialVelocityMmS = m_kinInitVel->value();
        p.targetVelocityMmS  = m_kinTargetVel->value();
        p.accelerationMmS2   = m_kinAcc->value();

        // Save specific params based on strictly what Hardware Type is selected right now.
        QString hwType = m_typeSelector->currentData().toString();
        if (hwType == CONF_HW_MOTOR_TYPE_STEPPER)
        {
            Kub3::Config::stepper_kinematics_params_t params;

            params.stepFraction = static_cast<uint8_t>(m_kinStepFraction->currentData().toInt());
            p.params            = params;
        }
        else
        {
            p.params = std::monostate{};
        }
    }

    void MotorConfigPage::onAddProfile()
    {
        bool ok;
        QString name = QInputDialog::getText(this, "New Profile", "Profile Name:", QLineEdit::Normal, "", &ok);

        if (ok && !name.trimmed().isEmpty())
        {
            std::string id = name.toStdString();
            if (m_profiles.contains(id))
                return; // Already exists

            Kub3::Config::kinematic_profile_t p;
            p.id           = id;
            m_profiles[id] = p;
            refreshProfileList();
            m_profileList->setCurrentItem(m_profileList->findItems(name, Qt::MatchExactly).first());
        }
    }

    void MotorConfigPage::onRemoveProfile()
    {
        if (auto *item = m_profileList->currentItem())
        {
            m_profiles.erase(item->text().toStdString());
            refreshProfileList();
        }
    }

    void MotorConfigPage::onExportProfile()
    {
        auto *item = m_profileList->currentItem();

        if (!item)
            return;

        // Save current changes before exporting
        saveCurrentProfileForm(item->text());

        QStringList targets;
        for (const auto &id : m_allMotorIds)
        {
            if (id != m_currentMotorId)
                targets << id;
        }

        if (targets.isEmpty())
        {
            QMessageBox::warning(this, "Export", "No other motors available.");
            return;
        }

        bool ok;
        QString targetMotor = QInputDialog::getItem(this, "Export Profile", "Target Motor:", targets, 0, false, &ok);

        if (ok && !targetMotor.isEmpty())
        {
            emit profileExportRequested(targetMotor, m_profiles[item->text().toStdString()]);
        }
    }

    // --- FINAL DATA EXTRACTION ---
    void MotorConfigPage::pullDataToStruct(Kub3::Config::motor_config_t &outHw,
                                           Kub3::Config::KinematicProfiles &outKin) const
    {
        // 1. HW Pull logic
        QString currentType = m_typeSelector->currentData().toString();
        if (currentType == CONF_HW_MOTOR_TYPE_STEPPER)
        {
            Kub3::Config::stepper_hw_properties_t stepper;
            stepper.stepsPerRev         = static_cast<uint16_t>(m_stepPerRev->value());
            stepper.screwPitchMm        = m_stepScrewPitch->value();
            stepper.maxVelocityMmS      = m_stepMaxVel->value();
            stepper.maxAccelerationMmS2 = m_stepMaxAcc->value();
            stepper.encoderTopsPerRev   = static_cast<uint16_t>(m_stepEncTops->value());
            outHw.hwProperties          = stepper;
        }
        else if (currentType == CONF_HW_MOTOR_TYPE_DC)
        {
            Kub3::Config::dc_motor_hw_properties_t dc;
            dc.screwPitchMm        = m_dcScrewPitch->value();
            dc.maxVelocityMmS      = m_dcMaxVel->value();
            dc.maxAccelerationMmS2 = m_dcMaxAcc->value();
            outHw.hwProperties     = dc;
        }

        // 2. Kinematics Pull Logic
        // Force the currently active list item to commit to m_profiles first
        if (m_profileList->currentItem())
        {
            const_cast<MotorConfigPage *>(this)->saveCurrentProfileForm(m_profileList->currentItem()->text());
        }

        outKin = m_profiles;
    }

} // namespace Components
