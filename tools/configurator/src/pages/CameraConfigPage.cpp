#include <format>

#include <pages/CameraConfigPage.h>
#include <utils.h>

namespace Kub3::Components
{
    // Helper to generate consistent UI fields
    static QDoubleSpinBox *createDoubleSpinBox(double maxLimit, int decimals = 2)
    {
        auto *sb = new QDoubleSpinBox();
        sb->setRange(0.0, maxLimit);
        sb->setDecimals(decimals);
        return sb;
    }

    CameraConfigPage::CameraConfigPage(const Kub3::Config::camera_config_t &conf, QWidget *parent) : QWidget(parent)
    {
        setupUI(conf);
        loadInitialData(conf);
    }

    void CameraConfigPage::setupUI(const Kub3::Config::camera_config_t &conf)
    {
        m_layout = new QVBoxLayout(this);

        // Header
        auto *header = new QLabel(QString("<b>Configuration for Camera: %1</b>").arg(Kub3::camelToNormal(conf.id)));
        header->setStyleSheet("font-size: 14px; margin-bottom: 15px;");
        m_layout->addWidget(header);

        auto *formLayout = new QFormLayout();

        m_serialNumber = new QLineEdit();

        // Machine Vision cameras often have exposures up to 1+ seconds (1,000,000 us)
        m_maxExposureUs     = createDoubleSpinBox(10000000.0, 2);
        m_defaultExposureUs = createDoubleSpinBox(10000000.0, 2);
        // Gain typically ranges from 0 to ~40-100 dB depending on the sensor
        m_maxGainDb     = createDoubleSpinBox(100.0, 2);
        m_defaultGainDb = createDoubleSpinBox(100.0, 2);
        // Framerate
        m_cbFramerate                  = new QComboBox();
        const QList<double> fpsOptions = {24, 25, 30, 48, 50, 60};
        for (double fps : fpsOptions)
        {
            m_cbFramerate->addItem(QString("%1 FPS").arg(fps), QVariant::fromValue(fps));
        }

        formLayout->addRow("Serial Number:", m_serialNumber);
        formLayout->addRow("Maximum Exposure (µs):", m_maxExposureUs);
        formLayout->addRow("Default Exposure (µs):", m_defaultExposureUs);
        formLayout->addRow("Maximum Gain (dB):", m_maxGainDb);
        formLayout->addRow("Default Gain (dB):", m_defaultGainDb);
        formLayout->addRow("Default Framerate:", m_cbFramerate);

        m_layout->addLayout(formLayout);
        m_layout->addStretch(); // Push UI to the top

        // --- POKA-YOKE (MISTAKE PROOFING) ---
        // Dynamically clamp the 'Default' spinboxes so they can never exceed the 'Maximum' spinboxes.
        connect(m_maxExposureUs, &QDoubleSpinBox::valueChanged, m_defaultExposureUs, &QDoubleSpinBox::setMaximum);
        connect(m_maxGainDb, &QDoubleSpinBox::valueChanged, m_defaultGainDb, &QDoubleSpinBox::setMaximum);
    }

    void CameraConfigPage::loadInitialData(const Kub3::Config::camera_config_t &conf)
    {
        // Set maximums first to avoid Qt automatically clamping default values during initialization
        m_maxExposureUs->setValue(conf.maxExposureUs);
        m_maxGainDb->setValue(conf.maxGainDb);

        // Apply clamping limits to the default boxes
        m_defaultExposureUs->setMaximum(conf.maxExposureUs);
        m_defaultGainDb->setMaximum(conf.maxGainDb);

        // Set actual default values
        m_defaultExposureUs->setValue(conf.defaultExposureUs);
        m_defaultGainDb->setValue(conf.defaultGainDb);

        m_serialNumber->setText(QString::fromStdString(conf.serialNumber));

        if (int idx = m_cbFramerate->findData(conf.framerate); idx != -1)
            m_cbFramerate->setCurrentIndex(idx);
        else
        {
            // Failsafe: If the .ini has an unusual custom framerate (e.g., set manually in a text editor),
            // inject it dynamically so it isn't accidentally lost/overwritten.
            m_cbFramerate->addItem(QString("%1 FPS (Custom)").arg(conf.framerate), conf.framerate);
            m_cbFramerate->setCurrentIndex(m_cbFramerate->count() - 1);
        }
    }

    void CameraConfigPage::pullDataToStruct(Kub3::Config::camera_config_t &outConf) const
    {
        // Note: outConf.id is strictly preserved by NOT overwriting it here.
        outConf.serialNumber      = m_serialNumber->text().toStdString();
        outConf.maxExposureUs     = m_maxExposureUs->value();
        outConf.defaultExposureUs = m_defaultExposureUs->value();
        outConf.maxGainDb         = m_maxGainDb->value();
        outConf.defaultGainDb     = m_defaultGainDb->value();
        outConf.framerate         = m_cbFramerate->currentData().toDouble();
    }

} // namespace Kub3::Components
