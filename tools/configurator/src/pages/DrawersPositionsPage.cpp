#include <pages/DrawersPositionsPage.h>

namespace Kub3::Components
{
    static QDoubleSpinBox *createCoordinateSpinBox(double min = -100000.0, double max = 100000.0, int decimals = 4)
    {
        auto *sb = new QDoubleSpinBox();
        sb->setRange(min, max);
        sb->setDecimals(decimals);
        return sb;
    }

    DrawersPositionsPage::DrawersPositionsPage(const Config::drawer_process_config_t &conf, QWidget *parent) : QWidget(parent)
    {
        setupUI();
        loadInitialData(conf);
    }

    void DrawersPositionsPage::setupUI()
    {
        m_layout = new QVBoxLayout(this);

        auto *header = new QLabel("<b>Drawers Calibration</b>");
        header->setStyleSheet("font-size: 14px; margin-bottom: 15px;");
        m_layout->addWidget(header);

        auto *formLayout = new QFormLayout();

        m_cm3Reset = createCoordinateSpinBox();

        formLayout->addRow("CM3 Reset Position (mm):", m_cm3Reset);

        m_layout->addLayout(formLayout);
        m_layout->addStretch();
    }

    void DrawersPositionsPage::loadInitialData(const Config::drawer_process_config_t &conf)
    {
        m_cm3Reset->setValue(conf.cm3_reset_pos_mm);
    }

    void DrawersPositionsPage::pullDataToStruct(Config::drawer_process_config_t &out) const
    {
        out.cm3_reset_pos_mm = m_cm3Reset->value();
    }

} // namespace Kub3::Components
