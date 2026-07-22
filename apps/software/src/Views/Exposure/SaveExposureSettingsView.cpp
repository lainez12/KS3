#include "ui_SaveExposureSettingsView.h"
#include <QGraphicsDropShadowEffect>
#include <QRegularExpression>
#include <QString>
#include <QStyle>
#include <Views/Components/Colors.h>
#include <Views/Components/DoubleClickButton.h>
#include <Views/Exposure/SaveExposureSettingsView.h>

#define ID_BTN_HOME     "H"
#define ID_BTN_BACK     "B"
#define ID_BTN_SAVE     "S"
#define ID_BTN_VALIDATE "V"
#define BUTTONS_SIZE    150

SaveExposureSettingsView::SaveExposureSettingsView(Unique<SaveExposureSettingsViewModel> viewModel, QWidget *parent) :
    ExposureViewBase(std::move(viewModel), parent),
    m_keyboard(this),
    ui(new Ui::SaveExposureSettingsView)
{
    ui->setupUi(this);
    m_keyboard.setupKeyboardConnections(this);

    QVBoxLayout *layout = new QVBoxLayout(ui->listPrestsQWidget);
    ui->listPrestsQWidget->setLayout(layout);

    setNewNavButtonsConfigs();
    createNavButtonsConfigs();
    setDefaultTitleBar("Save parameters");
    connect(ui->btnConfirm, &QPushButton::clicked, this, &SaveExposureSettingsView::onConfirmButtonClicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &SaveExposureSettingsView::onBackButtonClicked);

    connect(getViewModel<SaveExposureSettingsViewModel>(), &SaveExposureSettingsViewModel::s_presetSaved, this, &SaveExposureSettingsView::s_onPresetSaved);
    connect(getViewModel<SaveExposureSettingsViewModel>(), &SaveExposureSettingsViewModel::s_errorSavingPreset, this, &SaveExposureSettingsView::s_onErrorSavingPreset);
}

SaveExposureSettingsView::~SaveExposureSettingsView()
{
}

void SaveExposureSettingsView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void SaveExposureSettingsView::showEvent(QShowEvent *event)
{
    ui->lineEdit->setFocus();
    if (isAPresetSavedInThisSession())
    {
        populateViewWithCurrentPreset();
        setAPresetSavedInThisSession(false);
    }
    QWidget::showEvent(event);
}

void SaveExposureSettingsView::setNewNavButtonsConfigs()
{

    NavButtonConfig backBtn(
        "Back",
        ":/icons/back.svg",
        ID_BTN_BACK,
        std::bind(&SaveExposureSettingsView::onBackButtonClicked, this));
    addNavButton("left", backBtn, 1);
}

void SaveExposureSettingsView::onBackButtonClicked()
{
    emit s_openView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW);
}

void SaveExposureSettingsView::onValidateButtonClicked()
{
}

void SaveExposureSettingsView::onConfirmButtonClicked()
{
    if (ui->lineEdit->text().isEmpty())
    {
        UPDATE_DYNAMIC_PROPERTY(ui->lineEdit, "class", "error-lineEdit");
        ui->lineEdit->setFocus();
        return;
    }

    QString name = ui->lineEdit->text();
    if (m_presetsButton.contains(name))
    {
        showPopUpMessage("Preset Already Exists", "Choose a different name or replacement.", {{"Cancel", []() {}}, {"Replace", [this, name]() { this->userConfirmSaveReplacementPreset(name); }}});
        return;
    }

    auto viewModel = getViewModel<SaveExposureSettingsViewModel>();
    if (viewModel)
    {
        viewModel->userConfirmSavePreset(name);
    }
    ui->lineEdit->clear();
    onBackButtonClicked();
}

void SaveExposureSettingsView::userConfirmSaveReplacementPreset(const QString &name)
{
    auto viewModel = getViewModel<SaveExposureSettingsViewModel>();
    if (viewModel)
    {
        viewModel->userConfirmSavePreset(name);
    }
    onBackButtonClicked();
}

void SaveExposureSettingsView::populateViewWithCurrentPreset()
{
    m_presetsButton.clear();
    clearWidget(ui->listPrestsQWidget);
    auto viewModel = getViewModel<SaveExposureSettingsViewModel>();
    if (!viewModel)
    {
        return;
    }
    QString errorMessage;
    QList<PresetExposure> presets = viewModel->getAllPresets(&errorMessage);

    for (const PresetExposure &preset : presets)
    {
        DoubleClickButton *presetButton = new DoubleClickButton(preset.name);
        m_presetsButton.insert(preset.name, presetButton);
        presetButton->setFixedHeight(50);
        presetButton->setProperty("class", "button-blue");
        ui->listPrestsQWidget->layout()->addWidget(presetButton);
        connect(presetButton, &DoubleClickButton::doubleClicked, this, [this, preset]() {
            ui->lineEdit->setText(preset.name);
            ui->lineEdit->setFocus();
        });
    }
}

void SaveExposureSettingsView::s_onPresetSaved()
{
    setAPresetSavedInThisSession(true);
    // showPopUpMessage("Preset Saved", "The preset has been saved successfully.", {{"OK", []() {}}});
}

void SaveExposureSettingsView::s_onErrorSavingPreset(const QString &errorMessage)
{
    showPopUpMessage("Error Saving Preset", errorMessage, {{"OK", []() {}}});
}

void SaveExposureSettingsView::setAPresetSavedInThisSession(bool saved)
{
    m_PresetSaved = saved;
}

bool SaveExposureSettingsView::isAPresetSavedInThisSession() const
{
    return m_PresetSaved;
}