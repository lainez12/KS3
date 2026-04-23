#include <memory>

#include "ui_MachineStatusView.h"

#include <HAL/MachineStatus/sensors_labels.h>
#include <Views/MachineStatusView.h>
#include <Views/Modules/CamerasTestView.h>

#define SUCCESS_QLABEL_STYLESHEET "QLabel{ background: green; color : white; }"
#define FAILURE_QLABEL_STYLESHEET "QLabel{ background: red; color : white; }"

MachineStatusView::MachineStatusView(Shared<MachineStatusViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::MachineStatusView)
{
    ui->setupUi(this);

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
    if (auto it = m_boolSensorsMap.find(sensorId.toUtf8().constData()); it != m_boolSensorsMap.end())
        this->updateBoolSensorsText(it->second, value);
}

void MachineStatusView::ps_integerSensorUpdate(const QString &sensorId, int32_t value)
{
    if (auto it = m_intSensorsMap.find(sensorId.toUtf8().constData()); it != m_intSensorsMap.end())
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
    m_boolSensorsMap.insert({CW0, ui->cw0Value});
    m_boolSensorsMap.insert({CW1, ui->cw1Value});
    m_boolSensorsMap.insert({CW2, ui->cw2Value});

    // Mask conveyor limits
    m_boolSensorsMap.insert({CM0, ui->cm0Value});
    m_boolSensorsMap.insert({CM1, ui->cm1Value});
    m_boolSensorsMap.insert({CM2, ui->cm2Value});
    m_boolSensorsMap.insert({CM3, ui->cm3Value});

    // Z-motors limits
    m_boolSensorsMap.insert({Z_LEFT_LOW_LIMIT, ui->t2mkLeftLowValue});
    m_boolSensorsMap.insert({Z_RIGHT_LOW_LIMIT, ui->t2mkRightLowValue});
    m_boolSensorsMap.insert({Z_BACK_LOW_LIMIT, ui->t2mkBackLowValue});
    m_boolSensorsMap.insert({Z_LEFT_HIGH_LIMIT, ui->t2mkLeftHighValue});
    m_boolSensorsMap.insert({Z_RIGHT_HIGH_LIMIT, ui->t2mkRightHighValue});
    m_boolSensorsMap.insert({Z_BACK_HIGH_LIMIT, ui->t2mkBackHighValue});

    // Stowage zone limits
    m_boolSensorsMap.insert({Z1, ui->_3Z1Value});
    m_boolSensorsMap.insert({WAFER_ON, ui->waferOnValue});
    m_boolSensorsMap.insert({Z2, ui->_3Z2Value});

    // ARDKO limits
    m_boolSensorsMap.insert({ARDKO_BACK_LEFT_LIMIT, ui->ardkoBackLeftValue});
    m_boolSensorsMap.insert({ARDKO_BACK_RIGHT_LIMIT, ui->ardkoBackRightValue});
    m_boolSensorsMap.insert({ARDKO_FRONT_LEFT_LIMIT, ui->ardkoFrontLeftValue});
    m_boolSensorsMap.insert({ARDKO_FRONT_RIGHT_LIMIT, ui->ardkoFrontRightValue});

    // Solenoid valves state (opened/closed)
    // TODO: complete valve states
    // m_boolSensorsMap.insert({MASK_VACUUM_VALVE_STATUS, ui->});
    // m_boolSensorsMap.insert({WAFER_VACUUM_VALVE_STATUS, ui->});
    m_boolSensorsMap.insert({WAFER_COMPRESSED_AIR_VALVE_STATUS, ui->waferCompressedAirValveValue});

    // Pressure thresholds
    m_boolSensorsMap.insert({MASK_VACUUM_ACTIVE, ui->maskVacuumValue});
    m_boolSensorsMap.insert({WAFER_VACUUM_ACTIVE, ui->waferVacuumValue});
    m_boolSensorsMap.insert({WAFER_COMPRESSED_AIR_ACTIVE, ui->waferCompressedAirValue});

    // Alignment stages limits
    m_boolSensorsMap.insert({X_STAGE_LEFT_LIMIT, ui->waferXLeftValue});
    m_boolSensorsMap.insert({X_STAGE_RIGHT_LIMIT, ui->waferXRightValue});
    m_boolSensorsMap.insert({Y_STAGE_BACK_LIMIT, ui->waferYBackValue});
    m_boolSensorsMap.insert({Y_STAGE_FRONT_LIMIT, ui->waferYFrontValue});
    m_boolSensorsMap.insert({THETA_STAGE_ANTI_CLOCKWISE_LIMIT, ui->waferThetaCCWValue});
    m_boolSensorsMap.insert({THETA_STAGE_CLOCKWISE_LIMIT, ui->waferThetaCWValue});

    // Cameras limits
    m_boolSensorsMap.insert({LEFT_CAMERA_X_LEFT_LIMIT, ui->leftCameraXLeftLimitValue});
    // m_boolSensorsMap.insert({LEFT_CAMERA_X_RIGHT_LIMIT, });
    m_boolSensorsMap.insert({LEFT_CAMERA_Y_FRONT_LIMIT, ui->leftCameraYFrontLimitValue});
    // m_boolSensorsMap.insert({LEFT_CAMERA_Y_BACK_LIMIT, });
    // m_boolSensorsMap.insert({RIGHT_CAMERA_X_LEFT_LIMIT, });
    m_boolSensorsMap.insert({RIGHT_CAMERA_X_RIGHT_LIMIT, ui->rightCameraYFrontLimitValue});
    m_boolSensorsMap.insert({RIGHT_CAMERA_Y_FRONT_LIMIT, ui->rightCameraYFrontLimitValue});
    // m_boolSensorsMap.insert({RIGHT_CAMERA_Y_BACK_LIMIT, });

    // Cameras' deck limits
    m_boolSensorsMap.insert({DECK_BACK_LIMIT, ui->deckBackLimitValue});
    m_boolSensorsMap.insert({DECK_FRONT_LIMIT, ui->deckFrontLimitValue});
}

void MachineStatusView::populateIntegerSensorsMap(void)
{}

void MachineStatusView::populateUnsignedIntegerSensorsMap(void)
{
    m_uintSensorsMap.insert({FORCE_LEFT, ui->forceLeftValue});
    m_uintSensorsMap.insert({FORCE_RIGHT, ui->forceRightValue});
    m_uintSensorsMap.insert({FORCE_BACK, ui->forceBackValue});
}
