#pragma once

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <Config/process.h>

namespace Kub3::Components
{

    class CameraGeneralPage : public QWidget
    {
        Q_OBJECT

    public:
        explicit CameraGeneralPage(const Config::vision_process_config_t &conf, QWidget *parent = nullptr);
        void pullDataToStruct(Config::vision_process_config_t &out) const;

    private:
        void setupUI();
        void loadInitialData(const Config::vision_process_config_t &conf);

    private:
        QVBoxLayout *m_layout = nullptr;

        // Position & Clearance Limits
        QDoubleSpinBox *m_minCamDist        = nullptr;
        QDoubleSpinBox *m_leftCamXResetPos  = nullptr;
        QDoubleSpinBox *m_leftCamYResetPos  = nullptr;
        QDoubleSpinBox *m_rightCamXResetPos = nullptr;
        QDoubleSpinBox *m_rightCamYResetPos = nullptr;

        // Left Focal Configuration
        QSpinBox *m_leftFocalMin     = nullptr;
        QSpinBox *m_leftFocalMax     = nullptr;
        QSpinBox *m_leftFocalDefault = nullptr;

        // Right Focal Configuration
        QSpinBox *m_rightFocalMin     = nullptr;
        QSpinBox *m_rightFocalMax     = nullptr;
        QSpinBox *m_rightFocalDefault = nullptr;
    };

} // namespace Kub3::Components
