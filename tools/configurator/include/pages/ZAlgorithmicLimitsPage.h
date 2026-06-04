#pragma once

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

#include <Config/machine_config.h>

namespace Kub3::Components
{

    class ZAlgorithmicLimitsPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit ZAlgorithmicLimitsPage(const Kub3::Config::process_config_t &conf, QWidget *parent = nullptr);

        void pullDataToStruct(Kub3::Config::process_config_t &outConf) const;

    private:
        void setupUI();
        void loadInitialData(const Kub3::Config::process_config_t &conf);

    private:
        QVBoxLayout *m_layout = nullptr;

        QDoubleSpinBox *m_maxZRelativeDistMm   = nullptr; // Holds value in millimeter
        QDoubleSpinBox *m_maxMovementPerTickMm = nullptr; // Holds value in millimeter

        QDoubleSpinBox *m_dsbKMeanLowForceMmSGF  = nullptr; // Holds value in grams Force
        QDoubleSpinBox *m_dsbKMeanHighForceMmSGF = nullptr; // Holds value in grams Force
        QDoubleSpinBox *m_dsbKTiltLowForceMmSGF  = nullptr; // Holds value in grams Force
        QDoubleSpinBox *m_dsbKTiltHighForceMmSGF = nullptr; // Holds value in grams Force
    };

}
