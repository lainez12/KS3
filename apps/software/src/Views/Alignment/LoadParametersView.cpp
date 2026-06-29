#include <QString>

#include <Views/Alignment/LoadParametersView.h>
#include <Views/Components/Colors.h>

#include "ui_LoadParametersView.h"

#define ID_BTN_BACK "B"
#define ID_BTN_SAVE "S"
#define ID_BTN_HOME "H"

LoadParametersView::LoadParametersView(Unique<LoadParametersViewModel> viewModel, QWidget *parent) :
    AlignmentViewBase(std::move(viewModel), parent),
    ui(new Ui::LoadParametersView)
{
    ui->setupUi(this);
    setDefaultTitleBar("Exposure Settings");
    setNewNavButtonsConfigs();
}
LoadParametersView::~LoadParametersView()
{
}

void LoadParametersView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void LoadParametersView::setNewNavButtonsConfigs()
{
    NavButtonConfig backBtn(
        "Back",
        ":/icons/back.svg",
        "B",
        std::bind(&LoadParametersView::onBackButtonClicked, this, std::placeholders::_1));
    addNavButton("left", backBtn);
}

void LoadParametersView::onBackButtonClicked(const QString &buttonId)
{
    emit s_openView(Kub3::UI::ViewId::ALIGNMENT_VISUALISATION_VIEW);
}

void LoadParametersView::onValidateButtonClicked(const QString &buttonId)
{
}