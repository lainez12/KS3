#include "ui_MainWindow.h"

#include "Views/MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Debug View
    {
        m_debugView      = new DebugView(this);
        m_debugViewIndex = ui->stackedWidget->addWidget(m_debugView);

        connect(m_debugView, &DebugView::s_openMachineStatus, this, &MainWindow::openMachineStatusView);
        connect(m_debugView, &DebugView::s_initializationRequest, this, &MainWindow::s_initializationRequest);
    }

    // Machine Status View
    {
        m_machineStatusView      = new MachineStatusView(this);
        m_machineStatusViewIndex = ui->stackedWidget->addWidget(m_machineStatusView);

        connect(m_machineStatusView, &MachineStatusView::s_home, this, &MainWindow::goBackHome);
    }

    ui->stackedWidget->setCurrentIndex(m_debugViewIndex);
}

void MainWindow::ps_stateChanged(const QString &stateName)
{
    m_debugView->updateMachineState(stateName);
}

void MainWindow::goBackHome(void)
{
    ui->stackedWidget->setCurrentIndex(m_debugViewIndex);
}

void MainWindow::openMachineStatusView(void)
{
    ui->stackedWidget->setCurrentIndex(m_machineStatusViewIndex);
}

MainWindow::~MainWindow()
{
    delete ui;
}
