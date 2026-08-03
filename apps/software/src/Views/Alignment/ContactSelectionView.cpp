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
    connect(ui->pushButton_SoftContact, &QPushButton::clicked, this, [this]() {
        setNavButtonEnabled(ID_BTN_VALIDATE, true);
        ui->labelDescription->setText("This mode brings your substrate into contact with the mask with low pressure.\nAfter alignment, go to Force panel to initiate the lift of your substrate to the factory preset value.");
    });
    connect(ui->pushButton_HardContact, &QPushButton::clicked, this, [this]() {
        setNavButtonEnabled(ID_BTN_VALIDATE, true);
        ui->labelDescription->setText("This mode brings your substrate into contact with the mask with higher pressure.\nAfter alignment, go to Force panel to select the desired force value and initiate the lift of your substrate to that value.");
    });
}
ContactSelectionView::~ContactSelectionView()
{
}

void ContactSelectionView::setNewNavButtonsConfigs()
{
}

void ContactSelectionView::onBackButtonClicked()
{
    emit s_openView(Kub3::UI::ViewId::EXPOSURE_MODE_VIEW);
}

void ContactSelectionView::onValidateButtonClicked()
{
    auto vm = getViewModel<ContactSelectionViewModel>();
    if (!vm)
    {
        qCritical() << "[ContactSelectionView] ViewModel is null.";
        return;
    }
    vm->ui_setSelectedContactMode(ui->pushButton_SoftContact->isChecked() ? ContactSelectionViewModel::ContactMode::SOFT : ContactSelectionViewModel::ContactMode::HARD);
    emit s_openView(Kub3::UI::ViewId::ALIGNMENT_VISUALISATION_VIEW);
}