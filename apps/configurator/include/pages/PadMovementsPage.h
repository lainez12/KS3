#pragma once

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <Config/conf.h>

namespace Kub3::Components
{

    class PadMovementsPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit PadMovementsPage(const Kub3::Config::pad_process_config_t &conf, QWidget *parent = nullptr);

        void pullDataToStruct(Kub3::Config::pad_process_config_t &outConf) const;

    private:
        void setupUI();
        void loadInitialData(const Kub3::Config::pad_process_config_t &conf);

    private:
        QVBoxLayout *m_layout = nullptr;
        QTabWidget *m_tabs    = nullptr;

        // Cameras movements
        QDoubleSpinBox *m_leftCamXDistanceMm  = nullptr; // Holds value in mm
        QDoubleSpinBox *m_rightCamXDistanceMm = nullptr; // Holds value in mm
        QDoubleSpinBox *m_leftCamYDistanceMm  = nullptr; // Holds value in mm
        QDoubleSpinBox *m_rightCamYDistanceMm = nullptr; // Holds value in mm
        // Cameras' Lights/Focals settings
        QDoubleSpinBox *m_leftCamLight  = nullptr;
        QDoubleSpinBox *m_rightCamLight = nullptr;
        QDoubleSpinBox *m_leftCamFocal  = nullptr;
        QDoubleSpinBox *m_rightCamFocal = nullptr;
        // Alignment / Z-Elevator
        QDoubleSpinBox *m_xStageDistanceMm     = nullptr; // Holds value in mm
        QDoubleSpinBox *m_yStageDistanceMm     = nullptr; // Holds value in mm
        QDoubleSpinBox *m_thetaStageDistanceMm = nullptr; // Holds value in mm
        QDoubleSpinBox *m_zMotorsDistanceMm    = nullptr; // Holds value in mm
    };

}
