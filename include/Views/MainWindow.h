#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>

#include "Views/DebugView.h"
#include "Views/HomeView.h"
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

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::MainWindow *ui;
    QPixmap m_backgroundPixmap;

    DebugView *m_debugView;
    int m_debugViewIndex;

    HomeView *m_homeView;
    int m_homeViewIndex;

    MachineStatusView *m_machineStatusView;
    int m_machineStatusViewIndex;
};

#endif // MAINWINDOW_H
