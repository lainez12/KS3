#include "Views/FavoriteExposureSettingsView.h"
#include "Views/Components/Colors.h"
#include "Views/Components/FavoriteExposureSettingButton.h"
#include "ui_FavoriteExposureSettingsView.h"
#include <QRegularExpression>
#include <QString>

#define ID_BTN_HOME     "H"
#define ID_BTN_BACK     "B"
#define ID_BTN_SAVE     "S"
#define ID_BTN_VALIDATE "V"
#define BUTTONS_SIZE    150

FavoriteExposureSettingsView::FavoriteExposureSettingsView(Unique<FavoriteExposureSettingsViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::FavoriteExposureSettingsView) {
    ui->setupUi(this);

    createNavButtonsConfigs();
    configTitleBar();

    connect(ui->newSettingsLabel, &QPushButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW); });
    FavoriteExposureSettingButton *btnFirst = new FavoriteExposureSettingButton(
        "First",
        "Exposure duration: 10s\nExposure power: 20%");

    ui->horizontalLayout->addWidget(btnFirst);
    setUpShawedBoxStyle(btnFirst);
}
FavoriteExposureSettingsView::~FavoriteExposureSettingsView() {
}

void FavoriteExposureSettingsView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}

void FavoriteExposureSettingsView::createNavButtonsConfigs() {
    NavButtonConfig homeBtn(
        "Home",
        QColor(BLUE_COLOR),
        ":/icons/home.svg",
        ID_BTN_HOME,
        std::bind(&FavoriteExposureSettingsView::onHomeButtonClicked, this, std::placeholders::_1));
    addNavButton("left", homeBtn);

    NavButtonConfig backBtn(
        "Back",
        QColor(BLUE_COLOR),
        ":/icons/back.svg",
        ID_BTN_BACK,
        std::bind(&FavoriteExposureSettingsView::onBackButtonClicked, this, std::placeholders::_1));
    addNavButton("left", backBtn);

    NavButtonConfig saveBtn(
        "Save",
        QColor(BLUE_COLOR),
        ":/icons/save.svg",
        ID_BTN_SAVE,
        std::bind(&FavoriteExposureSettingsView::onSaveButtonClicked, this, std::placeholders::_1));
    addNavButton("right", saveBtn);

    NavButtonConfig validateBtn(
        "Validate",
        QColor(GREEN_COLOR),
        ":/icons/check.svg",
        ID_BTN_VALIDATE,
        std::bind(&FavoriteExposureSettingsView::onValidateButtonClicked, this, std::placeholders::_1));
    addNavButton("right", validateBtn);
}
void FavoriteExposureSettingsView::configTitleBar() {
    m_titleBar = TitleBarConfig(
        "Exposure Settings",
        QColor("#FFF"),
        QColor(PURPLE_COLOR),
        ":/icons/flood_icon.svg",
        "Flood Exposure",
        true,
        true);
}

void FavoriteExposureSettingsView::onHomeButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::HOME_VIEW);
}

void FavoriteExposureSettingsView::onBackButtonClicked(const QString &buttonId) {
    // emit s_openView(Kub3::UI::ViewId::BACK_VIEW);
}

void FavoriteExposureSettingsView::onSaveButtonClicked(const QString &buttonId) {
    // emit s_openView(Kub3::UI::ViewId::SAVE_VIEW);
}

void FavoriteExposureSettingsView::onValidateButtonClicked(const QString &buttonId) {
    // emit s_openView(Kub3::UI::ViewId::VALIDATE_VIEW);
}