#include "ui_OperatingTimesView.h"
#include <Views/Settings/OperatingTimesView.h>

OperatingTimesView::OperatingTimesView(Unique<OperatingTimesViewModel> viewModel, QWidget *parent) :
    SettingsViewBase(std::move(viewModel), parent),
    ui(new Ui::OperatingTimesView)
{
    ui->setupUi(this);

    setDefaultTitleBar("Operating Times");
}
OperatingTimesView::~OperatingTimesView()
{
}

void OperatingTimesView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}