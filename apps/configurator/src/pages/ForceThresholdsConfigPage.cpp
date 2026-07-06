#include <pages/ForceThresholdsConfigPage.h>

namespace Kub3::Components
{
    static QDoubleSpinBox *createDoubleSpinBox(double maxLimit = 10000.0, int decimals = 2)
    {
        auto *sb = new QDoubleSpinBox();
        sb->setRange(0.0, maxLimit);
        sb->setDecimals(decimals);
        return sb;
    }

    ForceThresholdsConfigPage::ForceThresholdsConfigPage(const Config::contact_process_config_t &conf, QWidget *parent) : QWidget(parent)
    {
        setupUI();
        loadInitialData(conf);
    }

    void ForceThresholdsConfigPage::setupUI()
    {
        m_layout = new QVBoxLayout(this);

        auto *header = new QLabel("<b>Force & Contact Limits</b>");
        header->setStyleSheet("font-size: 14px; margin-bottom: 15px;");
        m_layout->addWidget(header);

        auto *formLayout = new QFormLayout();

        m_hwCrashForce            = createDoubleSpinBox(20000.0); // Absolute hardware max
        m_maxProcessForce         = createDoubleSpinBox(20000.0);
        m_contactThreshold        = createDoubleSpinBox(20000.0);
        m_dsbAutolevelTargetForce = createDoubleSpinBox(20000.0);
        m_dsbAutolevelTolerance   = createDoubleSpinBox(100.0);

        formLayout->addRow("Hardware Crash Limit (gf):", m_hwCrashForce);
        formLayout->addRow("Maximum Process Force (gf):", m_maxProcessForce);
        formLayout->addRow("Contact Detection Threshold (gf):", m_contactThreshold);
        formLayout->addRow("Autolevel Target Force (gf):", m_dsbAutolevelTargetForce);
        formLayout->addRow("Autolevel Tolerance (±gf):", m_dsbAutolevelTolerance);

        m_layout->addLayout(formLayout);
        m_layout->addStretch();

        // --- POKA-YOKE (Bounds Logic) ---
        // hw_crash_force >= max_force >= contact_threshold
        connect(m_hwCrashForce, &QDoubleSpinBox::valueChanged, m_maxProcessForce, &QDoubleSpinBox::setMaximum);
        connect(m_maxProcessForce, &QDoubleSpinBox::valueChanged, m_contactThreshold, &QDoubleSpinBox::setMaximum);
    }

    void ForceThresholdsConfigPage::loadInitialData(const Config::contact_process_config_t &conf)
    {
        // Load top-down to prevent premature clamping
        m_hwCrashForce->setValue(conf.hw_crash_force_limit_gf);
        m_maxProcessForce->setMaximum(conf.hw_crash_force_limit_gf);

        m_maxProcessForce->setValue(conf.max_process_force_gf);
        m_contactThreshold->setMaximum(conf.max_process_force_gf);

        m_contactThreshold->setValue(conf.contact_threshold_gf);

        m_dsbAutolevelTargetForce->setValue(conf.autolevel_force_gf);
        m_dsbAutolevelTolerance->setValue(conf.autolevel_force_tolerance_gf);
    }

    void ForceThresholdsConfigPage::pullDataToStruct(Config::contact_process_config_t &out) const
    {
        out.hw_crash_force_limit_gf      = m_hwCrashForce->value();
        out.max_process_force_gf         = m_maxProcessForce->value();
        out.contact_threshold_gf         = m_contactThreshold->value();
        out.autolevel_force_gf           = m_dsbAutolevelTargetForce->value();
        out.autolevel_force_tolerance_gf = m_dsbAutolevelTolerance->value();
    }

}
