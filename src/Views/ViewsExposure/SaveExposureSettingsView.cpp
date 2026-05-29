#include "Views/ViewsExposure/SaveExposureSettingsView.h"
#include "Views/Components/Colors.h"
#include "ui_SaveExposureSettingsView.h"
#include <QGraphicsDropShadowEffect>
#include <QRegularExpression>
#include <QString>

#define ID_BTN_HOME     "H"
#define ID_BTN_BACK     "B"
#define ID_BTN_SAVE     "S"
#define ID_BTN_VALIDATE "V"
#define BUTTONS_SIZE    150

SaveExposureSettingsView::SaveExposureSettingsView(Unique<SaveExposureSettingsViewModel> viewModel, QWidget *parent) :
    ExposureViewBase(std::move(viewModel), parent),
    ui(new Ui::SaveExposureSettingsView) {
    ui->setupUi(this);

    setNewNavButtonsConfigs();
    createNavButtonsConfigs();
    setDefaultTitleBar("Save parameters");
}
SaveExposureSettingsView::~SaveExposureSettingsView() {
}

void SaveExposureSettingsView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}

void SaveExposureSettingsView::setNewNavButtonsConfigs() {

    NavButtonConfig backBtn(
        "Back",
        QColor(BLUE_COLOR),
        QColor(BLUE_COLOR_SHADOW),
        ":/icons/back.svg",
        ID_BTN_BACK,
        std::bind(&SaveExposureSettingsView::onBackButtonClicked, this, std::placeholders::_1));
    addNavButton("left", backBtn);

    NavButtonConfig saveBtn(
        "Save",
        QColor(BLUE_COLOR),
        QColor(BLUE_COLOR_SHADOW),
        ":/icons/save.svg",
        ID_BTN_SAVE,
        std::bind(&SaveExposureSettingsView::onSaveButtonClicked, this, std::placeholders::_1));
    addNavButton("right", saveBtn);
}

void SaveExposureSettingsView::onBackButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW);
}

void SaveExposureSettingsView::onSaveButtonClicked(const QString &buttonId) {
    // emit s_openView(Kub3::UI::ViewId::SAVE_VIEW);
}

void SaveExposureSettingsView::onValidateButtonClicked(const QString &buttonId) {
}