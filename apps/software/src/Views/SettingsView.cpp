#include <Views/Components/Colors.h>
#include <Views/SettingsView.h>

#include "ui_SettingsView.h"

#define ID_BTN_HOME "H"

#define BUTTONS_SIZE 150

SettingsView::SettingsView(Unique<SettingsViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::SettingsView)
{
    ui->setupUi(this);

    createNavButtonsConfigs();
    configTitleBar();

    QFont font("Arial", 22);
    ui->opeTimesBtn->setup("Operation Times", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/operating-times.svg", font);
    ui->opeTimesBtn->setSize(BUTTONS_SIZE);
    ui->aboutBtn->setup("About", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/about.svg", font);
    ui->aboutBtn->setSize(BUTTONS_SIZE);
    ui->temperatureBtn->setup("Temperature", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/temperature.svg", font);
    ui->temperatureBtn->setSize(BUTTONS_SIZE);
    ui->screenshootBtn->setup("Screenshot export", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/screenshots-export.svg", font);
    ui->screenshootBtn->setSize(BUTTONS_SIZE);
    ui->ledTestBtn->setup("Led test", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/led-test.svg", font);
    ui->ledTestBtn->setSize(BUTTONS_SIZE);
    ui->updateSoftBtn->setup("Update software", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/usb-software.svg", font);
    ui->updateSoftBtn->setSize(BUTTONS_SIZE);
    ui->machineSettingsBtn->setup("Machine Status", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/select-all.svg", font);
    ui->machineSettingsBtn->setSize(BUTTONS_SIZE);
    ui->adminBtn->setup("Admin", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/admin.svg", font);
    ui->adminBtn->setSize(BUTTONS_SIZE);

    connect(ui->aboutBtn, &NavButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::SETTINGS_ABOUT_VIEW); });
    connect(ui->temperatureBtn, &NavButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::SETTINGS_TEMPERATURE_VIEW); });
    connect(ui->opeTimesBtn, &NavButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::SETTINGS_OPERATING_TIMES_VIEW); });
    connect(ui->screenshootBtn, &NavButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::SETTINGS_SCREENSHOT_EXPORT_VIEW); });
    connect(ui->ledTestBtn, &NavButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::SETTINGS_LED_TEST_VIEW); });
    connect(ui->updateSoftBtn, &NavButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::SETTINGS_UPDATE_SOFTWARE_VIEW); });
    connect(ui->machineSettingsBtn, &NavButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::MACHINE_STATUS_VIEW); });
    connect(ui->adminBtn, &NavButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::SETTINGS_ADMIN_PASSWORD_VIEW); });
}
SettingsView::~SettingsView()
{
}

void SettingsView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void SettingsView::createNavButtonsConfigs()
{
    NavButtonConfig homeBtn(
        "Back to Main",
        QColor("#FFF"),
        QColor("#B2D4F4"),
        ":/icons/picto.png",
        ID_BTN_HOME,
        std::bind(&SettingsView::onHomeButtonClicked, this, std::placeholders::_1));
    homeBtn.isTextColorDifferent = true;
    homeBtn.textColor            = QColor("#000");
    addNavButton("left", homeBtn);
}
void SettingsView::configTitleBar()
{
    setTitleBar(TitleBarConfig{
        .viewTitle      = "Menu",
        .textColor      = QColor("#FFF"),
        .bgColor        = QColor(BLUE_COLOR),
        .iconPath       = ":/icons/settings.svg",
        .sectionTitle   = "Settings",
        .showTitleBar   = true,
        .m_showLeftLogo = true,
    });
}

void SettingsView::onHomeButtonClicked(const QString &buttonId)
{
    emit s_openView(Kub3::UI::ViewId::HOME_EIGHT_VIEW);
}