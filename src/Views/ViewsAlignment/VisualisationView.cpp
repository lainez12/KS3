#include "Views/ViewsAlignment/VisualisationView.h"
#include "Views/Components/Colors.h"
#include "ui_VisualisationView.h"
#include <QString>

#define ID_BTN_BACK "B"
#define ID_BTN_SAVE "S"
#define ID_BTN_HOME "H"

VisualisationView::VisualisationView(Unique<VisualisationViewModel> viewModel, QWidget *parent) :
    AlignmentViewBase(std::move(viewModel), parent),
    ui(new Ui::VisualisationView) {
    ui->setupUi(this);
    setDefaultTitleBar("Exposure Settings");
}
VisualisationView::~VisualisationView() {
}

void VisualisationView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}