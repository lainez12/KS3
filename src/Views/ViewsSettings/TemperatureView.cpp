#include "Views/ViewsSettings/TemperatureView.h"
#include "Views/Colors.h"
#include "ui_TemperatureView.h"
#include <QString>

#define ID_BTN_HOME "H"
#define ID_BTN_BACK "B"

TemperatureView::TemperatureView(Unique<TemperatureViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::TemperatureView) {
    ui->setupUi(this);

    createNavButtonsConfigs();
    configTitleBar();
}
TemperatureView::~TemperatureView() {
}

void TemperatureView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}

void TemperatureView::createNavButtonsConfigs() {
    NavButtonConfig homeBtn(
        "Home",
        QColor(BLUE_COLOR),
        ":/icons/home.svg",
        ID_BTN_HOME,
        std::bind(&TemperatureView::onHomeButtonClicked, this, std::placeholders::_1));
    addNavButton("left", homeBtn);

    NavButtonConfig backBtn(
        "Back",
        QColor(BLUE_COLOR),
        ":/icons/back.svg",
        ID_BTN_BACK,
        std::bind(&TemperatureView::onBackButtonClicked, this, std::placeholders::_1));
    addNavButton("left", backBtn);
}
void TemperatureView::configTitleBar() {
    m_titleBar = TitleBarConfig(
        "Temperature",
        QColor("#FFF"),
        QColor(BLUE_COLOR),
        ":/icons/admin-circle.svg",
        "Parameters",
        true,
        true);
}

void TemperatureView::onHomeButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::HOME_VIEW);
}

void TemperatureView::onBackButtonClicked(const QString &buttonId) {
    // emit s_openView(Kub3::UI::ViewId::BACK_VIEW);
}