#include "ui_CompleteExposureView.h"
#include <QString>
#include <Views/Components/Colors.h>
#include <Views/Exposure/CompleteExposureView.h>

#define ID_BTN_BACK "B"
#define ID_BTN_SAVE "S"
#define ID_BTN_HOME "H"

CompleteExposureView::CompleteExposureView(Unique<CompleteExposureViewModel> viewModel, QWidget *parent) :
    ExposureViewBase(std::move(viewModel), parent),
    ui(new Ui::CompleteExposureView)
{
    ui->setupUi(this);
    setNewNavButtonsConfigs();
    setDefaultTitleBar("Exposure Settings");
}
CompleteExposureView::~CompleteExposureView()
{
}

void CompleteExposureView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void CompleteExposureView::setNewNavButtonsConfigs()
{
    NavButtonConfig homeBtn(
        "Home",
        ":/icons/home.svg",
        "H",
        std::bind(&CompleteExposureView::onHomeButtonClicked, this));
    addNavButton("left", homeBtn);
}

void CompleteExposureView::onValidateButtonClicked()
{
}