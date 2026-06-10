#include <memory>

#include "ui_MachineStatusView.h"

#include <HAL/MachineStatus/sensors_labels.h>
#include <Views/Modules/CamerasTestView.h>
#include <Views/ViewsSettings/MachineStatusView.h>

#define SUCCESS_QLABEL_STYLESHEET "QLabel{ background: green; color : white; }"
#define FAILURE_QLABEL_STYLESHEET "QLabel{ background: red; color : white; }"

MachineStatusView::MachineStatusView(Shared<MachineStatusViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::MachineStatusView)
{
    ui->setupUi(this);

    m_showCentralLogo = false;
    configTitleBar();
    createNavButtonsConfigs();

    this->populateBoolSensorsMap();
    this->populateIntegerSensorsMap();
    this->populateUnsignedIntegerSensorsMap();

    Shared<MachineStatusViewModel> model = std::static_pointer_cast<MachineStatusViewModel>(m_viewModel);
    connect(model.get(), &MachineStatusViewModel::s_booleanSensorUpdate, this, &MachineStatusView::ps_booleanSensorUpdate);
    connect(model.get(), &MachineStatusViewModel::s_integerSensorUpdate, this, &MachineStatusView::ps_integerSensorUpdate);
    connect(model.get(), &MachineStatusViewModel::s_unsignedIntegerSensorUpdate, this, &MachineStatusView::ps_unsignedIntegerSensorUpdate);
}

MachineStatusView::~MachineStatusView()
{
    delete ui;
}

void MachineStatusView::ps_booleanSensorUpdate(const QString &sensorId, bool value)
{
    if (auto it = m_boolSensorsMap.find(sensorId); it != m_boolSensorsMap.end())
        this->updateBoolSensorsText(it->second, value);
}

void MachineStatusView::ps_integerSensorUpdate(const QString &sensorId, int32_t value)
{
    if (auto it = m_intSensorsMap.find(sensorId); it != m_intSensorsMap.end())
        this->updateIntSensorsText(it->second, value);
}

void MachineStatusView::ps_unsignedIntegerSensorUpdate(const QString &sensorId, uint32_t value)
{
    if (auto it = m_uintSensorsMap.find(sensorId); it != m_uintSensorsMap.end())
        this->updateUIntSensorsText(it->second, value);
}

void MachineStatusView::on_goBackBtn_clicked(void)
{
    emit s_home();
}

void MachineStatusView::on_openCamerasBtn_clicked(void)
{
    if (!m_viewModel)
        return;

    Modules::CamerasTestView modal(std::static_pointer_cast<MachineStatusViewModel>(m_viewModel), this);

    modal.exec();
}

void MachineStatusView::updateBoolSensorsText(QLabel *label, const bool state)
{
    const QString text = state ? "ON" : "OFF";

    label->setText(text);
    label->setStyleSheet(state ? SUCCESS_QLABEL_STYLESHEET : FAILURE_QLABEL_STYLESHEET);
}

void MachineStatusView::createNavButtonsConfigs()
{
    NavButtonConfig homeBtn(
        "Home",
        QColor("#0072BA"),
        QColor("#B2D4F4"),
        ":/icons/home.svg",
        "home",
        [this](const QString &) {
            // Retourner à HomeView
            emit s_openView(Kub3::UI::ViewId::HOME_VIEW);
        });
    addNavButton("left", homeBtn);
}

void MachineStatusView::configTitleBar()
{
    setTitleBar(TitleBarConfig{
        .viewTitle      = "Machine Status",
        .textColor      = QColor("#FFF"),
        .bgColor        = QColor("#0072BA"),
        .iconPath       = ":/icons/admin.svg",
        .sectionTitle   = "Settings",
        .showTitleBar   = true,
        .m_showLeftLogo = true,
    });
}

void MachineStatusView::updateIntSensorsText(QLabel *label, const int32_t value)
{
    label->setText(QString::number(value));
}

void MachineStatusView::updateUIntSensorsText(QLabel *label, const uint32_t value)
{
    label->setText(QString::number(value));
}

void MachineStatusView::populateBoolSensorsMap(void)
{
    // Wafer conveyor limits
    m_boolSensorsMap.emplace(CW0, ui->cw0Value);
    m_boolSensorsMap.emplace(CW1, ui->cw1Value);
    m_boolSensorsMap.emplace(CW2, ui->cw2Value);

    // Mask conveyor limits
    m_boolSensorsMap.emplace(CM0, ui->cm0Value);
    m_boolSensorsMap.emplace(CM1, ui->cm1Value);
    m_boolSensorsMap.emplace(CM2, ui->cm2Value);
    m_boolSensorsMap.emplace(CM3, ui->cm3Value);

    // Z-motors limits
    m_boolSensorsMap.emplace(Z_LEFT_LOW_LIMIT, ui->t2mkLeftLowValue);
    m_boolSensorsMap.emplace(Z_RIGHT_LOW_LIMIT, ui->t2mkRightLowValue);
    m_boolSensorsMap.emplace(Z_BACK_LOW_LIMIT, ui->t2mkBackLowValue);
    m_boolSensorsMap.emplace(Z_LEFT_HIGH_LIMIT, ui->t2mkLeftHighValue);
    m_boolSensorsMap.emplace(Z_RIGHT_HIGH_LIMIT, ui->t2mkRightHighValue);
    m_boolSensorsMap.emplace(Z_BACK_HIGH_LIMIT, ui->t2mkBackHighValue);

    // Stowage zone limits
    m_boolSensorsMap.emplace(Z1, ui->_3Z1Value);
    m_boolSensorsMap.emplace(WAFER_ON, ui->waferOnValue);
    m_boolSensorsMap.emplace(Z2, ui->_3Z2Value);

    // ARDKO limits
    m_boolSensorsMap.emplace(ARDKO_BACK_LEFT_LIMIT, ui->ardkoBackLeftValue);
    m_boolSensorsMap.emplace(ARDKO_BACK_RIGHT_LIMIT, ui->ardkoBackRightValue);
    m_boolSensorsMap.emplace(ARDKO_FRONT_LEFT_LIMIT, ui->ardkoFrontLeftValue);
    m_boolSensorsMap.emplace(ARDKO_FRONT_RIGHT_LIMIT, ui->ardkoFrontRightValue);

    // Solenoid valves state (opened/closed)
    // TODO: complete valve states
    // m_boolSensorsMap.emplace(MASK_VACUUM_VALVE_STATUS, ui->);
    // m_boolSensorsMap.emplace(WAFER_VACUUM_VALVE_STATUS, ui->);
    m_boolSensorsMap.emplace(WAFER_COMPRESSED_AIR_VALVE_STATUS, ui->waferCompressedAirValveValue);

    // Pressure thresholds
    m_boolSensorsMap.emplace(MASK_VACUUM_ACTIVE, ui->maskVacuumValue);
    m_boolSensorsMap.emplace(WAFER_VACUUM_ACTIVE, ui->waferVacuumValue);
    m_boolSensorsMap.emplace(WAFER_COMPRESSED_AIR_ACTIVE, ui->waferCompressedAirValue);

    // Alignment stages limits
    m_boolSensorsMap.emplace(X_STAGE_LEFT_LIMIT, ui->waferXLeftValue);
    m_boolSensorsMap.emplace(X_STAGE_RIGHT_LIMIT, ui->waferXRightValue);
    m_boolSensorsMap.emplace(Y_STAGE_BACK_LIMIT, ui->waferYBackValue);
    m_boolSensorsMap.emplace(Y_STAGE_FRONT_LIMIT, ui->waferYFrontValue);
    m_boolSensorsMap.emplace(THETA_STAGE_ANTI_CLOCKWISE_LIMIT, ui->waferThetaCCWValue);
    m_boolSensorsMap.emplace(THETA_STAGE_CLOCKWISE_LIMIT, ui->waferThetaCWValue);

    // Cameras limits
    m_boolSensorsMap.emplace(LEFT_CAMERA_X_LEFT_LIMIT, ui->leftCameraXLeftLimitValue);
    // m_boolSensorsMap.emplace(LEFT_CAMERA_X_RIGHT_LIMIT, );
    m_boolSensorsMap.emplace(LEFT_CAMERA_Y_FRONT_LIMIT, ui->leftCameraYFrontLimitValue);
    // m_boolSensorsMap.emplace(LEFT_CAMERA_Y_BACK_LIMIT, );
    // m_boolSensorsMap.emplace(RIGHT_CAMERA_X_LEFT_LIMIT, );
    m_boolSensorsMap.emplace(RIGHT_CAMERA_X_RIGHT_LIMIT, ui->rightCameraXRightLimitValue);
    m_boolSensorsMap.emplace(RIGHT_CAMERA_Y_FRONT_LIMIT, ui->rightCameraYFrontLimitValue);
    // m_boolSensorsMap.emplace(RIGHT_CAMERA_Y_BACK_LIMIT, );

    // Cameras' deck limits
    m_boolSensorsMap.emplace(DECK_BACK_LIMIT, ui->deckBackLimitValue);
    m_boolSensorsMap.emplace(DECK_FRONT_LIMIT, ui->deckFrontLimitValue);
}

void MachineStatusView::populateIntegerSensorsMap(void)
{
    // Encoders
    m_intSensorsMap.emplace(WAFER_ENCODER, ui->waferPosValue);
    m_intSensorsMap.emplace(MASK_ENCODER, ui->maskPosValue);
    m_intSensorsMap.emplace(Z_LEFT_ENCODER, ui->leftZPosValue);
    m_intSensorsMap.emplace(Z_RIGHT_ENCODER, ui->rightZPosValue);
    m_intSensorsMap.emplace(Z_BACK_ENCODER, ui->backZPosValue);
    m_intSensorsMap.emplace(X_STAGE_ENCODER, ui->xStagePosValue);
    m_intSensorsMap.emplace(Y_STAGE_ENCODER, ui->yStagePosValue);
    m_intSensorsMap.emplace(THETA_STAGE_ENCODER, ui->thetaStagePosValue);
    m_intSensorsMap.emplace(LEFT_CAMERA_X_ENCODER, ui->leftCamXPosValue);
    m_intSensorsMap.emplace(LEFT_CAMERA_Y_ENCODER, ui->leftCamYPosValue);
    m_intSensorsMap.emplace(RIGHT_CAMERA_X_ENCODER, ui->rightCamXPosValue);
    m_intSensorsMap.emplace(RIGHT_CAMERA_Y_ENCODER, ui->rightCamYPosValue);

    // Temperatures
    m_intSensorsMap.emplace(INTERNAL_TEMPERATURE, ui->internalTempValue);
    m_intSensorsMap.emplace(EXTERNAL_TEMPERATURE, ui->externalTempValue);
}

void MachineStatusView::populateUnsignedIntegerSensorsMap(void)
{
    m_uintSensorsMap.emplace(FORCE_LEFT, ui->forceLeftValue);
    m_uintSensorsMap.emplace(FORCE_RIGHT, ui->forceRightValue);
    m_uintSensorsMap.emplace(FORCE_BACK, ui->forceBackValue);
}
