#include "ui_ConfigWindow.h"
#include <ConfigWindow.h>

#include <Config/ConfigLoader.h>
#include <Config/ConfigSaver.h>

#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

ConfigWindow::ConfigWindow(QString hwConfigPath, QString processConfigPath, QWidget *parent) :
    QWidget(parent),
    m_hwConfigPath(std::move(hwConfigPath)),
    m_processConfigPath(std::move(processConfigPath)),
    ui(new Ui::ConfigWindow)
{
    ui->setupUi(this);

    // Wire the buttons
    connect(ui->saveBtn, &QPushButton::clicked, this, &ConfigWindow::onSaveClicked);
    connect(ui->reloadBtn, &QPushButton::clicked, this, &ConfigWindow::onReloadClicked);

    // MAGIC TRICK: This single line connects your list to your stacked widget automatically.
    // When the user clicks a different menu item, the stacked widget switches to the corresponding page.
    connect(ui->menuList, &QListWidget::currentRowChanged, ui->stackedWidget, &QStackedWidget::setCurrentIndex);

    onReloadClicked(); // Load at startup
}

ConfigWindow::~ConfigWindow()
{
    delete ui;
}

void ConfigWindow::onReloadClicked()
{
    try
    {
        // 1. Load from disk
        m_hwConfig      = Kub3::Config::ConfigLoader::loadHardwareConfig(m_hwConfigPath.toStdString());
        m_processConfig = Kub3::Config::ConfigLoader::loadProcessConfig(m_processConfigPath.toStdString());

        // 2. Rebuild UI
        populateUI();
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(this, "Load Error", e.what());
    }
}

void ConfigWindow::onSaveClicked()
{
    try
    {
        // 1. Execute all registered save hooks to pull data from the StackedWidget pages into RAM
        for (const auto &hook : m_saveHooks)
        {
            if (hook)
                hook();
        }

        // 2. Serialize to disk atomically
        Kub3::Config::ConfigSaver::saveHardwareConfig(m_hwConfig, m_hwConfigPath.toStdString());
        Kub3::Config::ConfigSaver::saveProcessConfig(m_processConfig, m_processConfigPath.toStdString());

        QMessageBox::information(this, "Success", "Configurations saved successfully.");
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(this, "Save Error", e.what());
    }
}

void ConfigWindow::clearUI()
{
    m_saveHooks.clear();
    ui->menuList->clear();

    // Safely delete all widgets currently in the stacked widget
    while (ui->stackedWidget->count() > 0)
    {
        QWidget *w = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(w);
        w->deleteLater();
    }
}

void ConfigWindow::addConfigPage(const QString &menuLabel, QWidget *pageWidget, std::function<void()> saveCallback)
{
    ui->menuList->addItem(menuLabel);
    ui->stackedWidget->addWidget(pageWidget);
    m_saveHooks.push_back(std::move(saveCallback));
}

void ConfigWindow::populateUI()
{
    // Clear out any old widgets
    clearUI();

    // -------------------------------------------------------------
    // PAGE 1: GLOBAL PROCESS SETTINGS
    // -------------------------------------------------------------
    QWidget *globalPage = new QWidget();
    auto *globalLayout  = new QVBoxLayout(globalPage);

    auto *spinCrashForce = new QDoubleSpinBox();
    spinCrashForce->setMaximum(10000.0);
    spinCrashForce->setValue(m_processConfig.hw_crash_force_limit_gf);

    globalLayout->addWidget(new QLabel("Hardware Crash Force Limit (gf):"));
    globalLayout->addWidget(spinCrashForce);
    globalLayout->addStretch();

    // Add it to the UI and register how it saves itself
    addConfigPage("Global Settings", globalPage, [this, spinCrashForce]() {
        m_processConfig.hw_crash_force_limit_gf = spinCrashForce->value();
    });

    // -------------------------------------------------------------
    // DYNAMIC PAGES: MOTORS
    // -------------------------------------------------------------
    for (auto &[motorId, motorConfig] : m_hwConfig.motors)
    {
        // Eventually, replace this raw QWidget with a custom 'MotorConfigWidget' class
        // designed in Qt Designer to keep this file clean!
        QWidget *motorPage = new QWidget();
        auto *motorLayout  = new QVBoxLayout(motorPage);

        motorLayout->addWidget(new QLabel(QString("Configuration for Motor: %1").arg(motorId)));

        QDoubleSpinBox *spinPitch = nullptr;

        if (std::holds_alternative<Kub3::Config::stepper_hw_properties_t>(motorConfig.hwProperties))
        {
            auto &stepper = std::get<Kub3::Config::stepper_hw_properties_t>(motorConfig.hwProperties);

            spinPitch = new QDoubleSpinBox();
            spinPitch->setDecimals(4);
            spinPitch->setValue(stepper.screwPitchMm);
            motorLayout->addWidget(new QLabel("Screw Pitch (mm):"));
            motorLayout->addWidget(spinPitch);
        }

        motorLayout->addStretch();

        // Add to stack, and define how this motor updates its specific struct
        QString menuName = QString("Motor: %1").arg(motorId);

        addConfigPage(menuName, motorPage, [&motorConfig, spinPitch]() {
            if (spinPitch && std::holds_alternative<Kub3::Config::stepper_hw_properties_t>(motorConfig.hwProperties))
            {
                auto &stepper        = std::get<Kub3::Config::stepper_hw_properties_t>(motorConfig.hwProperties);
                stepper.screwPitchMm = spinPitch->value();
            }
        });
    }

    // -------------------------------------------------------------
    // DYNAMIC PAGES: CAMERAS
    // -------------------------------------------------------------
    for (auto &[camId, camConfig] : m_hwConfig.cameras)
    {
        QWidget *camPage = new QWidget();
        auto *camLayout  = new QVBoxLayout(camPage);

        camLayout->addWidget(new QLabel(QString("Camera: %1").arg(camId)));

        auto *spinMaxExposure = new QDoubleSpinBox();
        spinMaxExposure->setMaximum(1000000.0);
        spinMaxExposure->setValue(camConfig.maxExposureUs);

        camLayout->addWidget(new QLabel("Max Exposure (us):"));
        camLayout->addWidget(spinMaxExposure);
        camLayout->addStretch();

        addConfigPage(QString("Camera: %1").arg(camId), camPage, [&camConfig, spinMaxExposure]() {
            camConfig.maxExposureUs = spinMaxExposure->value();
        });
    }

    // -------------------------------------------------------------
    // SELECT FIRST ITEM BY DEFAULT
    // -------------------------------------------------------------
    if (ui->menuList->count() > 0)
    {
        ui->menuList->setCurrentRow(0);
    }
}
