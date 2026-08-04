#include <QString>

#include <QDebug>
#include <Views/Alignment/SaveParametersView.h>
#include <Views/Components/Colors.h>
#include <Views/Components/DoubleClickButton.h>

#include "ui_SaveParametersView.h"

#define ID_BTN_BACK "B"
#define ID_BTN_SAVE "S"
#define ID_BTN_HOME "H"

SaveParametersView::SaveParametersView(Unique<SaveParametersViewModel> viewModel, QWidget *parent) :
    AlignmentViewBase(std::move(viewModel), parent),
    m_keyboard(this),
    ui(new Ui::SaveParametersView)
{
    ui->setupUi(this);
    m_keyboard.setupKeyboardConnections(this);
    setDefaultTitleBar("Exposure settings");
    setNewNavButtonsConfigs();

    QVBoxLayout *layout = new QVBoxLayout(ui->parametersListWidget);
    ui->parametersListWidget->setLayout(layout);
    connect(ui->btnCancel, &QPushButton::clicked, this, &SaveParametersView::onBackButtonClicked);
    connect(ui->btnConfirm, &QPushButton::clicked, this, &SaveParametersView::onConfirmButtonClicked);

    auto vm = getViewModel<SaveParametersViewModel>();
    if (vm)
    {
        connect(vm, &SaveParametersViewModel::s_parameterSaved, this, &SaveParametersView::ps_onParameterSaved);
        connect(vm, &SaveParametersViewModel::s_errorSavingParameter, this, &SaveParametersView::ps_onErrorSavingParameter);
    }
}

SaveParametersView::~SaveParametersView()
{
    delete ui;
}

void SaveParametersView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void SaveParametersView::showEvent(QShowEvent *event)
{
    ui->lineEdit->setFocus();
    if (isAParameterSavedInThisSession())
    {
        populateViewWithCurrentParameter();
        setAParameterSavedInThisSession(false);
    }
    AlignmentViewBase::showEvent(event);
}

void SaveParametersView::ps_onParameterSaved()
{
    setAParameterSavedInThisSession(true);
}

void SaveParametersView::ps_onErrorSavingParameter(const QString &errorMessage)
{
    ps_createPopUp(
        "Error Saving Parameter",
        {
            {"OK", [this]() { ps_closePopUp(); }},
        });
    qWarning() << "Error saving parameter:" << errorMessage;
}

void SaveParametersView::setNewNavButtonsConfigs()
{
    NavButtonConfig backBtn(
        "Back",
        ":/icons/back.svg",
        "B",
        std::bind(&SaveParametersView::onBackButtonClicked, this));
    addNavButton("left", backBtn);
}

void SaveParametersView::onBackButtonClicked()
{
    emit s_openView(Kub3::UI::ViewId::ALIGNMENT_VISUALISATION_VIEW);
}

void SaveParametersView::onValidateButtonClicked()
{
}

void SaveParametersView::onConfirmButtonClicked()
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
                {"Cancel", [this]() { ps_closePopUp(); }},
                {"Replace", [this, name]() { this->userConfirmSaveReplacementPreset(name); }},
            },
            "Choose a different name or replacement.");
        return;
    }
    auto vm = getViewModel<SaveParametersViewModel>();
    if (vm)
    {
        vm->ui_userRequestSaveParameters(name, true);
    }
    ui->lineEdit->clear();
    onBackButtonClicked();
}

void SaveParametersView::userConfirmSaveReplacementPreset(const QString &name)
{
    auto vm = getViewModel<SaveParametersViewModel>();

    if (vm)
    {
        vm->ui_userRequestSaveParameters(name, true);
    }

    ui->lineEdit->clear();
    ps_closePopUp();
    onBackButtonClicked();
}

void SaveParametersView::setAParameterSavedInThisSession(bool saved)
{
    m_parameterSaved = saved;
}

bool SaveParametersView::isAParameterSavedInThisSession() const
{
    return m_parameterSaved;
}

void SaveParametersView::populateViewWithCurrentParameter()
{
    auto vm = getViewModel<SaveParametersViewModel>();

    if (!vm)
    {
        return;
    }

    auto res = vm->getAllNamesSavedParameters();
    if (res.is_err())
    {
        qWarning() << "Failed to load parameters:" << res.unwrap_err();
        return;
    }

    const auto &parametersArray = *res;
    clearWidget(ui->parametersListWidget);

    for (const QString &value : parametersArray)
    {
        DoubleClickButton *presetButton = new DoubleClickButton(value);
        m_presetsButton.insert(value, presetButton);
        presetButton->setFixedHeight(50);
        presetButton->setProperty("class", "button-blue");
        ui->parametersListWidget->layout()->addWidget(presetButton);
        connect(presetButton, &DoubleClickButton::doubleClicked, this, [this, value]() {
            ui->lineEdit->setText(value);
            ui->lineEdit->setFocus();
        });
    }
}