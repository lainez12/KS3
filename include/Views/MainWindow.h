#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMap>
#include <QPixmap>
#include <QPointer>

#include "Views/Components/NavButton.h"
#include "Views/Components/UpBar.h"
#include "Views/DebugView.h"
#include "Views/HomeView.h"
#include "Views/SettingsView.h"
#include "Views/ViewBase.h"
#include "Views/ViewsExposure/CompleteExposureView.h"
#include "Views/ViewsExposure/ExposureSettingsView.h"
#include "Views/ViewsExposure/FavoriteExposureSettingsView.h"
#include "Views/ViewsExposure/ProgressExposureView.h"
#include "Views/ViewsExposure/RecapExposureSettingsView.h"
#include "Views/ViewsExposure/SaveExposureSettingsView.h"
#include "Views/ViewsSettings/LedTestView.h"
#include "Views/ViewsSettings/MachineStatusView.h"
#include "Views/ViewsSettings/OperatingTimesView.h"
#include "Views/ViewsSettings/ScreenshotExportView.h"
#include "Views/ViewsSettings/TemperatureView.h"
#include "Views/ViewsSettings/UpdateSoftwareView.h"
#include "Views/ViewsSettings/VersionView.h"
#include "Views/ViewsAlignment/DistanceView.h"
#include "Views/ViewsAlignment/VisualisationView.h"
#include "Views/ViewsAlignment/LoadParametersView.h"
#include "Views/ViewsAlignment/SaveParametersView.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    void addView(Kub3::UI::ViewId viewId, Kub3::UI::Views::ViewBase *view);

signals:
    void s_initializationRequest(void);

public slots:
    void ps_openView(Kub3::UI::ViewId viewId);
    void ps_stateChanged(const QString &stateName);

private slots:
    void onViewButtonConfigsUpdated();
    void onViewButtonStateChanged(const QString &buttonId, bool newState);
    void onViewButtonTextChanged(const QString &buttonId, const QString &newText);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void updateTopBar(Kub3::UI::Views::ViewBase *view);
    void updateBottomBar(Kub3::UI::Views::ViewBase *view);
    void clearBottomBar();
    void showLogoIfNeeded(Kub3::UI::Views::ViewBase *view);
    NavButton *createNavButton(const Kub3::UI::Views::NavButtonConfig &config);

    void connectViewSignals(Kub3::UI::Views::ViewBase *view);
    void disconnectViewSignals(Kub3::UI::Views::ViewBase *view);

private:
    Ui::MainWindow *ui;
    QPixmap m_backgroundPixmap;
    std::unordered_map<Kub3::UI::ViewId, Kub3::UI::Views::ViewBase *> m_views;
    UpBar *m_topBar       = nullptr;
    QLabel *m_topBarTitle = nullptr;

    QHBoxLayout *m_bottomBarLeft   = nullptr;
    QHBoxLayout *m_bottomBarCenter = nullptr;
    QHBoxLayout *m_bottomBarRight  = nullptr;

    struct NavButtonEntry {
        QPointer<NavButton> button = nullptr;
        QString position;
    };
    QMap<QString, NavButtonEntry> m_bottomBarButtons;

    Kub3::UI::Views::ViewBase *m_currentView = nullptr;
};

#endif // MAINWINDOW_H
