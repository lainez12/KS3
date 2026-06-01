#include "Views/ViewsAlignment/LoadParametersView.h"
#include "Views/Components/Colors.h"
#include "ui_LoadParametersView.h"
#include <QString>

#define ID_BTN_BACK "B"
#define ID_BTN_SAVE "S"
#define ID_BTN_HOME "H"

LoadParametersView::LoadParametersView(Unique<LoadParametersViewModel> viewModel, QWidget *parent) :
    AlignmentViewBase(std::move(viewModel), parent),
    ui(new Ui::LoadParametersView) {
    ui->setupUi(this);
    setDefaultTitleBar("Exposure Settings");
}
LoadParametersView::~LoadParametersView() {
}

void LoadParametersView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}