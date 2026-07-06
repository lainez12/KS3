#include <QGroupBox>

#include <pages/ZAlgorithmicLimitsPage.h>

namespace Kub3::Components
{
    static QDoubleSpinBox *createDoubleSpinBox(double minLimit = 0.0, double maxLimit = 10000.0, int decimals = 2)
    {
        auto *sb = new QDoubleSpinBox();

        sb->setRange(minLimit, maxLimit);
        sb->setDecimals(decimals);
        sb->setSingleStep(std::min((maxLimit - minLimit) / 100.0, 1.0));
        return sb;
    }

    ZAlgorithmicLimitsPage::ZAlgorithmicLimitsPage(const Kub3::Config::process_config_t &conf, QWidget *parent) : QWidget(parent)
    {
        setupUI();
        loadInitialData(conf);
    }

    void ZAlgorithmicLimitsPage::setupUI()
    {
        m_layout = new QVBoxLayout(this);

        auto *header = new QLabel("<b>Z-axes Algorithmic Limits</b>");
        header->setStyleSheet("font-size: 14px; margin-bottom: 15px;");
        m_layout->addWidget(header);

        auto kinematicSafetyLimitsGroup = new QGroupBox(this);
        {
            auto *formLayout = new QFormLayout();

            m_maxZRelativeDistMm   = createDoubleSpinBox(0.0, 10.0, 4);
            m_maxMovementPerTickMm = createDoubleSpinBox(0.0, 10.0, 3);
            m_deadbandVelocityMmS  = createDoubleSpinBox(0.0001, 5.0, 5);

            formLayout->addRow("Maximum Z axes relative distance (mm):", m_maxZRelativeDistMm);
            formLayout->addRow("Maximum movement per loop cycle (mm):", m_maxMovementPerTickMm);
            formLayout->addRow("Deadband/minimum velocity (mm/s):", m_deadbandVelocityMmS);

            kinematicSafetyLimitsGroup->setTitle("Kinematic Safety Limits");
            kinematicSafetyLimitsGroup->setLayout(formLayout);
        }
        m_layout->addWidget(kinematicSafetyLimitsGroup);

        auto admittancePlanarizationTuningGroup = new QGroupBox(this);
        {
            auto *formLayout = new QFormLayout();

            m_dsbKMeanLowForceMmSGF  = createDoubleSpinBox(0.0, 1.0, 5);
            m_dsbKMeanHighForceMmSGF = createDoubleSpinBox(0.0, 1.0, 5);
            m_dsbKTiltLowForceMmSGF  = createDoubleSpinBox(0.0, 1.0, 5);
            m_dsbKTiltHighForceMmSGF = createDoubleSpinBox(0.0, 1.0, 5);

            formLayout->addRow("Translational Gain - Low Force (mm/s/gf)", m_dsbKMeanLowForceMmSGF);
            formLayout->addRow("Translational Gain - High Force (mm/s/gf)", m_dsbKMeanHighForceMmSGF);
            formLayout->addRow("Rotational Gain - Low Force (mm/s/gf)", m_dsbKTiltLowForceMmSGF);
            formLayout->addRow("Rotational Gain - High Force (mm/s/gf)", m_dsbKTiltHighForceMmSGF);

            // Low Force Gain dictates the MAXIMUM allowed High Force Gain
            connect(m_dsbKMeanLowForceMmSGF, &QDoubleSpinBox::valueChanged, m_dsbKMeanHighForceMmSGF, &QDoubleSpinBox::setMaximum);
            connect(m_dsbKTiltLowForceMmSGF, &QDoubleSpinBox::valueChanged, m_dsbKTiltHighForceMmSGF, &QDoubleSpinBox::setMaximum);

            admittancePlanarizationTuningGroup->setTitle("Admittance / Planarization Tuning (Wedge Error Correction)");
            admittancePlanarizationTuningGroup->setLayout(formLayout);
        }
        m_layout->addWidget(admittancePlanarizationTuningGroup);

        m_layout->addStretch();
    }

    void ZAlgorithmicLimitsPage::loadInitialData(const Kub3::Config::process_config_t &conf)
    {
        const QSignalBlocker b1(m_dsbKMeanLowForceMmSGF);
        const QSignalBlocker b2(m_dsbKMeanHighForceMmSGF);
        const QSignalBlocker b3(m_dsbKTiltLowForceMmSGF);
        const QSignalBlocker b4(m_dsbKTiltHighForceMmSGF);

        // Load the values
        // --- Kinematics safety limits
        m_maxZRelativeDistMm->setValue(conf.elevator.max_z_relative_distance_mm);
        // --- Admittance tuning
        m_deadbandVelocityMmS->setValue(conf.contact.admittance.deadband_velocity_mm_s);
        m_maxMovementPerTickMm->setValue(conf.contact.admittance.max_step_mm_per_tick);
        m_dsbKMeanLowForceMmSGF->setValue(conf.contact.admittance.translational_gain_low_force);
        m_dsbKMeanHighForceMmSGF->setValue(conf.contact.admittance.translational_gain_high_force);
        m_dsbKTiltLowForceMmSGF->setValue(conf.contact.admittance.rotational_gain_low_force);
        m_dsbKTiltHighForceMmSGF->setValue(conf.contact.admittance.rotational_gain_high_force);

        // Initialize the boundaries
        m_dsbKMeanHighForceMmSGF->setMaximum(conf.contact.admittance.translational_gain_low_force);
        m_dsbKTiltHighForceMmSGF->setMaximum(conf.contact.admittance.rotational_gain_low_force);
    }

    void ZAlgorithmicLimitsPage::pullDataToStruct(Kub3::Config::process_config_t &outConf) const
    {
        // Kinematics safety limits
        outConf.elevator.max_z_relative_distance_mm = m_maxZRelativeDistMm->value();
        // Admittance tuning
        outConf.contact.admittance.deadband_velocity_mm_s        = m_deadbandVelocityMmS->value();
        outConf.contact.admittance.max_step_mm_per_tick          = m_maxMovementPerTickMm->value();
        outConf.contact.admittance.translational_gain_low_force  = m_dsbKMeanLowForceMmSGF->value();
        outConf.contact.admittance.translational_gain_high_force = m_dsbKMeanHighForceMmSGF->value();
        outConf.contact.admittance.rotational_gain_low_force     = m_dsbKTiltLowForceMmSGF->value();
        outConf.contact.admittance.rotational_gain_high_force    = m_dsbKTiltHighForceMmSGF->value();
    }

}
