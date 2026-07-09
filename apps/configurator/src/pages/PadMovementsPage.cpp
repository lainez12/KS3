#include <QGroupBox>

#include <pages/PadMovementsPage.h>

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

    PadMovementsPage::PadMovementsPage(const Kub3::Config::pad_process_config_t &conf, QWidget *parent) : QWidget(parent)
    {
        setupUI();
        loadInitialData(conf);
    }

    void PadMovementsPage::setupUI()
    {
        m_layout = new QVBoxLayout(this);

        auto *header = new QLabel("<b>Steps for short pushes on the pad</b>");
        header->setStyleSheet("font-size: 14px; margin-bottom: 15px;");
        m_layout->addWidget(header);

        m_tabs = new QTabWidget();

        {
            auto alignmentAndZPage = new QWidget(this);
            auto formLayout        = new QFormLayout(alignmentAndZPage);

            m_xStageDistanceMm     = createDoubleSpinBox(0.0, 10.0, 4);
            m_yStageDistanceMm     = createDoubleSpinBox(0.0, 10.0, 4);
            m_thetaStageDistanceMm = createDoubleSpinBox(0.0, 10.0, 4);
            m_zMotorsDistanceMm    = createDoubleSpinBox(0.0, 10.0, 4);

            formLayout->addRow("X alignment stage movement (mm):", m_xStageDistanceMm);
            formLayout->addRow("Y alignment stage movement (mm):", m_yStageDistanceMm);
            formLayout->addRow("Theta alignment stage movement (mm):", m_thetaStageDistanceMm);
            formLayout->addRow("Z elevators movement (mm):", m_zMotorsDistanceMm);

            m_tabs->addTab(alignmentAndZPage, "Alignment / Z-axis Movements");
        }

        {
            auto camerasSettingsPage = new QWidget(this);
            auto formLayout          = new QFormLayout(camerasSettingsPage);

            m_leftCamLight  = createDoubleSpinBox(0.0, 100.0, 1);
            m_rightCamLight = createDoubleSpinBox(0.0, 100.0, 1);
            m_leftCamFocal  = createDoubleSpinBox(0.0, 100.0, 1);
            m_rightCamFocal = createDoubleSpinBox(0.0, 100.0, 1);

            formLayout->addRow("Left camera lighting step (percentage):", m_leftCamLight);
            formLayout->addRow("Right camera lighting step (percentage)::", m_rightCamLight);
            formLayout->addRow("Left camera focal step (percentage):", m_leftCamFocal);
            formLayout->addRow("Right camera focal step (percentage)::", m_rightCamFocal);

            m_tabs->addTab(camerasSettingsPage, "Cameras Lighting / Focus");
        }

        {
            auto camerasPage = new QWidget(this);
            auto formLayout  = new QFormLayout(camerasPage);

            m_leftCamXDistanceMm  = createDoubleSpinBox(0.0, 10.0, 4);
            m_rightCamXDistanceMm = createDoubleSpinBox(0.0, 10.0, 4);
            m_leftCamYDistanceMm  = createDoubleSpinBox(0.0, 10.0, 4);
            m_rightCamYDistanceMm = createDoubleSpinBox(0.0, 10.0, 4);

            formLayout->addRow("Left camera X movement (mm):", m_leftCamXDistanceMm);
            formLayout->addRow("Left camera Y movement (mm):", m_leftCamYDistanceMm);
            formLayout->addRow("Right camera X movement (mm):", m_rightCamXDistanceMm);
            formLayout->addRow("Right camera Y movement (mm):", m_rightCamYDistanceMm);

            m_tabs->addTab(camerasPage, "Cameras Movements");
        }
        m_layout->addWidget(m_tabs);

        m_layout->addStretch();
    }

    void PadMovementsPage::loadInitialData(const Kub3::Config::pad_process_config_t &conf)
    {
        // Load the values
        // --- Cameras (movements)
        m_leftCamXDistanceMm->setValue(conf.left_cam_x_distance_mm);
        m_rightCamXDistanceMm->setValue(conf.right_cam_x_distance_mm);
        m_leftCamYDistanceMm->setValue(conf.left_cam_y_distance_mm);
        m_rightCamYDistanceMm->setValue(conf.right_cam_y_distance_mm);
        // --- Cameras (lighting and focus)
        m_leftCamLight->setValue(conf.left_cam_x_distance_mm);
        m_rightCamLight->setValue(conf.right_cam_x_distance_mm);
        m_leftCamFocal->setValue(conf.left_cam_y_distance_mm);
        m_rightCamFocal->setValue(conf.right_cam_y_distance_mm);
        // --- Alignment / Z-axis
        m_xStageDistanceMm->setValue(conf.x_stage_distance_mm);
        m_yStageDistanceMm->setValue(conf.y_stage_distance_mm);
        m_thetaStageDistanceMm->setValue(conf.theta_stage_distance_mm);
        m_zMotorsDistanceMm->setValue(conf.z_motors_distance_mm);
    }

    void PadMovementsPage::pullDataToStruct(Kub3::Config::pad_process_config_t &outConf) const
    {
        // --- Cameras
        outConf.left_cam_x_distance_mm  = m_leftCamXDistanceMm->value();
        outConf.right_cam_x_distance_mm = m_rightCamXDistanceMm->value();
        outConf.left_cam_y_distance_mm  = m_leftCamYDistanceMm->value();
        outConf.right_cam_y_distance_mm = m_rightCamYDistanceMm->value();
        // --- Alignment / Z-axis
        outConf.x_stage_distance_mm     = m_xStageDistanceMm->value();
        outConf.y_stage_distance_mm     = m_yStageDistanceMm->value();
        outConf.theta_stage_distance_mm = m_thetaStageDistanceMm->value();
        outConf.z_motors_distance_mm    = m_zMotorsDistanceMm->value();
    }
}
