#include "Views/SettingsView.h"
#include "ui_SettingsView.h"

#define ID_BTN_HOME "H"
#define BLUE_COLOR "#0070DB"

SettingsView::SettingsView(Unique<SettingsViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::SettingsView) {
    ui->setupUi(this);
}
SettingsView::~SettingsView() {
}

void SettingsView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
    ui->buttonsLayout->setSpacing(this->width() * 0.07); // 7% of window width
}

void SettingsView::createNavButtonsConfigs()
{
    NavButtonConfig homeBtn(
        "Home",
        QColor(BLUE_COLOR),
        ":/icons/home.svg",
        ID_BTN_HOME,
        std::bind(&SettingsView::onHomeButtonClicked, this, std::placeholders::_1)
    );
    addNavButton("left", homeBtn);
}
void SettingsView::configTitleBar()
{
    m_titleBar = TitleBarConfig(
        "Parameters",
        QColor("#FFF"),
        QColor(BLUE_COLOR),
        ":/icons/admin.svg",
        "Parameters",
        true,
        true
    );
}

void SettingsView::onHomeButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::HOME_VIEW);
}