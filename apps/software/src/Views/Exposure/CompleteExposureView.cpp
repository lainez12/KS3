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
    setDefaultTitleBar("Exposure complete");
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
    NavButtonConfig backToMainBtn(
        "Back to Main",
        QColor("#FFF"),
        QColor("#B2D4F4"),
        ":/icons/picto.png",
        "B",
        std::bind(&CompleteExposureView::onBackToMainButtonClicked, this));
    backToMainBtn.isTextColorDifferent = true;
    backToMainBtn.textColor            = QColor("#000");
    addNavButton("left", backToMainBtn);
}

void CompleteExposureView::onValidateButtonClicked()
{
}

void CompleteExposureView::onBackToMainButtonClicked()
{
    emit s_openView(Kub3::UI::ViewId::HOME_VIEW);
}