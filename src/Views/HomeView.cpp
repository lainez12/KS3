#include "Views/HomeView.h"
#include "ui_HomeView.h"

#define ID_BTN_SETTINGS "S"
#define ID_BTN_OPEN     "O"
#define ID_BTN_CLOSE    "C"

HomeView::HomeView(Unique<HomeViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::HomeView) {
    ui->setupUi(this);

    createNavButtonsConfigs();
    m_showCentralLogo = true;
    configTitleBar();

    ui->floodBtn->setup("Flood Exposure", "#8a2be2", ":/icons/flood_icon.svg"); // Purple
    // ui->maskExpBtn->setup("Mask Exposure", "#0000cd", ":/icons/mask-exposure_icon.svg");     // Blue
    ui->maskAlignBtn->setup("Mask Alignment", "#00ced1", ":/icons/mask-alignment_icon.svg"); // Cyan

    connect(ui->floodBtn, &ActionBox::clicked, this, &HomeView::onFloodExposureClicked);
    // connect(ui->maskExpBtn, &ActionBox::clicked, this, &HomeView::onMaskExposureClicked);
    connect(ui->maskAlignBtn, &ActionBox::clicked, this, &HomeView::onMaskAlignmentClicked);

    this->updateMachineLogo(this->height());
}

HomeView::~HomeView() {
    delete ui;
}

void HomeView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);

    this->updateMachineLogo(this->height());
    ui->buttonsLayout->setSpacing(this->width() * 0.07); // 7% of window width
}

void HomeView::updateMachineLogo(int h) {
    if (h <= 0)
        return;

    const int baseFontSize          = h * 0.1;             // 6% of window height
    const int substrateSizeFontSize = baseFontSize * 0.64; // 64% of base
    // We use arg %1 for the logo size, arg %2 for the substrate size, arg %3 for the text
    const QString templateStr =
        "<span style='font-family:\"Art ttnorm\"; font-style:italic;"
        "font-size:%1px; color:#0072ba;'>UV-KUB 3</span>"
        "<span style='font-family:\"Art ttnorm\"; font-style:italic;"
        "font-size:%2px; color:#e85420;'>&nbsp;%3-inch</span>";

    ui->machineName->setText(templateStr.arg(baseFontSize).arg(substrateSizeFontSize).arg(KUB_MODEL_STR));
}

void HomeView::createNavButtonsConfigs() {
    NavButtonConfig settingsBtn(
        "settings",
        QColor("#0072BA"),
        QColor("#B2D4F4"),
        ":/icons/settings.svg",
        ID_BTN_SETTINGS,
        std::bind(&HomeView::onSettingsButtonClicked, this, std::placeholders::_1));
    addNavButton("left", settingsBtn);

    NavButtonConfig openBtn(
        "open",
        QColor("#0072BA"),
        QColor("#B2D4F4"),
        ":/icons/eject.svg",
        ID_BTN_OPEN,
        std::bind(&HomeView::onOpenButtonClicked, this, std::placeholders::_1));
    addNavButton("right", openBtn);
}
void HomeView::configTitleBar() {
    m_titleBar = TitleBarConfig();
}

void HomeView::onSettingsButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::SETTINGS_VIEW);
}
void HomeView::onOpenButtonClicked(const QString &buttonId) {
    setNavButtonEnabled(ID_BTN_SETTINGS, false);
    removeNavButton(ID_BTN_OPEN);
    NavButtonConfig closeBtn(
        "close",
        QColor("#0072BA"),
        QColor("#B2D4F4"),
        ":/icons/insert.svg",
        ID_BTN_CLOSE,
        std::bind(&HomeView::onCloseButtonCliked, this, std::placeholders::_1));
    addNavButton("right", closeBtn);
}

void HomeView::onCloseButtonCliked(const QString &buttonId) {
    setNavButtonEnabled(ID_BTN_SETTINGS, true);
    m_buttonManager.removeButton(ID_BTN_CLOSE);
    NavButtonConfig openBtn(
        "open",
        QColor("#0072BA"),
        QColor("#B2D4F4"),
        ":/icons/eject.svg",
        ID_BTN_OPEN,
        std::bind(&HomeView::onOpenButtonClicked, this, std::placeholders::_1));
    addNavButton("right", openBtn);
}

void HomeView::onFloodExposureClicked() {
    emit s_openView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW);
}

void HomeView::onMaskAlignmentClicked() {
    emit s_openView(Kub3::UI::ViewId::ALIGNMENT_DISTANCE_VIEW);
}
