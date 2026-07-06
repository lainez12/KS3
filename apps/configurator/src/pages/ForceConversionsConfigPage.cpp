#include <HAL/MachineStatus/sensors_labels.h>
#include <pages/ForceConversionsConfigPage.h>

namespace Kub3::Components
{
    static QDoubleSpinBox *createDoubleSpinBox(double maxLimit = 10000.0, int decimals = 2)
    {
        auto *sb = new QDoubleSpinBox();
        sb->setRange(0.0, maxLimit);
        sb->setDecimals(decimals);
        return sb;
    }

    ForceConversionsConfigPage::ForceConversionsConfigPage(const Config::hardware_config_t &conf, QWidget *parent) :
        QWidget(parent)
    {
        setupUI();
        loadInitialData(conf);
    }

    void ForceConversionsConfigPage::setupUI()
    {
        m_layout = new QVBoxLayout(this);

        auto *header = new QLabel("<b>Hardware Force Conversions factors</b>");
        header->setStyleSheet("font-size: 14px; margin-bottom: 15px;");
        m_layout->addWidget(header);

        auto *formLayout = new QFormLayout();

        m_leftADCToGFRatio  = createDoubleSpinBox(100.0, 3);
        m_rightADCToGFRatio = createDoubleSpinBox(100.0, 3);
        m_backADCToGFRatio  = createDoubleSpinBox(100.0, 3);

        formLayout->addRow("Left ADC to gram-force multiplier :", m_leftADCToGFRatio);
        formLayout->addRow("Right ADC to gram-force multiplier :", m_rightADCToGFRatio);
        formLayout->addRow("Back ADC to gram-force multiplier :", m_backADCToGFRatio);

        m_layout->addLayout(formLayout);
        m_layout->addStretch();
    }

    void ForceConversionsConfigPage::loadInitialData(const Config::hardware_config_t &conf)
    {
        auto notFoundFallback = [](QDoubleSpinBox *sb, const char *id) {
            qWarning().noquote().nospace()
                << "[ForceConversionsConfigPage] Failed to load initial data for: '" << id
                << "'. Falling back to 1.0";
            if (sb)
                sb->setValue(1.0);
            else
                qCritical() << "Failed to set default value for:" << id;
        };

        if (auto itFactor = conf.adc_to_gf_factors.find(FORCE_LEFT_ADC);
            itFactor != conf.adc_to_gf_factors.end())
        {
            m_leftADCToGFRatio->setValue(itFactor->second);
        }
        else
        {
            notFoundFallback(m_leftADCToGFRatio, FORCE_LEFT_ADC);
        }

        if (auto itFactor = conf.adc_to_gf_factors.find(FORCE_RIGHT_ADC);
            itFactor != conf.adc_to_gf_factors.end())
        {
            m_rightADCToGFRatio->setValue(itFactor->second);
        }
        else
        {
            notFoundFallback(m_rightADCToGFRatio, FORCE_RIGHT_ADC);
        }

        if (auto itFactor = conf.adc_to_gf_factors.find(FORCE_BACK_ADC);
            itFactor != conf.adc_to_gf_factors.end())
        {
            m_backADCToGFRatio->setValue(itFactor->second);
        }
        else
        {
            notFoundFallback(m_backADCToGFRatio, FORCE_BACK_ADC);
        }
    }

    void ForceConversionsConfigPage::pullDataToStruct(Config::hardware_config_t &out) const
    {
        out.adc_to_gf_factors = {
            {FORCE_LEFT_ADC, m_leftADCToGFRatio->value()},
            {FORCE_RIGHT_ADC, m_rightADCToGFRatio->value()},
            {FORCE_BACK_ADC, m_backADCToGFRatio->value()},
        };
    }

}
