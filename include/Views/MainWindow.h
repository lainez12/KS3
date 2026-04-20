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

namespace Kub3::UI
{
    enum class ViewId
    {
        HOME_VIEW,
        ALIGNMENT_VIEW,
        MACHINE_STATUS_VIEW
    };
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    void addView(Kub3::UI::ViewId viewId, QWidget *view);

signals:
    void s_initializationRequest(void);

public slots:
    void ps_openView(Kub3::UI::ViewId viewId);
    void ps_stateChanged(const QString &stateName);

private slots:
    void goBackHome(void);
    void openMachineStatusView(void);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::MainWindow *ui;
    QPixmap m_backgroundPixmap;
    std::unordered_map<Kub3::UI::ViewId, QWidget *> m_views;
};

#endif // MAINWINDOW_H
