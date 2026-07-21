#include <Views/HomeEightView.h>

#include "ui_HomeEightView.h"

#define ID_BTN_SETTINGS "S"
#define ID_BTN_OPEN     "O"
#define ID_BTN_CLOSE    "C"
#define BUTTONS_SIZE    280

HomeEightView::HomeEightView(Unique<HomeViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::HomeEightView)
{
    ui->setupUi(this);

    QFont font("Arial", 22, QFont::Bold);
    ui->maskBtn->setup(NavButton::SetupParams{"Mask", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/insert.svg", font});
    ui->maskBtn->setSize(BUTTONS_SIZE);
    ui->waferBtn->setup(NavButton::SetupParams{"Wafer", BLUE_COLOR, BLUE_COLOR_SHADOW, ":/icons/insert.svg", font});
    ui->waferBtn->setSize(BUTTONS_SIZE);

    createNavButtonsConfigs();
    m_showCentralLogo = true;
    configTitleBar();

    this->updateMachineLogo(this->height());

    HomeViewModel *vm = static_cast<HomeViewModel *>(m_viewModel.get());

    if (vm)
    {
        connect(ui->btnInitialization, &QPushButton::clicked, vm, &HomeViewModel::uiRequestInitialization);
    }
    connect(ui->mainSubmenuExposureBtn, &QPushButton::clicked, this, &HomeEightView::onExposureMenuClicked);
}

HomeEightView::~HomeEightView()
{
    delete ui;
}

void HomeEightView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);

    this->updateMachineLogo(this->height());
}

void HomeEightView::updateMachineLogo(int h)
{
    if (h <= 0)
        return;

    const int baseFontSize          = h * 0.11;            // 6% of window height
    const int substrateSizeFontSize = baseFontSize * 0.45; // 64% of base
    // We use arg %1 for the logo size, arg %2 for the substrate size, arg %3 for the text
    const QString templateStr =
        "<span style='font-family:\"Art ttnorm\"; font-style:italic;"
        "font-size:%1px; color:#0072ba; letter-spacing: 0px;'>&nbsp;UV-KUB 3</span>"
        "<span style='font-family:\"Art ttnorm\"; font-style:italic;"
        "font-size:%1px; color:#e85420; letter-spacing: 0px;'> - </span>"
        "<span style='font-family:\"Art ttnorm\"; font-style:italic;"
        "font-size:%1px; color:#e85420; letter-spacing: 3px;'>%3</span>"
        "<span style='font-family:\"Art ttnorm\"; font-style:italic;"
        "font-size:%2px; color:#e85420; letter-spacing: 0px;'>-inch</span>";

    ui->mainTitle->setText(templateStr.arg(baseFontSize).arg(substrateSizeFontSize).arg(KUB_MODEL_STR));
}

void HomeEightView::createNavButtonsConfigs()
{
    NavButtonConfig settingsBtn(
        "Settings",
        QColor("#0072BA"),
        QColor("#B2D4F4"),
        ":/icons/settings.svg",
        ID_BTN_SETTINGS,
        std::bind(&HomeEightView::onSettingsButtonClicked, this));
    addNavButton("left", settingsBtn);
}
void HomeEightView::configTitleBar()
{
    setTitleBar(TitleBarConfig{});
}

void HomeEightView::onSettingsButtonClicked()
{
    emit s_openView(Kub3::UI::ViewId::SETTINGS_VIEW);
}

void HomeEightView::onExposureMenuClicked()
{
    emit s_openView(Kub3::UI::ViewId::EXPOSURE_MENU_VIEW);
}

void HomeEightView::onMaskAlignmentClicked()
{
    emit s_openView(Kub3::UI::ViewId::ALIGNMENT_DISTANCE_VIEW);
}
