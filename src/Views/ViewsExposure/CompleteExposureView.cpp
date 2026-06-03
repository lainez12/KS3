#include "Views/ViewsExposure/CompleteExposureView.h"
#include "Views/Components/Colors.h"
#include "ui_CompleteExposureView.h"
#include <QString>

#define ID_BTN_BACK "B"
#define ID_BTN_SAVE "S"
#define ID_BTN_HOME "H"

CompleteExposureView::CompleteExposureView(Unique<CompleteExposureViewModel> viewModel, QWidget *parent) :
    ExposureViewBase(std::move(viewModel), parent),
    ui(new Ui::CompleteExposureView) {
    ui->setupUi(this);
    setNewNavButtonsConfigs();
    setDefaultTitleBar("Exposure Settings");
}
CompleteExposureView::~CompleteExposureView() {
}

void CompleteExposureView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}

void CompleteExposureView::setNewNavButtonsConfigs() {
    NavButtonConfig homeBtn(
        "Home",
        ":/icons/home.svg",
        "H",
        std::bind(&CompleteExposureView::onHomeButtonClicked, this, std::placeholders::_1));
    addNavButton("left", homeBtn);
}

void CompleteExposureView::onBackButtonClicked(const QString &buttonId) {
    // emit s_openView(Kub3::UI::ViewId::BACK_VIEW);
}

void CompleteExposureView::onValidateButtonClicked(const QString &buttonId) {
    // emit s_openView(Kub3::UI::ViewId::VALIDATE_VIEW);
}