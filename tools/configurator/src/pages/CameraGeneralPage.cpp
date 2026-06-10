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

    static QSpinBox *createSpinBox(int minLimit = 0, int maxLimit = 4095)
    {
        auto *sb = new QSpinBox();
        sb->setRange(minLimit, maxLimit);
        return sb;
    }

    CameraGeneralPage::CameraGeneralPage(const Kub3::Config::vision_process_config_t &conf, QWidget *parent) : QWidget(parent)
    {
        setupUI();
        loadInitialData(conf);
    }

    void CameraGeneralPage::setupUI()
    {
        m_layout = new QVBoxLayout(this);

        auto *header = new QLabel("<b>General Camera Settings</b>");
        header->setStyleSheet("font-size: 14px; margin-bottom: 5px;");
        m_layout->addWidget(header);

        auto *tabs = new QTabWidget();

        // --- TAB 1: Clearances & Reset Positions ---
        auto *positionsTab    = new QWidget();
        auto *positionsLayout = new QFormLayout(positionsTab);

        m_minCamDist = createDoubleSpinBox(0.0, 10000.0, 2);
        positionsLayout->addRow("Minimum Camera Distance (mm):", m_minCamDist);
        m_leftCamXResetPos = createDoubleSpinBox(-100000.0, 100000.0, 2);
        positionsLayout->addRow("Left Camera X Reset Position (mm):", m_leftCamXResetPos);
        m_leftCamYResetPos = createDoubleSpinBox(-100000.0, 100000.0, 2);
        positionsLayout->addRow("Left Camera Y Reset Position (mm):", m_leftCamYResetPos);
        m_rightCamXResetPos = createDoubleSpinBox(-100000.0, 100000.0, 2);
        positionsLayout->addRow("Right Camera X Reset Position (mm):", m_rightCamXResetPos);
        m_rightCamYResetPos = createDoubleSpinBox(-100000.0, 100000.0, 2);
        positionsLayout->addRow("Right Camera Y Reset Position (mm):", m_rightCamYResetPos);

        tabs->addTab(positionsTab, "Positions and Clearances");

        // --- TAB 2: Focal Tuning ---
        auto *focalsTab    = new QWidget();
        auto *focalsLayout = new QFormLayout(focalsTab);

        m_leftFocalMin     = createSpinBox(0, 4095);
        m_leftFocalMax     = createSpinBox(0, 4095);
        m_leftFocalDefault = createSpinBox(0, 4095);

        m_rightFocalMin     = createSpinBox(0, 4095);
        m_rightFocalMax     = createSpinBox(0, 4095);
        m_rightFocalDefault = createSpinBox(0, 4095);

        auto *leftLabel = new QLabel("<b>Left Camera Focal</b>");
        leftLabel->setStyleSheet("margin-top: 5px; margin-bottom: 5px;");
        focalsLayout->addRow(leftLabel);
        focalsLayout->addRow("Minimum Value:", m_leftFocalMin);
        focalsLayout->addRow("Maximum Value:", m_leftFocalMax);
        focalsLayout->addRow("Default Setup Value:", m_leftFocalDefault);

        auto *rightLabel = new QLabel("<b>Right Camera Focal</b>");
        rightLabel->setStyleSheet("margin-top: 15px; margin-bottom: 5px;");
        focalsLayout->addRow(rightLabel);
        focalsLayout->addRow("Minimum Value:", m_rightFocalMin);
        focalsLayout->addRow("Maximum Value:", m_rightFocalMax);
        focalsLayout->addRow("Default Setup Value:", m_rightFocalDefault);

        tabs->addTab(focalsTab, "Focal Constraints");

        m_layout->addWidget(tabs);

        // --- POKA-YOKE (Strict Dynamic Boundaries) ---
        // Minimum cannot exceed Maximum
        connect(m_leftFocalMin, &QSpinBox::valueChanged, m_leftFocalMax, &QSpinBox::setMinimum);
        connect(m_rightFocalMin, &QSpinBox::valueChanged, m_rightFocalMax, &QSpinBox::setMinimum);
        // Default value is strictly clamped between Minimum and Maximum
        connect(m_leftFocalMin, &QSpinBox::valueChanged, m_leftFocalDefault, &QSpinBox::setMinimum);
        connect(m_leftFocalMax, &QSpinBox::valueChanged, m_leftFocalDefault, &QSpinBox::setMaximum);
        connect(m_rightFocalMin, &QSpinBox::valueChanged, m_rightFocalDefault, &QSpinBox::setMinimum);
        connect(m_rightFocalMax, &QSpinBox::valueChanged, m_rightFocalDefault, &QSpinBox::setMaximum);
    }

    void CameraGeneralPage::loadInitialData(const Kub3::Config::vision_process_config_t &conf)
    {
        m_minCamDist->setValue(conf.min_camera_distance_mm);
        m_leftCamXResetPos->setValue(conf.left_cam_x_reset_pos_mm);
        m_leftCamYResetPos->setValue(conf.left_cam_y_reset_pos_mm);
        m_rightCamXResetPos->setValue(conf.right_cam_x_reset_pos_mm);
        m_rightCamYResetPos->setValue(conf.right_cam_y_reset_pos_mm);

        // Block signals locally to safely push the initial structural values without
        // triggering intermediate auto-clamping limits.
        const QSignalBlocker b1(m_leftFocalMin), b2(m_leftFocalMax), b3(m_leftFocalDefault);
        const QSignalBlocker b4(m_rightFocalMin), b5(m_rightFocalMax), b6(m_rightFocalDefault);

        m_leftFocalMin->setValue(conf.left_focal_conf.min_value);
        m_leftFocalMax->setValue(conf.left_focal_conf.max_value);
        m_leftFocalDefault->setValue(conf.left_focal_conf.default_value);

        m_rightFocalMin->setValue(conf.right_focal_conf.min_value);
        m_rightFocalMax->setValue(conf.right_focal_conf.max_value);
        m_rightFocalDefault->setValue(conf.right_focal_conf.default_value);

        // Explicitly set limits after setting the values
        m_leftFocalDefault->setRange(conf.left_focal_conf.min_value, conf.left_focal_conf.max_value);
        m_leftFocalMax->setMinimum(conf.left_focal_conf.min_value);

        m_rightFocalDefault->setRange(conf.right_focal_conf.min_value, conf.right_focal_conf.max_value);
        m_rightFocalMax->setMinimum(conf.right_focal_conf.min_value);
    }

    void CameraGeneralPage::pullDataToStruct(Kub3::Config::vision_process_config_t &out) const
    {
        out.min_camera_distance_mm   = m_minCamDist->value();
        out.left_cam_x_reset_pos_mm  = m_leftCamXResetPos->value();
        out.left_cam_y_reset_pos_mm  = m_leftCamYResetPos->value();
        out.right_cam_x_reset_pos_mm = m_rightCamXResetPos->value();
        out.right_cam_y_reset_pos_mm = m_rightCamYResetPos->value();

        out.left_focal_conf.min_value     = m_leftFocalMin->value();
        out.left_focal_conf.max_value     = m_leftFocalMax->value();
        out.left_focal_conf.default_value = m_leftFocalDefault->value();

        out.right_focal_conf.min_value     = m_rightFocalMin->value();
        out.right_focal_conf.max_value     = m_rightFocalMax->value();
        out.right_focal_conf.default_value = m_rightFocalDefault->value();
    }

} // namespace Kub3::Components
