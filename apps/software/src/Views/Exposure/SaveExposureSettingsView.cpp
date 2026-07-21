#include "ui_SaveExposureSettingsView.h"
#include <QGraphicsDropShadowEffect>
#include <QRegularExpression>
#include <QString>
#include <QStyle>
#include <Views/Components/Colors.h>
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

    setNewNavButtonsConfigs();
    createNavButtonsConfigs();
    setDefaultTitleBar("Save parameters");
    connect(ui->btnConfirm, &QPushButton::clicked, this, &SaveExposureSettingsView::onConfirmButtonClicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &SaveExposureSettingsView::onBackButtonClicked);
    populateViewWithCurrentPreset();
}

SaveExposureSettingsView::~SaveExposureSettingsView()
{
}

void SaveExposureSettingsView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
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
        return;
    }

    QString name   = ui->lineEdit->text();
    auto viewModel = getViewModel<SaveExposureSettingsViewModel>();
    if (viewModel)
    {
        viewModel->userConfirmSavePreset(name);
    }
}

void SaveExposureSettingsView::populateViewWithCurrentPreset()
{
    auto viewModel = getViewModel<SaveExposureSettingsViewModel>();
    if (!viewModel)
    {
        return;
    }
    QString errorMessage;
    QList<PresetExposure> presets = viewModel->getAllPresets(&errorMessage);
    QVBoxLayout *layout           = new QVBoxLayout(ui->listPrestsQWidget);
    ui->listPrestsQWidget->setLayout(layout);
    for (const PresetExposure &preset : presets)
    {
        QPushButton *presetButton = new QPushButton(preset.name);
        presetButton->setFixedHeight(50);
        presetButton->setProperty("class", "button-blue");
        ui->listPrestsQWidget->layout()->addWidget(presetButton);
    }
}