#include <views/components/FocalTestModule.h>

#include "ui_FocalTestModule.h"

namespace Kub3::Tools::Tester
{

    FocalTestModule::FocalTestModule(const QString &focalId, QWidget *parent) :
        QWidget(parent),
        ui(std::make_unique<Ui::FocalTestModule>()),
        m_focalId(focalId)
    {
        ui->setupUi(this);

        ui->focalName->setText(focalId);
        ui->focalValue->setText("0");

        // Slider value update
        connect(ui->slider, &QSlider::valueChanged, this, [this](int value) {
            ui->focalValue->setText(QString::number(value));
            emit s_valueChanged(m_focalId, static_cast<uint16_t>(value));
        });
        // Enable state update
        connect(ui->chkEnable, &QCheckBox::toggled, this, [this](bool checked) {
            ui->slider->setEnabled(checked);
            emit s_toggled(m_focalId, checked);
        });
    }

    FocalTestModule::~FocalTestModule() = default;

} // namespace Kub3::Tools::Tester
