#include <pages/CameraGeneralPage.h>

namespace Kub3::Components
{
    static QDoubleSpinBox *createDoubleSpinBox(double minLimit = 0.0, double maxLimit = 10000.0, int decimals = 2)
    {
        auto *sb = new QDoubleSpinBox();

        sb->setRange(minLimit, maxLimit);
        sb->setDecimals(decimals);
        sb->setSingleStep(std::min((maxLimit - minLimit) / 100.0, 1.0));
        return sb;
    }

    CameraGeneralPage::CameraGeneralPage(const Kub3::Config::vision_process_config_t &conf, QWidget *parent) : QWidget(parent)
    {
        m_layout = new QVBoxLayout(this);

        auto *header = new QLabel("<b>General Camera Settings</b>");
        header->setStyleSheet("font-size: 14px; margin-bottom: 15px;");
        m_layout->addWidget(header);

        auto *formLayout = new QFormLayout();

        m_minCamDist = createDoubleSpinBox(0.0, 10000.0, 2);
        m_minCamDist->setValue(conf.min_camera_distance_mm);
        formLayout->addRow("Minimum Camera Distance (mm):", m_minCamDist);

        m_leftCamXResetPos = createDoubleSpinBox(-100000.0, 100000.0, 2);
        m_leftCamXResetPos->setValue(conf.left_cam_x_reset_pos_mm);
        formLayout->addRow("Left Camera X Reset Position (mm):", m_leftCamXResetPos);

        m_leftCamYResetPos = createDoubleSpinBox(-100000.0, 100000.0, 2);
        m_leftCamYResetPos->setValue(conf.left_cam_y_reset_pos_mm);
        formLayout->addRow("Left Camera Y Reset Position (mm):", m_leftCamYResetPos);

        m_rightCamXResetPos = createDoubleSpinBox(-100000.0, 100000.0, 2);
        m_rightCamXResetPos->setValue(conf.right_cam_x_reset_pos_mm);
        formLayout->addRow("Right Camera X Reset Position (mm):", m_rightCamXResetPos);

        m_rightCamYResetPos = createDoubleSpinBox(-100000.0, 100000.0, 2);
        m_rightCamYResetPos->setValue(conf.right_cam_y_reset_pos_mm);
        formLayout->addRow("Right Camera Y Reset Position (mm):", m_rightCamYResetPos);

        m_layout->addLayout(formLayout);
        m_layout->addStretch();
    }

    void CameraGeneralPage::pullDataToStruct(Kub3::Config::vision_process_config_t &out) const
    {
        out.min_camera_distance_mm   = m_minCamDist->value();
        out.left_cam_x_reset_pos_mm  = m_leftCamXResetPos->value();
        out.left_cam_y_reset_pos_mm  = m_leftCamYResetPos->value();
        out.right_cam_x_reset_pos_mm = m_rightCamXResetPos->value();
        out.right_cam_y_reset_pos_mm = m_rightCamYResetPos->value();
    }

}
