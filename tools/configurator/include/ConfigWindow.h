#pragma once

#include <QWidget>
#include <functional>
#include <memory>
#include <vector>

#include <Config/machine_config.h>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class ConfigWindow;
}
QT_END_NAMESPACE

class ConfigWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigWindow(QString hwConfigPath, QString processConfigPath, QWidget *parent = nullptr);
    ~ConfigWindow() override;

private slots:
    void onSaveClicked();
    void onReloadClicked();

private:
    void populateUI();
    void clearUI();

    // Helper to add a view to the list and stack cleanly
    void addConfigPage(const QString &menuLabel, QWidget *pageWidget, std::function<void()> saveCallback);

private:
    Ui::ConfigWindow *ui;

    Kub3::Config::hardware_config_t m_hwConfig;
    Kub3::Config::process_config_t m_processConfig;

    QString m_hwConfigPath;
    QString m_processConfigPath;

    // A list of closures that pull data from the UI back into the structs
    std::vector<std::function<void()>> m_saveHooks;
};
