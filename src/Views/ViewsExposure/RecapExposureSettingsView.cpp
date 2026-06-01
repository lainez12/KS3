#include "Views/ViewsExposure/RecapExposureSettingsView.h"
#include "Views/Components/Colors.h"
#include "ui_RecapExposureSettingsView.h"
#include <QString>

#define ID_BTN_BACK     "B"
#define ID_BTN_SAVE     "S"
#define ID_BTN_VALIDATE "V"

RecapExposureSettingsView::RecapExposureSettingsView(Unique<RecapExposureSettingsViewModel> viewModel, QWidget *parent) :
    ExposureViewBase(std::move(viewModel), parent),
    ui(new Ui::RecapExposureSettingsView) {
    ui->setupUi(this);

    createNavButtonsConfigs();
    setNewNavButtonsConfigs();
    setDefaultTitleBar("Exposure Settings");
    setNavButtonEnabled(ID_BTN_VALIDATE, true);
}
RecapExposureSettingsView::~RecapExposureSettingsView() {
}

void RecapExposureSettingsView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}

void RecapExposureSettingsView::setNewNavButtonsConfigs() {

    NavButtonConfig backBtn(
        "Back",
        QColor(BLUE_COLOR),
        QColor(BLUE_COLOR_SHADOW),
        ":/icons/back.svg",
        ID_BTN_BACK,
        std::bind(&RecapExposureSettingsView::onBackButtonClicked, this, std::placeholders::_1));
    addNavButton("left", backBtn);
}

void RecapExposureSettingsView::onBackButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW);
}

void RecapExposureSettingsView::onValidateButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::PROGRESS_EXPOSURE_VIEW);
}