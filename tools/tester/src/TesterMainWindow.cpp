#include "ui_TesterMainWindow.h"

#include <HAL/MachineStatus/sensors_labels.h>
#include <TesterMainWindow.h>
#include <view_models/MotorTestViewModel.h>
#include <view_models/ProcedureTestViewModel.h>

#define SUCCESS_QLABEL_STYLESHEET "QLabel{ background: green; color : white; }"
#define FAILURE_QLABEL_STYLESHEET "QLabel{ background: red; color : white; }"

namespace Kub3::Tools::Tester
{

    TesterMainWindow::TesterMainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(std::make_unique<Ui::TesterMainWindow>())
    {
        ui->setupUi(this);

        connect(ui->btnEmergencyStop, &QPushButton::clicked, this, &TesterMainWindow::s_globalEmergencyStopRequested);
    }

    TesterMainWindow::~TesterMainWindow() = default;

    void TesterMainWindow::addView(const QString &tabTitle, QWidget *view)
    {
        if (!view)
            return;
        ui->tabWidget->addTab(view, tabTitle);
    }

} // namespace Kub3::Tools::Tester
