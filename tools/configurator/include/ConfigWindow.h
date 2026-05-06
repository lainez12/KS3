#pragma once

#include <QListWidgetItem>
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

    void onCategorySelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onItemSelected(QListWidgetItem *current, QListWidgetItem *previous);

private:
    void populateUI();
    void clearUI();

    // Helper to add a view to the list and stack cleanly
    int addConfigPage(QWidget *pageWidget, std::function<void()> saveCallback);

    // Hardcoded Ergonomic Categorization
    QString categorizeMotor(const QString &motorId) const;

private:
    Ui::ConfigWindow *ui;

    Kub3::Config::hardware_config_t m_hwConfig;
    Kub3::Config::process_config_t m_processConfig;

    QString m_hwConfigPath;
    QString m_processConfigPath;

    // A list of closures that pull data from the UI back into the structs
    std::vector<std::function<void()>> m_saveHooks;

    // --- Category Routing Data ---
    struct PageReference {
        QString displayName;
        int stackIndex;
    };

    // Maps a Category Name (e.g. "Z Motors") to a list of its UI Pages
    std::map<QString, std::vector<PageReference>> m_categoryMap;
};
