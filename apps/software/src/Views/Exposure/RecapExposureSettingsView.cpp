#include "ui_RecapExposureSettingsView.h"
#include <QString>
#include <Views/Components/Colors.h>
#include <Views/Exposure/RecapExposureSettingsView.h>

#define ID_BTN_BACK     "B"
#define ID_BTN_SAVE     "S"
#define ID_BTN_VALIDATE "V"

RecapExposureSettingsView::RecapExposureSettingsView(Unique<RecapExposureSettingsViewModel> viewModel, QWidget *parent) :
    ExposureViewBase(std::move(viewModel), parent),
    ui(new Ui::RecapExposureSettingsView)
{
    ui->setupUi(this);

    createNavButtonsConfigs();
    setNewNavButtonsConfigs();
    setDefaultTitleBar("Exposure Settings");
    setNavButtonEnabled(ID_BTN_VALIDATE, true);
}
RecapExposureSettingsView::~RecapExposureSettingsView()
{
}

void RecapExposureSettingsView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void RecapExposureSettingsView::showEvent(QShowEvent *event)
{
    auto vm = getViewModel<RecapExposureSettingsViewModel>();
    if (vm && vm->isPresetSetAndValid())
    {
        PresetExposure preset = vm->getCurrentPreset();
        ui->modeExposureLabel->setText(vm->modeToString(preset.mode) + " exposure");
        ui->detailsExposure->setText(vm->presetDetailsToStr(preset));
    }
    QWidget::showEvent(event);
}

void RecapExposureSettingsView::setNewNavButtonsConfigs()
{
    NavButtonConfig backBtn(
        "Back",
        ":/icons/back.svg",
        ID_BTN_BACK,
        std::bind(&RecapExposureSettingsView::onBackButtonClicked, this));

    addNavButton("left", backBtn);
}

void RecapExposureSettingsView::onBackButtonClicked(void)
{
    emit s_openView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW);
}

void RecapExposureSettingsView::onValidateButtonClicked(void)
{
    getViewModel<RecapExposureSettingsViewModel>()->ui_requestLaunchExposure();
    emit s_openView(Kub3::UI::ViewId::PROGRESS_EXPOSURE_VIEW);
}