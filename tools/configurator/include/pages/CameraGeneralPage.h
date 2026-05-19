#pragma once

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

#include <Config/machine_config.h>

namespace Kub3::Components
{

    class CameraGeneralPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit CameraGeneralPage(const Kub3::Config::process_config_t &conf, QWidget *parent = nullptr);
        void pullDataToStruct(Kub3::Config::process_config_t &outConf) const;

    private:
        QVBoxLayout *m_layout               = nullptr;
        QDoubleSpinBox *m_minCamDist        = nullptr;
        QDoubleSpinBox *m_leftCamXResetPos  = nullptr;
        QDoubleSpinBox *m_leftCamYResetPos  = nullptr;
        QDoubleSpinBox *m_rightCamXResetPos = nullptr;
        QDoubleSpinBox *m_rightCamYResetPos = nullptr;
    };

}
