#include "Views/MachineStatusView.h"
#include "ui_MachineStatusView.h"

MachineStatusView::MachineStatusView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MachineStatusView)
{
    ui->setupUi(this);
}

MachineStatusView::~MachineStatusView()
{
    delete ui;
}

void MachineStatusView::on_goBackBtn_clicked(void)
{
    emit s_home();
}
