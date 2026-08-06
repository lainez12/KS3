#include <QDebug>
#include <QString>

#include <Views/Alignment/RenameParametersView.h>
#include <Views/Components/Colors.h>
#include <Views/Components/DoubleClickButton.h>

#include "ui_RenameParametersView.h"

#define ID_BTN_BACK "B"
#define ID_BTN_SAVE "S"
#define ID_BTN_HOME "H"

RenameParametersView::RenameParametersView(Unique<RenameParametersViewModel> viewModel, QWidget *parent) :
    AlignmentViewBase(std::move(viewModel), parent),
    m_keyboard(this),
    ui(new Ui::RenameParametersView)
{
    ui->setupUi(this);
    m_keyboard.setupKeyboardConnections(this);
    setDefaultTitleBar("Rename parameters");
    setNewNavButtonsConfigs();

    connect(ui->btnCancel, &QPushButton::clicked, this, &RenameParametersView::onBackButtonClicked);
    connect(ui->btnConfirm, &QPushButton::clicked, this, &RenameParametersView::onConfirmButtonClicked);

    auto vm = getViewModel<RenameParametersViewModel>();
    if (vm)
    {
        connect(vm, &RenameParametersViewModel::s_parameterRenamed, this, &RenameParametersView::ps_onParameterRenamed);
        connect(vm, &RenameParametersViewModel::s_errorRenamingParameter, this, &RenameParametersView::ps_onErrorRenamingParameter);
    }
}

RenameParametersView::~RenameParametersView()
{
    delete ui;
}

void RenameParametersView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void RenameParametersView::showEvent(QShowEvent *event)
{
    auto vm = getViewModel<RenameParametersViewModel>();
    if (vm)
    {
        ui->lineEdit->setText(vm->getCurrentNameParameter());
    }
    ui->lineEdit->setFocus();
    AlignmentViewBase::showEvent(event);
}

void RenameParametersView::ps_onErrorRenamingParameter(const QString &errorMessage)
{
    ps_createPopUp(
        "Error Renaming Parameter",
        {
            {"OK", [this]() { ps_closePopUp(); }},
        });
    qWarning() << "Error renaming parameter:" << errorMessage;
}

void RenameParametersView::setNewNavButtonsConfigs()
{
    NavButtonConfig backBtn(
        "Back",
        ":/icons/back.svg",
        "B",
        std::bind(&RenameParametersView::onBackButtonClicked, this));
    addNavButton("left", backBtn);
}

void RenameParametersView::onBackButtonClicked()
{
    emit s_openView(Kub3::UI::ViewId::ALIGNMENT_LOAD_PARAMETERS_VIEW);
}

void RenameParametersView::onValidateButtonClicked()
{
}

void RenameParametersView::onConfirmButtonClicked()
{
    if (ui->lineEdit->text().isEmpty())
    {
        UPDATE_DYNAMIC_PROPERTY(ui->lineEdit, "class", "error-lineEdit");
        ui->lineEdit->setFocus();
        return;
    }

    QString name = ui->lineEdit->text();
    auto vm      = getViewModel<RenameParametersViewModel>();
    if (!vm)
    {
        return;
    }
    QString currentName = vm->getCurrentNameParameter();
    if (currentName == name)
    {
        onBackButtonClicked();
        return;
    }
    vm->ui_userRequestRenameParameters(name);
}

void RenameParametersView::ps_onParameterRenamed()
{
    ui->lineEdit->clear();
    onBackButtonClicked();
}