#include <QMessageBox>

#include "../include/ConfigWindow.h"
#include "ui_ConfigWindow.h"
#include <Config/ConfigLoader.h>

ConfigWindow::ConfigWindow(QString hwConfigPath, QString processConfigPath, QWidget *parent) :
    QMainWindow(parent),
    m_hwConfigPath(std::move(hwConfigPath)),
    m_processConfigPath(std::move(processConfigPath))
{
    ui = new Ui::ConfigWindow;

    ui->setupUi(this);

    connect(ui->saveBtn, &QPushButton::clicked, this, &ConfigWindow::onSaveClicked);
    connect(ui->reloadBtn, &QPushButton::clicked, this, &ConfigWindow::onReloadClicked);

    onReloadClicked(); // Load at startup
}

ConfigWindow::~ConfigWindow()
{
    delete ui;
}

void ConfigWindow::onReloadClicked(void)
{
    try
    {
        m_hwConfig      = Kub3::Config::ConfigLoader::loadHardwareConfig(m_hwConfigPath.toStdString());
        m_processConfig = Kub3::Config::ConfigLoader::loadProcessConfig(m_processConfigPath.toStdString());
        populateUI();
        // ui->statusbar->showMessage("Configurations loaded successfully.", 3000);
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(this, "Load Error", e.what());
    }
}

void ConfigWindow::onSaveClicked(void)
{
    // TODO: Pull values from UI widgets back into the m_hwConfig/m_processConfig structs

    try
    {
        // ui->statusbar->showMessage("Configurations saved.", 3000);
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(this, "Save Error", e.what());
    }
}

void ConfigWindow::populateUI(void)
{
    // Example: map your data to UI fields so the tech can edit them
    // ui->spinBoxWaferEjectThresh->setValue(m_processConfig.getEncoderThreshold("ejection_slowdown_threshold"));
}