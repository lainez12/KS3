#include <Views/Components/Colors.h>
#include <Views/ExposureMenuView.h>

#include "ui_ExposureMenuView.h"

ExposureMenuView::ExposureMenuView(Unique<ExposureMenuViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::ExposureMenuView)
{
    ui->setupUi(this);

    createNavButtonsConfigs();
    configTitleBar();

    connect(ui->exposureModeBtn, &QPushButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::HOME_VIEW); });
}

ExposureMenuView::~ExposureMenuView()
{
    delete ui;
}

void ExposureMenuView::createNavButtonsConfigs()
{
    NavButtonConfig backBtn(
        "Back",
        ":/icons/back.svg",
        "B",
        std::bind(&ExposureMenuView::onBackButtonClicked, this, std::placeholders::_1));
    addNavButton("left", backBtn);
}

void ExposureMenuView::configTitleBar()
{
    setTitleBar(TitleBarConfig{
        .viewTitle      = "Exposure Menu",
        .textColor      = QColor("#FFF"),
        .bgColor        = QColor(BLUE_COLOR),
        .iconPath       = "",
        .sectionTitle   = "Load",
        .showTitleBar   = true,
        .m_showLeftLogo = true,
    });
}

void ExposureMenuView::onBackButtonClicked(const QString &buttonId)
{
    emit s_openView(Kub3::UI::ViewId::HOME_EIGHT_VIEW);
}