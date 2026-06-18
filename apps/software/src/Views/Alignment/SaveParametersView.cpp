#include <QString>

#include <Views/Alignment/SaveParametersView.h>
#include <Views/Components/Colors.h>

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
    setDefaultTitleBar("Exposure Settings");
}

SaveParametersView::~SaveParametersView()
{
    delete ui;
}

void SaveParametersView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void SaveParametersView::setNewNavButtonsConfigs()
{
}

void SaveParametersView::onBackButtonClicked(const QString &buttonId)
{
}

void SaveParametersView::onValidateButtonClicked(const QString &buttonId)
{
}