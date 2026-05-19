#include <pages/ForceConfigPage.h>

namespace Kub3::Components
{
    static QDoubleSpinBox *createDoubleSpinBox(double maxLimit = 10000.0, int decimals = 2)
    {
        auto *sb = new QDoubleSpinBox();
        sb->setRange(0.0, maxLimit);
        sb->setDecimals(decimals);
        return sb;
    }

    ForceConfigPage::ForceConfigPage(const Kub3::Config::process_config_t &conf, QWidget *parent) : QWidget(parent)
    {
        setupUI();
        loadInitialData(conf);
    }

    void ForceConfigPage::setupUI()
    {
        m_layout = new QVBoxLayout(this);

        auto *header = new QLabel("<b>Force & Contact Limits</b>");
        header->setStyleSheet("font-size: 14px; margin-bottom: 15px;");
        m_layout->addWidget(header);

        auto *formLayout = new QFormLayout();

        m_hwCrashForce     = createDoubleSpinBox(20000.0); // Absolute hardware max
        m_maxForce         = createDoubleSpinBox(20000.0);
        m_contactThreshold = createDoubleSpinBox(20000.0);

        formLayout->addRow("Hardware Crash Limit (gf):", m_hwCrashForce);
        formLayout->addRow("Maximum Process Force (gf):", m_maxForce);
        formLayout->addRow("Contact Detection Threshold (gf):", m_contactThreshold);

        m_layout->addLayout(formLayout);
        m_layout->addStretch();

        // --- POKA-YOKE (Bounds Logic) ---
        // hw_crash_force >= max_force >= contact_threshold
        connect(m_hwCrashForce, &QDoubleSpinBox::valueChanged, m_maxForce, &QDoubleSpinBox::setMaximum);
        connect(m_maxForce, &QDoubleSpinBox::valueChanged, m_contactThreshold, &QDoubleSpinBox::setMaximum);
    }

    void ForceConfigPage::loadInitialData(const Kub3::Config::process_config_t &conf)
    {
        // Load top-down to prevent premature clamping
        m_hwCrashForce->setValue(conf.hw_crash_force_limit_gf);
        m_maxForce->setMaximum(conf.hw_crash_force_limit_gf);

        m_maxForce->setValue(conf.max_force_gf);
        m_contactThreshold->setMaximum(conf.max_force_gf);

        m_contactThreshold->setValue(conf.contact_threshold_gf);
    }

    void ForceConfigPage::pullDataToStruct(Kub3::Config::process_config_t &outConf) const
    {
        outConf.hw_crash_force_limit_gf = m_hwCrashForce->value();
        outConf.max_force_gf            = m_maxForce->value();
        outConf.contact_threshold_gf    = m_contactThreshold->value();
    }

}
