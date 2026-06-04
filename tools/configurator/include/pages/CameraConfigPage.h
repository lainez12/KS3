#pragma once

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWidget>

#include <Config/conf.h>

namespace Kub3::Components
{

    class CameraConfigPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit CameraConfigPage(const Kub3::Config::camera_config_t &conf, QWidget *parent = nullptr);

        // Pulls the UI data back into the struct. Note: `outConf.id` is left untouched.
        void pullDataToStruct(Kub3::Config::camera_config_t &outConf) const;

    private:
        void setupUI(const Kub3::Config::camera_config_t &conf);
        void loadInitialData(const Kub3::Config::camera_config_t &conf);

    private:
        QVBoxLayout *m_layout = nullptr;

        QLineEdit *m_serialNumber           = nullptr;
        QDoubleSpinBox *m_maxExposureUs     = nullptr;
        QDoubleSpinBox *m_defaultExposureUs = nullptr;
        QDoubleSpinBox *m_maxGainDb         = nullptr;
        QDoubleSpinBox *m_defaultGainDb     = nullptr;
    };

} // namespace Kub3::Components
