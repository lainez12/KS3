#include "Views/ViewsSettings/LedTestView.h"
#include "ui_LedTestView.h"

LedTestView::LedTestView(Unique<LedTestViewModel> viewModel, QWidget *parent) :
    SettingsViewBase(std::move(viewModel), parent),
    ui(new Ui::LedTestView) {
    ui->setupUi(this);

    setDefaultTitleBar("LED Test");
}
LedTestView::~LedTestView() {
}

void LedTestView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}