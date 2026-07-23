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

    QVBoxLayout *layout = new QVBoxLayout(ui->presetsListWidget);

    ui->presetsListWidget->setLayout(layout);
    setNewNavButtonsConfigs();
    createNavButtonsConfigs();
    setDefaultTitleBar("Save parameters");
    connect(ui->btnConfirm, &QPushButton::clicked, this, &SaveExposureSettingsView::ps_onConfirmButtonClicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &SaveExposureSettingsView::ps_onBackButtonClicked);

    connect(getViewModel<SaveExposureSettingsViewModel>(), &SaveExposureSettingsViewModel::s_presetSaved, this, &SaveExposureSettingsView::ps_onPresetSaved);
    connect(getViewModel<SaveExposureSettingsViewModel>(), &SaveExposureSettingsViewModel::s_errorSavingPreset, this, &SaveExposureSettingsView::ps_onErrorSavingPreset);
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
        std::bind(&SaveExposureSettingsView::ps_onBackButtonClicked, this));

    addNavButton("left", backBtn, 1);
}

void SaveExposureSettingsView::ps_onBackButtonClicked()
{
    emit s_openView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW);
}

void SaveExposureSettingsView::onValidateButtonClicked()
{
}

void SaveExposureSettingsView::ps_onConfirmButtonClicked()
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
        ps_createPopUpWithText(
            "Preset Already Exists",
            {
                {"Cancel", []() {}},
                {"Replace", [this, name]() { this->userConfirmSaveReplacementPreset(name); }},
            },
            "Choose a different name or replacement.");
        return;
    }

    auto viewModel = getViewModel<SaveExposureSettingsViewModel>();

    if (viewModel)
    {
        viewModel->userConfirmSavePreset(name);
    }
    ui->lineEdit->clear();
    ps_onBackButtonClicked();
}

void SaveExposureSettingsView::userConfirmSaveReplacementPreset(const QString &name)
{
    auto viewModel = getViewModel<SaveExposureSettingsViewModel>();

    if (viewModel)
    {
        viewModel->userConfirmSavePreset(name);
    }

    ui->lineEdit->clear();
    ps_closePopUp();
    ps_onBackButtonClicked();
}

void SaveExposureSettingsView::populateViewWithCurrentPreset()
{
    m_presetsButton.clear();
    clearWidget(ui->presetsListWidget);
    auto viewModel = getViewModel<SaveExposureSettingsViewModel>();

    if (!viewModel)
    {
        return;
    }

    if (auto res = viewModel->getAllPresets())
    {
        const auto &presets = *res;

        for (const PresetExposure &preset : presets)
        {
            DoubleClickButton *presetButton = new DoubleClickButton(preset.name);

            m_presetsButton.insert(preset.name, presetButton);
            presetButton->setFixedHeight(50);
            presetButton->setProperty("class", "button-blue");
            ui->presetsListWidget->layout()->addWidget(presetButton);
            connect(presetButton, &DoubleClickButton::doubleClicked, this, [this, preset]() {
                ui->lineEdit->setText(preset.name);
                ui->lineEdit->setFocus();
            });
        }
    }
    else
    {
        qWarning() << "Failed to load presets:" << res.unwrap_err();
    }
}

void SaveExposureSettingsView::ps_onPresetSaved()
{
    setAPresetSavedInThisSession(true);
}

void SaveExposureSettingsView::ps_onErrorSavingPreset(const QString &errorMessage)
{
    ps_createPopUpWithText("Error Saving Preset", {{"OK", []() {}}}, errorMessage);
}

void SaveExposureSettingsView::setAPresetSavedInThisSession(bool saved)
{
    m_presetSaved = saved;
}

bool SaveExposureSettingsView::isAPresetSavedInThisSession() const
{
    return m_presetSaved;
}