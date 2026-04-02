#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "Views/DebugView.h"
#include "Views/MachineStatusView.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void s_initializationRequest(void);

public slots:
    void ps_stateChanged(const QString &stateName);

private slots:
    void goBackHome(void);
    void openMachineStatusView(void);

private:
    Ui::MainWindow *ui;

    DebugView *m_debugView;
    int m_debugViewIndex;

    MachineStatusView *m_machineStatusView;
    int m_machineStatusViewIndex;
};

#endif // MAINWINDOW_H
