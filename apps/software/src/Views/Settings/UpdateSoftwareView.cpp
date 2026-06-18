#include "ui_UpdateSoftwareView.h"
#include <Views/Settings/UpdateSoftwareView.h>

UpdateSoftwareView::UpdateSoftwareView(Unique<UpdateSoftwareViewModel> viewModel, QWidget *parent) :
    SettingsViewBase(std::move(viewModel), parent),
    ui(new Ui::UpdateSoftwareView)
{
    ui->setupUi(this);

    setDefaultTitleBar("Update Software");
}
UpdateSoftwareView::~UpdateSoftwareView()
{
}

void UpdateSoftwareView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}