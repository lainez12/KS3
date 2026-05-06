#include <pages/CameraGeneralPage.h>

namespace Kub3::Components
{
    CameraGeneralPage::CameraGeneralPage(const Kub3::Config::process_config_t &conf, QWidget *parent) : QWidget(parent)
    {
        m_layout = new QVBoxLayout(this);

        auto *header = new QLabel("<b>General Camera Settings</b>");
        header->setStyleSheet("font-size: 14px; margin-bottom: 15px;");
        m_layout->addWidget(header);

        auto *formLayout = new QFormLayout();

        m_minCamDist = new QDoubleSpinBox();
        m_minCamDist->setRange(0.0, 10000.0);
        m_minCamDist->setDecimals(2);
        m_minCamDist->setValue(conf.min_camera_distance_mm);
        formLayout->addRow("Minimum Camera Distance (mm):", m_minCamDist);

        m_leftCamXResetPos = new QDoubleSpinBox();
        m_leftCamXResetPos->setRange(-100000.0, 100000.0);
        m_leftCamXResetPos->setDecimals(2);
        m_leftCamXResetPos->setValue(conf.left_cam_x_reset_pos_mm);
        formLayout->addRow("Left Camera X Reset Position (mm):", m_leftCamXResetPos);

        m_leftCamYResetPos = new QDoubleSpinBox();
        m_leftCamYResetPos->setRange(-100000.0, 100000.0);
        m_leftCamYResetPos->setDecimals(2);
        m_leftCamYResetPos->setValue(conf.left_cam_y_reset_pos_mm);
        formLayout->addRow("Left Camera Y Reset Position (mm):", m_leftCamYResetPos);

        m_rightCamXResetPos = new QDoubleSpinBox();
        m_rightCamXResetPos->setRange(-100000.0, 100000.0);
        m_rightCamXResetPos->setDecimals(2);
        m_rightCamXResetPos->setValue(conf.right_cam_x_reset_pos_mm);
        formLayout->addRow("Right Camera X Reset Position (mm):", m_rightCamXResetPos);

        m_rightCamYResetPos = new QDoubleSpinBox();
        m_rightCamYResetPos->setRange(-100000.0, 100000.0);
        m_rightCamYResetPos->setDecimals(2);
        m_rightCamYResetPos->setValue(conf.right_cam_y_reset_pos_mm);
        formLayout->addRow("Right Camera Y Reset Position (mm):", m_rightCamYResetPos);

        m_layout->addLayout(formLayout);
        m_layout->addStretch();
    }

    void CameraGeneralPage::pullDataToStruct(Kub3::Config::process_config_t &outConf) const
    {
        outConf.min_camera_distance_mm   = m_minCamDist->value();
        outConf.left_cam_x_reset_pos_mm  = m_leftCamXResetPos->value();
        outConf.left_cam_y_reset_pos_mm  = m_leftCamYResetPos->value();
        outConf.right_cam_x_reset_pos_mm = m_rightCamXResetPos->value();
        outConf.right_cam_y_reset_pos_mm = m_rightCamYResetPos->value();
    }

}
