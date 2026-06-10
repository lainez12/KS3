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

        // X and Y are in millimeters, Theta is in degrees
        m_resetX     = createCoordinateSpinBox();
        m_resetY     = createCoordinateSpinBox();
        m_resetTheta = createCoordinateSpinBox();

        formLayout->addRow("X-Axis Reset Position (mm):", m_resetX);
        formLayout->addRow("Y-Axis Reset Position (mm):", m_resetY);
        formLayout->addRow("Theta-Axis Reset Position (mm):", m_resetTheta);

        m_layout->addLayout(formLayout);
        m_layout->addStretch();
    }

    void AlignmentPositionsPage::loadInitialData(const Config::alignment_process_config_t &conf)
    {
        m_resetX->setValue(conf.x_stage_center_pos_mm);
        m_resetY->setValue(conf.y_stage_center_pos_mm);
        m_resetTheta->setValue(conf.theta_stage_center_pos_mm);
    }

    void AlignmentPositionsPage::pullDataToStruct(Config::alignment_process_config_t &out) const
    {
        out.x_stage_center_pos_mm     = m_resetX->value();
        out.y_stage_center_pos_mm     = m_resetY->value();
        out.theta_stage_center_pos_mm = m_resetTheta->value();
    }

} // namespace Kub3::Components
