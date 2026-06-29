#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMap>
#include <QPixmap>
#include <QPointer>

#include <Views/Alignment/ContactSelectionView.h>
#include <Views/Alignment/DistanceView.h>
#include <Views/Alignment/LoadParametersView.h>
#include <Views/Alignment/SaveParametersView.h>
#include <Views/Alignment/VisualisationView.h>
#include <Views/Components/NavButton.h>
#include <Views/Components/UpBar.h>
#include <Views/Exposure/CompleteExposureView.h>
#include <Views/Exposure/ExposureSettingsView.h>
#include <Views/Exposure/FavoriteExposureSettingsView.h>
#include <Views/Exposure/ProgressExposureView.h>
#include <Views/Exposure/RecapExposureSettingsView.h>
#include <Views/Exposure/SaveExposureSettingsView.h>
#include <Views/ExposureMenuView.h>
#include <Views/HomeEightView.h>
#include <Views/HomeView.h>
#include <Views/Settings/LedTestView.h>
#include <Views/Settings/MachineStatusView.h>
#include <Views/Settings/OperatingTimesView.h>
#include <Views/Settings/ScreenshotExportView.h>
#include <Views/Settings/TemperatureView.h>
#include <Views/Settings/UpdateSoftwareView.h>
#include <Views/Settings/VersionView.h>
#include <Views/SettingsView.h>
#include <Views/ViewBase.h>

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
    void changeButtonColor(const QString &buttonId, bool EnabledColor);
    void onViewButtonTextChanged(const QString &buttonId, const QString &newText);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateTopBar(Kub3::UI::Views::ViewBase *view);
    void updateBottomBar(Kub3::UI::Views::ViewBase *view);
    void clearBottomBar();
    void showLogoIfNeeded(Kub3::UI::Views::ViewBase *view);
    NavButton *createNavButton(const Kub3::UI::Views::NavButtonConfig &config);

    void connectViewSignals(Kub3::UI::Views::ViewBase *view);
    void disconnectViewSignals(Kub3::UI::Views::ViewBase *view);

    void switchShadow(bool enabled);

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
