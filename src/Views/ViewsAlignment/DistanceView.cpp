#include "Views/ViewsAlignment/DistanceView.h"
#include "Views/Components/Colors.h"
#include "ui_DistanceView.h"
#include <QString>

#define ID_BTN_BACK "B"
#define ID_BTN_SAVE "S"
#define ID_BTN_HOME "H"

DistanceView::DistanceView(Unique<DistanceViewModel> viewModel, QWidget *parent) :
    AlignmentViewBase(std::move(viewModel), parent),
    ui(new Ui::DistanceView) {
    ui->setupUi(this);
    setDefaultTitleBar("Exposure Settings");
}
DistanceView::~DistanceView() {
}

void DistanceView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}