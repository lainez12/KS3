#include <QString>

#include "ui_ContactSelectionView.h"
#include <Views/Alignment/ContactSelectionView.h>
#include <Views/Components/Colors.h>

#define ID_BTN_BACK     "B"
#define ID_BTN_SAVE     "S"
#define ID_BTN_HOME     "H"
#define ID_BTN_VALIDATE "V"

ContactSelectionView::ContactSelectionView(Unique<ContactSelectionViewModel> viewModel, QWidget *parent) :
    AlignmentViewBase(std::move(viewModel), parent),
    ui(new Ui::ContactSelectionView)
{
    ui->setupUi(this);
    setDefaultTitleBar("Contact Selection");
    createNavButtonsConfigs();
    setNavButtonEnabled(ID_BTN_VALIDATE, true);
}
ContactSelectionView::~ContactSelectionView()
{
}

void ContactSelectionView::setNewNavButtonsConfigs()
{
}

void ContactSelectionView::onBackButtonClicked(const QString &buttonId)
{
    emit s_openView(Kub3::UI::ViewId::HOME_VIEW);
}

void ContactSelectionView::onValidateButtonClicked(const QString &buttonId)
{
    emit s_openView(Kub3::UI::ViewId::ALIGNMENT_VISUALISATION_VIEW);
}