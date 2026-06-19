#include <format>

#include <pages/CameraConfigPage.h>
#include <utils.h>

#include "ui_CameraConfigPage.h"

namespace Kub3::Components
{
    CameraConfigPage::CameraConfigPage(const Kub3::Config::camera_config_t &conf, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::CameraConfigPage)
    {
        ui->setupUi(this);
        setupUI(conf);
        loadInitialData(conf);
    }

    CameraConfigPage::~CameraConfigPage()
    {
        delete ui;
    }

    void CameraConfigPage::setupUI(const Kub3::Config::camera_config_t &conf)
    {
        ui->headerLabel->setText(QString("<b>Configuration for Camera: %1</b>").arg(Kub3::camelToNormal(conf.id)));

        const QList<double> fpsOptions = {24, 25, 30, 48, 50, 60};
        for (double fps : fpsOptions)
        {
            ui->framerateCombo->addItem(QString("%1 FPS").arg(fps), QVariant::fromValue(fps));
        }

        // --- MISTAKE PROOFING ---
        // Dynamically clamp the 'Default' spinboxes so they can never exceed the 'Maximum' spinboxes.
        connect(ui->maxExposureSpin, &QDoubleSpinBox::valueChanged, ui->defaultExposureSpin, &QDoubleSpinBox::setMaximum);
        connect(ui->maxGainSpin, &QDoubleSpinBox::valueChanged, ui->defaultGainSpin, &QDoubleSpinBox::setMaximum);
    }

    void CameraConfigPage::loadInitialData(const Kub3::Config::camera_config_t &conf)
    {
        // Set maximums first to avoid Qt automatically clamping default values during initialization
        ui->maxExposureSpin->setValue(conf.maxExposureUs);
        ui->maxGainSpin->setValue(conf.maxGainDb);

        // Apply clamping limits to the default boxes
        ui->defaultExposureSpin->setMaximum(conf.maxExposureUs);
        ui->defaultGainSpin->setMaximum(conf.maxGainDb);

        // Set actual default values
        ui->defaultExposureSpin->setValue(conf.defaultExposureUs);
        ui->defaultGainSpin->setValue(conf.defaultGainDb);

        ui->serialNumberEdit->setText(QString::fromStdString(conf.serialNumber));

        // Load optionals: fallback to empty string if nullopt
        ui->focalIdEdit->setText(conf.associatedFocalId.value_or(""));
        ui->lightIdEdit->setText(conf.associatedLightId.value_or(""));

        if (int idx = ui->framerateCombo->findData(conf.framerate); idx != -1)
            ui->framerateCombo->setCurrentIndex(idx);
        else
        {
            // Failsafe: If the .ini has an unusual custom framerate
            ui->framerateCombo->addItem(QString("%1 FPS (Custom)").arg(conf.framerate), conf.framerate);
            ui->framerateCombo->setCurrentIndex(ui->framerateCombo->count() - 1);
        }
    }

    void CameraConfigPage::pullDataToStruct(Kub3::Config::camera_config_t &outConf) const
    {
        // Note: outConf.id is strictly preserved by NOT overwriting it here.
        outConf.serialNumber      = ui->serialNumberEdit->text().toStdString();
        outConf.maxExposureUs     = ui->maxExposureSpin->value();
        outConf.defaultExposureUs = ui->defaultExposureSpin->value();
        outConf.maxGainDb         = ui->maxGainSpin->value();
        outConf.defaultGainDb     = ui->defaultGainSpin->value();
        outConf.framerate         = ui->framerateCombo->currentData().toDouble();

        // Optionals parsing: If line edit is empty, resolve to std::nullopt
        QString fId               = ui->focalIdEdit->text().trimmed();
        outConf.associatedFocalId = fId.isEmpty() ? std::nullopt : std::make_optional(fId);
        QString lId               = ui->lightIdEdit->text().trimmed();
        outConf.associatedLightId = lId.isEmpty() ? std::nullopt : std::make_optional(lId);
    }

} // namespace Kub3::Components
