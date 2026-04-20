#pragma once

#include <QMainWindow>
#include <memory>

#include <Config/machine_config.h>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class ConfigWindow;
}
QT_END_NAMESPACE

class ConfigWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConfigWindow(QString hwConfigPath, QString processConfigPath, QWidget *parent = nullptr);
    ~ConfigWindow() override;

private slots:
    void onSaveClicked(void);
    void onReloadClicked(void);

private:
    void populateUI(void);

private:
    Ui::ConfigWindow *ui;
    Kub3::Config::hardware_config_t m_hwConfig;
    Kub3::Config::process_config_t m_processConfig;

    // File paths could be passed via args or QFileDialog
    QString m_hwConfigPath;
    QString m_processConfigPath;
};
