#include "Views/HomeEightView.h"
#include "ui_HomeEightView.h"

#define ID_BTN_SETTINGS "S"
#define ID_BTN_OPEN     "O"
#define ID_BTN_CLOSE    "C"

HomeEightView::HomeEightView(Unique<HomeViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::HomeEightView) {
    ui->setupUi(this);

    createNavButtonsConfigs();
    m_showCentralLogo = true;
    configTitleBar();

    this->updateMachineLogo(this->height());

    connect(ui->mainSubmenuExposureBtn, &QPushButton::clicked, this, &HomeEightView::onFloodExposureClicked);
}

HomeEightView::~HomeEightView() {
    delete ui;
}

void HomeEightView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);

    this->updateMachineLogo(this->height());
}

void HomeEightView::updateMachineLogo(int h) {
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

    ui->mainTitle->setText(templateStr.arg(baseFontSize).arg(substrateSizeFontSize).arg(KUB_MODEL_STR));
}

void HomeEightView::createNavButtonsConfigs() {
    NavButtonConfig settingsBtn(
        "settings",
        QColor("#0072BA"),
        QColor("#B2D4F4"),
        ":/icons/settings.svg",
        ID_BTN_SETTINGS,
        std::bind(&HomeEightView::onSettingsButtonClicked, this, std::placeholders::_1));
    addNavButton("left", settingsBtn);

    NavButtonConfig openBtn(
        "open",
        QColor("#0072BA"),
        QColor("#B2D4F4"),
        ":/icons/eject.svg",
        ID_BTN_OPEN,
        std::bind(&HomeEightView::onOpenButtonClicked, this, std::placeholders::_1));
    addNavButton("right", openBtn);
}
void HomeEightView::configTitleBar() {
    m_titleBar = TitleBarConfig();
}

void HomeEightView::onSettingsButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::SETTINGS_VIEW);
}
void HomeEightView::onOpenButtonClicked(const QString &buttonId) {
    setNavButtonEnabled(ID_BTN_SETTINGS, false);
    removeNavButton(ID_BTN_OPEN);
    NavButtonConfig closeBtn(
        "close",
        QColor("#0072BA"),
        QColor("#B2D4F4"),
        ":/icons/insert.svg",
        ID_BTN_CLOSE,
        std::bind(&HomeEightView::onCloseButtonCliked, this, std::placeholders::_1));
    addNavButton("right", closeBtn);
}

void HomeEightView::onCloseButtonCliked(const QString &buttonId) {
    setNavButtonEnabled(ID_BTN_SETTINGS, true);
    m_buttonManager.removeButton(ID_BTN_CLOSE);
    NavButtonConfig openBtn(
        "open",
        QColor("#0072BA"),
        QColor("#B2D4F4"),
        ":/icons/eject.svg",
        ID_BTN_OPEN,
        std::bind(&HomeEightView::onOpenButtonClicked, this, std::placeholders::_1));
    addNavButton("right", openBtn);
}

void HomeEightView::onFloodExposureClicked() {
    emit s_openView(Kub3::UI::ViewId::HOME_VIEW);
}

void HomeEightView::onMaskAlignmentClicked() {
    emit s_openView(Kub3::UI::ViewId::ALIGNMENT_DISTANCE_VIEW);
}
