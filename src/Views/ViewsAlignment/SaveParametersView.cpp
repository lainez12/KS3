#include "Views/ViewsAlignment/SaveParametersView.h"
#include "Views/Components/Colors.h"
#include "ui_SaveParametersView.h"
#include <QString>

#define ID_BTN_BACK "B"
#define ID_BTN_SAVE "S"
#define ID_BTN_HOME "H"

SaveParametersView::SaveParametersView(Unique<SaveParametersViewModel> viewModel, QWidget *parent) :
    AlignmentViewBase(std::move(viewModel), parent),
    ui(new Ui::SaveParametersView) {
    ui->setupUi(this);
    setDefaultTitleBar("Exposure Settings");
}
SaveParametersView::~SaveParametersView() {
}

void SaveParametersView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}

void SaveParametersView::setNewNavButtonsConfigs() {
}

void SaveParametersView::onBackButtonClicked(const QString &buttonId) {
}

void SaveParametersView::onValidateButtonClicked(const QString &buttonId) {
}