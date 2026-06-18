#include "ui_TemperatureView.h"
#include <Views/Settings/TemperatureView.h>

TemperatureView::TemperatureView(Unique<TemperatureViewModel> viewModel, QWidget *parent) :
    SettingsViewBase(std::move(viewModel), parent),
    ui(new Ui::TemperatureView)
{
    ui->setupUi(this);

    setDefaultTitleBar("Temperature");
}
TemperatureView::~TemperatureView()
{
}

void TemperatureView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}