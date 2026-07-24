#include "ui_ProgressExposureView.h"
#include <QString>
#include <Views/Components/Colors.h>
#include <Views/Exposure/ProgressExposureView.h>

#define ID_BTN_BACK     "B"
#define ID_BTN_SAVE     "S"
#define ID_BTN_VALIDATE "V"

ProgressExposureView::ProgressExposureView(Unique<ProgressExposureViewModel> viewModel, QWidget *parent) :
    ExposureViewBase(std::move(viewModel), parent),
    ui(new Ui::ProgressExposureView)
{
    ui->setupUi(this);

    createNavButtonsConfigs();
    setDefaultTitleBar("Exposure in progress");
    setNavButtonEnabled(ID_BTN_VALIDATE, true);
}
ProgressExposureView::~ProgressExposureView()
{
}

void ProgressExposureView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void ProgressExposureView::showEvent(QShowEvent *event)
{
    auto vm = getViewModel<ProgressExposureViewModel>();
    if (vm)
    {
        PresetExposure preset = vm->getCurrentPreset();
        ui->exposureModeLabel->setText(vm->modeToString(preset.mode) + " exposure");
        ui->detailsExposureLabel->setText(vm->presetDetailsToStr(preset));

        qDebug() << "Preset details: " << vm->presetDetailsToStr(preset);
    }
    ui->progressBar->setValue(0);
    QWidget::showEvent(event);
}

void ProgressExposureView::onBackButtonClicked()
{
    // emit s_openView(Kub3::UI::ViewId::BACK_VIEW);
}

void ProgressExposureView::onValidateButtonClicked()
{
    emit s_openView(Kub3::UI::ViewId::COMPLETE_EXPOSURE_VIEW);
}