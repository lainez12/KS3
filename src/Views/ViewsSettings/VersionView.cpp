#include "Views/ViewsSettings/VersionView.h"
#include "Views/Colors.h"
#include "ui_VersionView.h"
#include <QString>

#define ID_BTN_HOME "H"
#define ID_BTN_BACK "B"

VersionView::VersionView(Unique<VersionViewModel> viewModel, QWidget *parent) :
    SettingsViewBase(std::move(viewModel), parent),
    ui(new Ui::VersionView) {
    ui->setupUi(this);
    setDefaultTitleBar("Version Note");
}
VersionView::~VersionView() {
}

void VersionView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}