#include "Views/ExposureSettingsView.h"
#include "Views/Components/Colors.h"
#include "ui_ExposureSettingsView.h"
#include <QGraphicsDropShadowEffect>
#include <QRegularExpression>
#include <QString>

#define ID_BTN_HOME     "H"
#define ID_BTN_BACK     "B"
#define ID_BTN_SAVE     "S"
#define ID_BTN_VALIDATE "V"
#define BUTTONS_SIZE    150

ExposureSettingsView::ExposureSettingsView(Unique<ExposureSettingsViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::ExposureSettingsView) {
    ui->setupUi(this);

    ui->inputsStack->setCurrentWidget(ui->pageContinuous);

    createNavButtonsConfigs();
    configTitleBar();

    // ui->flashingContainer->hide();

    setUpShawedBoxStyle(ui->minContinuouspinBox);
    setUpShawedBoxStyle(ui->segContinuouspinBox);
    setUpShawedBoxStyle(ui->numberCyclespinBox);
    setUpShawedBoxStyle(ui->minOnFlashingspinBox);
    setUpShawedBoxStyle(ui->segOnFlashingspinBox);
    setUpShawedBoxStyle(ui->minOffFlashingspinBox);
    setUpShawedBoxStyle(ui->segOffFlashingspinBox);
    setUpShawedBoxStyle(ui->powerContinuouspinBox);
    setUpShawedBoxStyle(ui->powerFlashingspinBox);

    QPixmap pixmapLastCycle(":icons/last-cycle-enabled.png");

    QPalette paleta;
    paleta.setBrush(QPalette::Window, pixmapLastCycle.scaled(ui->containerLastCycle->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    ui->containerLastCycle->setPalette(paleta);
    ui->containerLastCycle->setAutoFillBackground(true);
    ui->containerLastCycle->setStyleSheet("border-image: url(:/icons/last-cycle-enabled.png);");

    ui->lastCycleTextLabel->setStyleSheet("background: transparent; border-image: none; color: " ORANGE_COLOR "; font-size: 18px; font-weight: 700;");

    connect(ui->favoriteSettingslabel, &QPushButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::FAVORITE_EXPOSURE_SETTINGS_VIEW); });
    connect(ui->continuousModeBtn, &QPushButton::clicked, this, &ExposureSettingsView::switchToContinuousMode);
    connect(ui->flashingModeBtn, &QPushButton::clicked, this, &ExposureSettingsView::switchToFlashingMode);
}
ExposureSettingsView::~ExposureSettingsView() {
}

void ExposureSettingsView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}

void ExposureSettingsView::createNavButtonsConfigs() {
    NavButtonConfig homeBtn(
        "Home",
        QColor(BLUE_COLOR),
        ":/icons/home.svg",
        ID_BTN_HOME,
        std::bind(&ExposureSettingsView::onHomeButtonClicked, this, std::placeholders::_1));
    addNavButton("left", homeBtn);

    NavButtonConfig backBtn(
        "Back",
        QColor(BLUE_COLOR),
        ":/icons/back.svg",
        ID_BTN_BACK,
        std::bind(&ExposureSettingsView::onBackButtonClicked, this, std::placeholders::_1));
    addNavButton("left", backBtn);

    NavButtonConfig saveBtn(
        "Save",
        QColor(BLUE_COLOR),
        ":/icons/save.svg",
        ID_BTN_SAVE,
        std::bind(&ExposureSettingsView::onSaveButtonClicked, this, std::placeholders::_1));
    addNavButton("right", saveBtn);

    NavButtonConfig validateBtn(
        "Validate",
        QColor(GREEN_COLOR),
        ":/icons/check.svg",
        ID_BTN_VALIDATE,
        std::bind(&ExposureSettingsView::onValidateButtonClicked, this, std::placeholders::_1));
    addNavButton("right", validateBtn);
}
void ExposureSettingsView::configTitleBar() {
    m_titleBar = TitleBarConfig(
        "Exposure Settings",
        QColor("#FFF"),
        QColor(PURPLE_COLOR),
        ":/icons/flood_icon.svg",
        "Flood Exposure",
        true,
        true);
}

void ExposureSettingsView::onHomeButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::HOME_VIEW);
}

void ExposureSettingsView::onBackButtonClicked(const QString &buttonId) {
    // emit s_openView(Kub3::UI::ViewId::BACK_VIEW);
}

void ExposureSettingsView::onSaveButtonClicked(const QString &buttonId) {
    // emit s_openView(Kub3::UI::ViewId::SAVE_VIEW);
}

void ExposureSettingsView::onValidateButtonClicked(const QString &buttonId) {
    // emit s_openView(Kub3::UI::ViewId::VALIDATE_VIEW);
}

void ExposureSettingsView::switchToFlashingMode() {
    ui->inputsStack->setCurrentWidget(ui->pageFlashing);
    if (m_isFlashingMode)
        return;

    QString styleSheet = ui->flashingModeBtn->styleSheet();
    ui->continuousModeBtn->setStyleSheet(styleSheet);
    ui->continuousCarre->setPixmap(QPixmap(":/icons/carre-bleu-clair.png"));

    styleSheet.replace(QRegularExpression(BLUE_COLOR_SHADOW), BLUE_COLOR);

    ui->flashingModeBtn->setStyleSheet(styleSheet);
    ui->flashingCarre->setPixmap(QPixmap(":/icons/carre-bleu-fonce.png"));
    m_isFlashingMode = true;
    ui->imageFlashingMode->setPixmap(QPixmap(":/icons/schema-flashing.svg"));
}

void ExposureSettingsView::switchToContinuousMode() {

    ui->inputsStack->setCurrentWidget(ui->pageContinuous);

    if (!m_isFlashingMode)
        return;

    m_isFlashingMode   = false;
    QString styleSheet = ui->flashingModeBtn->styleSheet();
    ui->imageFlashingMode->setPixmap(QPixmap(":/icons/schema-flashing-vide.svg"));
    ui->continuousModeBtn->setStyleSheet(styleSheet);
    ui->flashingCarre->setPixmap(QPixmap(":/icons/carre-bleu-clair.png"));

    styleSheet.replace(QRegularExpression(BLUE_COLOR), BLUE_COLOR_SHADOW);
    ui->flashingModeBtn->setStyleSheet(styleSheet);
    ui->continuousCarre->setPixmap(QPixmap(":/icons/carre-bleu-fonce.png"));
}