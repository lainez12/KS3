#include "Views/SettingsView.h"
#include "Views/Components/Colors.h"
#include "ui_SettingsView.h"

#define ID_BTN_HOME "H"

#define BUTTONS_SIZE 150

SettingsView::SettingsView(Unique<SettingsViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::SettingsView) {
    ui->setupUi(this);

    createNavButtonsConfigs();
    configTitleBar();

    ui->opeTimesBtn->setup("Operation Times", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/operating-times.svg");
    ui->opeTimesBtn->setSize(BUTTONS_SIZE);
    ui->aboutBtn->setup("About", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/about.svg");
    ui->aboutBtn->setSize(BUTTONS_SIZE);
    ui->temperatureBtn->setup("Temperature", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/temperature.svg");
    ui->temperatureBtn->setSize(BUTTONS_SIZE);
    ui->screenshootBtn->setup("Screenshot export", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/screenshots-export.svg");
    ui->screenshootBtn->setSize(BUTTONS_SIZE);
    ui->ledTestBtn->setup("Led test", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/led-test.svg");
    ui->ledTestBtn->setSize(BUTTONS_SIZE);
    ui->updateSoftBtn->setup("Update software", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/usb-software.svg");
    ui->updateSoftBtn->setSize(BUTTONS_SIZE);

    connect(ui->aboutBtn, &NavButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::SETTINGS_ABOUT_VIEW); });
    connect(ui->temperatureBtn, &NavButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::SETTINGS_TEMPERATURE_VIEW); });
    connect(ui->opeTimesBtn, &NavButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::SETTINGS_OPERATING_TIMES_VIEW); });
    connect(ui->screenshootBtn, &NavButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::SETTINGS_SCREENSHOT_EXPORT_VIEW); });
    connect(ui->ledTestBtn, &NavButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::SETTINGS_LED_TEST_VIEW); });
    connect(ui->updateSoftBtn, &NavButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::SETTINGS_UPDATE_SOFTWARE_VIEW); });
}
SettingsView::~SettingsView() {
}

void SettingsView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
    ui->buttonsLayout->setSpacing(this->width() * 0.03); // 7% of window width
}

void SettingsView::createNavButtonsConfigs() {
    NavButtonConfig homeBtn(
        "Home",
        ":/icons/home.svg",
        ID_BTN_HOME,
        std::bind(&SettingsView::onHomeButtonClicked, this, std::placeholders::_1));
    addNavButton("left", homeBtn);
}
void SettingsView::configTitleBar() {
    m_titleBar = TitleBarConfig(
        "Parameters",
        QColor("#FFF"),
        QColor(BLUE_COLOR),
        ":/icons/admin-circle.svg",
        "Parameters",
        true,
        true);
}

void SettingsView::onHomeButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::HOME_VIEW);
}