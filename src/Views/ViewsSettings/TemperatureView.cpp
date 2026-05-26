#include "Views/ViewsSettings/TemperatureView.h"
#include "Views/Colors.h"
#include "ui_TemperatureView.h"
#include <QString>

#define ID_BTN_HOME "H"
#define ID_BTN_BACK "B"

TemperatureView::TemperatureView(Unique<TemperatureViewModel> viewModel, QWidget *parent) :
    SettingsViewBase(std::move(viewModel), parent),
    ui(new Ui::TemperatureView) {
    ui->setupUi(this);

    setDefaultTitleBar("Temperature");
}
TemperatureView::~TemperatureView() {
}

void TemperatureView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}