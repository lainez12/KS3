#include "ui_VersionView.h"
#include <QString>
#include <Views/Components/Colors.h>
#include <Views/Settings/VersionView.h>

#define ID_BTN_HOME "H"
#define ID_BTN_BACK "B"

VersionView::VersionView(Unique<VersionViewModel> viewModel, QWidget *parent) :
    SettingsViewBase(std::move(viewModel), parent),
    ui(new Ui::VersionView)
{
    ui->setupUi(this);
    setDefaultTitleBar("Version Note");
}
VersionView::~VersionView()
{
}

void VersionView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}