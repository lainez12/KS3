#include "Views/ViewsExposure/ExposureSettingsView.h"
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
    ExposureViewBase(std::move(viewModel), parent),
    ui(new Ui::ExposureSettingsView) {
    ui->setupUi(this);

    ui->inputsStack->setCurrentWidget(ui->pageContinuous);

    setNewNavButtonsConfigs();
    createNavButtonsConfigs();
    setDefaultTitleBar("Exposure Settings");

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

    setNavButtonEnabled(ID_BTN_VALIDATE, true);
}
ExposureSettingsView::~ExposureSettingsView() {
}

void ExposureSettingsView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}

void ExposureSettingsView::setNewNavButtonsConfigs() {

    NavButtonConfig backBtn(
        "Back",
        QColor(BLUE_COLOR),
        QColor(BLUE_COLOR_SHADOW),
        ":/icons/back.svg",
        ID_BTN_BACK,
        std::bind(&ExposureSettingsView::onBackButtonClicked, this, std::placeholders::_1));
    addNavButton("left", backBtn);

    NavButtonConfig saveBtn(
        "Save",
        QColor(BLUE_COLOR),
        QColor(BLUE_COLOR_SHADOW),
        ":/icons/save.svg",
        ID_BTN_SAVE,
        std::bind(&ExposureSettingsView::onSaveButtonClicked, this, std::placeholders::_1));
    addNavButton("right", saveBtn);
}

void ExposureSettingsView::onBackButtonClicked(const QString &buttonId) {
    // emit s_openView(Kub3::UI::ViewId::BACK_VIEW);
}

void ExposureSettingsView::onSaveButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::SAVE_EXPOSURE_SETTINGS_VIEW);
}

void ExposureSettingsView::onValidateButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::RECAP_EXPOSURE_SETTINGS_VIEW);
}

void ExposureSettingsView::switchToFlashingMode() {
    if (m_isFlashingMode)
        return;
    ui->inputsStack->setCurrentWidget(ui->pageFlashing);

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
    if (!m_isFlashingMode)
        return;
    ui->inputsStack->setCurrentWidget(ui->pageContinuous);

    m_isFlashingMode   = false;
    QString styleSheet = ui->flashingModeBtn->styleSheet();
    ui->imageFlashingMode->setPixmap(QPixmap(":/icons/schema-flashing-vide.svg"));
    ui->continuousModeBtn->setStyleSheet(styleSheet);
    ui->flashingCarre->setPixmap(QPixmap(":/icons/carre-bleu-clair.png"));

    styleSheet.replace(QRegularExpression(BLUE_COLOR), BLUE_COLOR_SHADOW);
    ui->flashingModeBtn->setStyleSheet(styleSheet);
    ui->continuousCarre->setPixmap(QPixmap(":/icons/carre-bleu-fonce.png"));
}