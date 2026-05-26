#include "Views/ViewsSettings/VersionView.h"
#include "Views/Colors.h"
#include "ui_VersionView.h"
#include <QString>

#define ID_BTN_HOME "H"
#define ID_BTN_BACK "B"

VersionView::VersionView(Unique<VersionViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::VersionView) {
    ui->setupUi(this);

    createNavButtonsConfigs();
    configTitleBar();
}
VersionView::~VersionView() {
}

void VersionView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}

void VersionView::createNavButtonsConfigs() {
    NavButtonConfig homeBtn(
        "Home",
        QColor(BLUE_COLOR),
        ":/icons/home.svg",
        ID_BTN_HOME,
        std::bind(&VersionView::onHomeButtonClicked, this, std::placeholders::_1));
    addNavButton("left", homeBtn);

    NavButtonConfig backBtn(
        "Back",
        QColor(BLUE_COLOR),
        ":/icons/back.svg",
        ID_BTN_BACK,
        std::bind(&VersionView::onBackButtonClicked, this, std::placeholders::_1));
    addNavButton("left", backBtn);
}
void VersionView::configTitleBar() {
    m_titleBar = TitleBarConfig(
        "Version Note",
        QColor("#FFF"),
        QColor(PURPLE_COLOR),
        ":/icons/flood_icon.svg",
        "Parameters",
        true,
        true);
}

void VersionView::onHomeButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::HOME_VIEW);
}

void VersionView::onBackButtonClicked(const QString &buttonId) {
    // emit s_openView(Kub3::UI::ViewId::BACK_VIEW);
}