#include <views/components/CamLightTestModule.h>

#include "ui_CamLightTestModule.h"

namespace Kub3::Tools::Tester
{

    CamLightTestModule::CamLightTestModule(const QString &lightId, QWidget *parent) :
        QWidget(parent),
        ui(std::make_unique<Ui::CamLightTestModule>()),
        m_lightId(lightId)
    {
        ui->setupUi(this);

        ui->camLightName->setText(lightId);
        ui->camLightValue->setText("0");

        // Slider value update
        connect(ui->slider, &QSlider::valueChanged, this, [this](int value) {
            ui->camLightValue->setText(QString::number(value));
            emit s_valueChanged(m_lightId, static_cast<uint16_t>(value));
        });
        // Enable state update
        connect(ui->chkEnable, &QCheckBox::toggled, this, [this](bool checked) {
            ui->slider->setEnabled(checked);
            emit s_toggled(m_lightId, checked);
        });
    }

    CamLightTestModule::~CamLightTestModule() = default;

} // namespace Kub3::Tools::Tester
