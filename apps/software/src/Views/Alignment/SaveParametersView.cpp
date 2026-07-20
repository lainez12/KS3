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
    setNewNavButtonsConfigs();
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