#include "ui_ExposureSettingsView.h"
#include <QGraphicsDropShadowEffect>
#include <QRegularExpression>
#include <QString>
#include <Views/Components/Colors.h>
#include <Views/Exposure/ExposureSettingsView.h>

#define ID_BTN_HOME     "H"
#define ID_BTN_BACK     "B"
#define ID_BTN_SAVE     "S"
#define ID_BTN_VALIDATE "V"
#define BUTTONS_SIZE    150

ExposureSettingsView::ExposureSettingsView(Unique<ExposureSettingsViewModel> viewModel, QWidget *parent) :
    ExposureViewBase(std::move(viewModel), parent),
    m_keyboard(this),
    ui(new Ui::ExposureSettingsView)
{
    ui->setupUi(this);
    m_keyboard.setupKeyboardConnections(this);
    ui->inputsStack->setCurrentWidget(ui->pageContinuous);

    NavButtonConfig saveBtn(
        "Save",
        ":/icons/save.svg",
        ID_BTN_SAVE,
        std::bind(&ExposureSettingsView::onSaveButtonClicked, this));
    addNavButton("right", saveBtn);

    createNavButtonsConfigs();

    NavButtonConfig backBtn(
        "Back",
        ":/icons/back.svg",
        ID_BTN_BACK,
        std::bind(&ExposureSettingsView::onBackButtonClicked, this));
    addNavButton("left", backBtn);
    setDefaultTitleBar("Exposure settings");

    setUpShadowedBoxStyle(ui->minContinuouspinBox);
    setUpShadowedBoxStyle(ui->segContinuouspinBox);
    setUpShadowedBoxStyle(ui->numberCyclespinBox);
    setUpShadowedBoxStyle(ui->minOnFlashingspinBox);
    setUpShadowedBoxStyle(ui->segOnFlashingspinBox);
    setUpShadowedBoxStyle(ui->minOffFlashingspinBox);
    setUpShadowedBoxStyle(ui->segOffFlashingspinBox);
    setUpShadowedBoxStyle(ui->powerContinuouspinBox);
    setUpShadowedBoxStyle(ui->powerFlashingspinBox);

    QPixmap pixmapLastCycle(":icons/last-cycle-enabled.png");

    QPalette paleta;
    paleta.setBrush(QPalette::Window, pixmapLastCycle.scaled(ui->containerLastCycle->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    ui->containerLastCycle->setPalette(paleta);
    ui->containerLastCycle->setAutoFillBackground(true);

    connect(ui->favoriteSettingslabel, &QPushButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::FAVORITE_EXPOSURE_SETTINGS_VIEW); });
    connect(ui->continuousModeBtn, &QPushButton::clicked, this, &ExposureSettingsView::switchToContinuousMode);
    connect(ui->flashingModeBtn, &QPushButton::clicked, this, &ExposureSettingsView::switchToFlashingMode);

    connectQSpinBoxSignals();
    ui->exposureDuration->hide();
    ui->titleExposureDuration->hide();
}

ExposureSettingsView::~ExposureSettingsView()
{
}

void ExposureSettingsView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void ExposureSettingsView::onBackButtonClicked()
{
}

void ExposureSettingsView::onSaveButtonClicked()
{
    PresetExposure settings = getCurrentPresetExposure();

    getViewModel<ExposureSettingsViewModel>()->uiRequestSaveExposureSettings(settings);
    emit s_openView(Kub3::UI::ViewId::SAVE_EXPOSURE_SETTINGS_VIEW);
}

void ExposureSettingsView::onValidateButtonClicked()
{
    auto mv = getViewModel<ExposureSettingsViewModel>();

    if (mv)
    {
        PresetExposure settings = getCurrentPresetExposure();
        settings.name           = "current"; // Temporary name for validation purposes
        mv->uiRequestExposureSettingsByForm(settings);
    }
    emit s_openView(Kub3::UI::ViewId::RECAP_EXPOSURE_SETTINGS_VIEW);
}

void ExposureSettingsView::switchToFlashingMode()
{
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

void ExposureSettingsView::switchToContinuousMode()
{
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

PresetExposure ExposureSettingsView::getCurrentPresetExposure() const
{
    PresetExposure preset;

    preset.mode = m_isFlashingMode ? ExposureMode::Flashing : ExposureMode::Continuous;
    if (preset.mode == ExposureMode::Continuous)
    {
        preset.continuous.duration.minutes = ui->minContinuouspinBox->value();
        preset.continuous.duration.seconds = ui->segContinuouspinBox->value();
        preset.continuous.power            = ui->powerContinuouspinBox->value();
    }
    else
    {
        preset.flashing.numberOfCycles      = ui->numberCyclespinBox->value();
        preset.flashing.durationOn.minutes  = ui->minOnFlashingspinBox->value();
        preset.flashing.durationOn.seconds  = ui->segOnFlashingspinBox->value();
        preset.flashing.durationOff.minutes = ui->minOffFlashingspinBox->value();
        preset.flashing.durationOff.seconds = ui->segOffFlashingspinBox->value();
        preset.flashing.power               = ui->powerFlashingspinBox->value();
    }

    return preset;
}

void ExposureSettingsView::watchForChangesInPresetExposure()
{
    auto vm                      = getViewModel<ExposureSettingsViewModel>();
    PresetExposure currentPreset = getCurrentPresetExposure();

    currentPreset.name = "current"; // Temporary name for validation purposes
    if (vm)
    {
        QString errorMessage;
        bool isValid = vm->validatePreset(currentPreset, &errorMessage);
        setNavButtonEnabled(ID_BTN_VALIDATE, isValid);
        setNavButtonEnabled(ID_BTN_SAVE, isValid);
    }
}

void ExposureSettingsView::connectQSpinBoxSignals()
{
    connect(ui->minContinuouspinBox, &QSpinBox::valueChanged, this, &ExposureSettingsView::watchForChangesInPresetExposure);
    connect(ui->segContinuouspinBox, &QSpinBox::valueChanged, this, &ExposureSettingsView::watchForChangesInPresetExposure);
    connect(ui->powerContinuouspinBox, &QSpinBox::valueChanged, this, &ExposureSettingsView::watchForChangesInPresetExposure);
    connect(ui->numberCyclespinBox, &QSpinBox::valueChanged, this, &ExposureSettingsView::watchForChangesInPresetExposure);
    connect(ui->minOnFlashingspinBox, &QSpinBox::valueChanged, this, &ExposureSettingsView::watchForChangesInPresetExposure);
    connect(ui->segOnFlashingspinBox, &QSpinBox::valueChanged, this, &ExposureSettingsView::watchForChangesInPresetExposure);
    connect(ui->minOffFlashingspinBox, &QSpinBox::valueChanged, this, &ExposureSettingsView::watchForChangesInPresetExposure);
    connect(ui->segOffFlashingspinBox, &QSpinBox::valueChanged, this, &ExposureSettingsView::watchForChangesInPresetExposure);
    connect(ui->powerFlashingspinBox, &QSpinBox::valueChanged, this, &ExposureSettingsView::watchForChangesInPresetExposure);
}
