#include "Views/ViewsSettings/ScreenshotExportView.h"
#include "ui_ScreenshotExportView.h"

ScreenshotExportView::ScreenshotExportView(Unique<ScreenshotExportViewModel> viewModel, QWidget *parent) :
    SettingsViewBase(std::move(viewModel), parent),
    ui(new Ui::ScreenshotExportView) {
    ui->setupUi(this);

    setDefaultTitleBar("Screenshot Export");
}
ScreenshotExportView::~ScreenshotExportView() {
}

void ScreenshotExportView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}