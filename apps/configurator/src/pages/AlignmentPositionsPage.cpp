#include <pages/AlignmentPositionsPage.h>

namespace Kub3::Components
{
    // Helper to generate consistent UI fields for physical dimensions
    static QDoubleSpinBox *createCoordinateSpinBox(double min = -100000.0, double max = 100000.0, int decimals = 4)
    {
        auto *sb = new QDoubleSpinBox();
        sb->setRange(min, max);
        sb->setDecimals(decimals);
        return sb;
    }

    AlignmentPositionsPage::AlignmentPositionsPage(const Config::alignment_process_config_t &conf, QWidget *parent) : QWidget(parent)
    {
        setupUI();
        loadInitialData(conf);
    }

    void AlignmentPositionsPage::setupUI()
    {
        m_layout = new QVBoxLayout(this);

        auto *header = new QLabel("<b>Alignment Stage Calibration</b>");
        header->setStyleSheet("font-size: 14px; margin-bottom: 15px;");
        m_layout->addWidget(header);

        auto *formLayout = new QFormLayout();

        m_xCenterMm     = createCoordinateSpinBox();
        m_yCenterMm     = createCoordinateSpinBox();
        m_thetaCenterMm = createCoordinateSpinBox();

        formLayout->addRow("X-Axis Center Position (mm):", m_xCenterMm);
        formLayout->addRow("Y-Axis Center Position (mm):", m_yCenterMm);
        formLayout->addRow("Theta-Axis Center Position (mm):", m_thetaCenterMm);

        m_layout->addLayout(formLayout);
        m_layout->addStretch();
    }

    void AlignmentPositionsPage::loadInitialData(const Config::alignment_process_config_t &conf)
    {
        m_xCenterMm->setValue(conf.x_stage_center_pos_mm);
        m_yCenterMm->setValue(conf.y_stage_center_pos_mm);
        m_thetaCenterMm->setValue(conf.theta_stage_center_pos_mm);
    }

    void AlignmentPositionsPage::pullDataToStruct(Config::alignment_process_config_t &out) const
    {
        out.x_stage_center_pos_mm     = m_xCenterMm->value();
        out.y_stage_center_pos_mm     = m_yCenterMm->value();
        out.theta_stage_center_pos_mm = m_thetaCenterMm->value();
    }

} // namespace Kub3::Components
