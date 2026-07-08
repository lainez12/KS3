#pragma once

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
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

        QDoubleSpinBox *m_leftCamXDistanceMm;   // Holds value in mm
        QDoubleSpinBox *m_rightCamXDistanceMm;  // Holds value in mm
        QDoubleSpinBox *m_leftCamYDistanceMm;   // Holds value in mm
        QDoubleSpinBox *m_rightCamYDistanceMm;  // Holds value in mm
        QDoubleSpinBox *m_xStageDistanceMm;     // Holds value in mm
        QDoubleSpinBox *m_yStageDistanceMm;     // Holds value in mm
        QDoubleSpinBox *m_thetaStageDistanceMm; // Holds value in mm
        QDoubleSpinBox *m_zMotorsDistanceMm;    // Holds value in mm
    };

}
