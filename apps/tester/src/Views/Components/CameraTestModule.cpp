#include "ui_CameraTestModule.h"

#include <QCheckBox>
#include <Views/Components/CameraTestModule.h>

namespace
{
    struct ComboBoxItem {
        const char *text;
        double value;
    };

    const ComboBoxItem __framerates[] = {
        {"24 Hz", 24.0},
        {"25 Hz", 25.0},
        {"30 Hz", 30.0},
        {"48 Hz", 48.0},
        {"50 Hz", 50.0},
        {"60 Hz", 60.0},
    };

    const ComboBoxItem __centeredZooms[] = {
        {"1x Zoom (Full Sensor, 100%)", 1.0},
        {"2x Zoom (Center 50%)", 2.0},
        {"4x Zoom (Center 25%)", 4.0},
    };
}

namespace Kub3::Tools::Tester
{
    CameraTestModule::CameraTestModule(const QString &cameraId,
                                       const QString &title,
                                       const Optional<QString> &focalId,
                                       const Optional<QString> &lightId,
                                       QWidget *parent) :
        QWidget(parent),
        ui(new Ui::CameraTestModule),
        m_cameraId(cameraId),
        m_focalId(focalId),
        m_lightId(lightId)
    {
        ui->setupUi(this);
        ui->cameraTitle->setText(title);

        setupComponents();

        // Wiring standard camera components
        connect(ui->exposureSlider, &DoubleRatioSlider::doubleValueChanged, this, [this](double v) { emit s_exposureChanged(m_cameraId, v); });
        connect(ui->gainSlider, &DoubleRatioSlider::doubleValueChanged, this, [this](double v) { emit s_gainChanged(m_cameraId, v); });

        connect(ui->framerateBox, &QComboBox::currentIndexChanged, this, [this](int idx) {
            emit s_framerateChanged(m_cameraId, ui->framerateBox->itemData(idx).toDouble());
        });

        connect(ui->zoomBox, &QComboBox::currentIndexChanged, this, [this](int idx) {
            emit s_zoomChanged(m_cameraId, ui->zoomBox->itemData(idx).toDouble());
        });

        // ROI wiring
        connect(ui->roiXBox, &QSpinBox::valueChanged, this, &CameraTestModule::onROIFieldChanged);
        connect(ui->roiYBox, &QSpinBox::valueChanged, this, &CameraTestModule::onROIFieldChanged);
        connect(ui->roiWBox, &QSpinBox::valueChanged, this, &CameraTestModule::onROIFieldChanged);
        connect(ui->roiHBox, &QSpinBox::valueChanged, this, &CameraTestModule::onROIFieldChanged);

        // Focal Wiring
        if (!m_focalId)
        {
            ui->focalEnableCheck->hide();
            ui->focalSlider->hide();
            ui->focalValueLabel->hide();
        }
        else
        {
            connect(ui->focalEnableCheck, &QCheckBox::toggled, this, [this](bool checked) {
                ui->focalSlider->setEnabled(checked);
                emit s_focalToggled(m_focalId.value(), checked);
            });
            connect(ui->focalSlider, &DoubleRatioSlider::doubleValueChanged, this, [this](double v) {
                ui->focalValueLabel->setText(QString::number(static_cast<uint8_t>(v * 100.0)) + "%");
                emit s_focalChanged(m_focalId.value(), v);
            });
        }
        // Light Wiring
        if (!m_lightId)
        {
            ui->lightEnableCheck->hide();
            ui->lightSlider->hide();
            ui->lightValueLabel->hide();
        }
        else
        {
            connect(ui->lightEnableCheck, &QCheckBox::toggled, this, [this](bool checked) {
                ui->lightSlider->setEnabled(checked);
                emit s_lightToggled(m_lightId.value(), checked);
            });
            connect(ui->lightSlider, &DoubleRatioSlider::doubleValueChanged, this, [this](double v) {
                ui->lightValueLabel->setText(QString::number(static_cast<uint8_t>(v * 100.0)) + "%");
                emit s_lightChanged(m_lightId.value(), v);
            });
        }
    }

    CameraTestModule::~CameraTestModule()
    {
        delete ui;
    }

    CameraStreamWidget *CameraTestModule::streamWidget() const
    {
        return ui->streamWidget;
    }

    void CameraTestModule::setupComponents()
    {
        for (const auto &it : __framerates)
            ui->framerateBox->addItem(QString::fromUtf8(it.text), QVariant(it.value));

        for (const auto &it : __centeredZooms)
            ui->zoomBox->addItem(QString::fromUtf8(it.text), QVariant(it.value));
    }

    void CameraTestModule::onROIFieldChanged()
    {
        emit s_roiChanged(m_cameraId, QRect(ui->roiXBox->value(), ui->roiYBox->value(), ui->roiWBox->value(), ui->roiHBox->value()));
    }
}
