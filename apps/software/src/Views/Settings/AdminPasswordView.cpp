#include <QString>

#include <Views/Components/Colors.h>
#include <Views/Settings/AdminPasswordView.h>

#include "ui_AdminPasswordView.h"

#define ID_BTN_BACK "B"
#define ID_BTN_SAVE "S"
#define ID_BTN_HOME "H"

AdminPasswordView::AdminPasswordView(Unique<AdminPasswordViewModel> viewModel, QWidget *parent) :
    SettingsViewBase(std::move(viewModel), parent),
    m_keyboard(this),
    ui(new Ui::AdminPasswordView)
{
    ui->setupUi(this);
    m_keyboard.setupKeyboardConnections(this);
    setDefaultTitleBar("Admin Password");
    setNewNavButtonsConfigs();

    if (!m_viewModel)
        return;
    AdminPasswordViewModel *vm = static_cast<AdminPasswordViewModel *>(m_viewModel.get());

    connect(vm, &AdminPasswordViewModel::s_authenticationSuccess, this, &AdminPasswordView::onBackButtonClicked);

    connect(ui->btnConfirm, &QPushButton::clicked, this, &AdminPasswordView::onValidateButtonClicked);
}

AdminPasswordView::~AdminPasswordView()
{
    delete ui;
}

void AdminPasswordView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void AdminPasswordView::setNewNavButtonsConfigs()
{
}

void AdminPasswordView::onValidateButtonClicked(void)
{
    if (!m_viewModel)
        return;
    AdminPasswordViewModel *vm = static_cast<AdminPasswordViewModel *>(m_viewModel.get());

    vm->submitPassword(ui->lineEditPasswd->text());
}