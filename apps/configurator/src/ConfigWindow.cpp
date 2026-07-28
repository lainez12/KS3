#include "ui_ConfigWindow.h"

#include <Config/ConfigLoader.h>
#include <Config/ConfigSaver.h>
#include <ConfigWindow.h>
#include <HAL/MachineStatus/actuators_labels.h>
#include <pages/AlignmentPositionsPage.h>
#include <pages/CameraConfigPage.h>
#include <pages/CameraGeneralPage.h>
#include <pages/DrawersPositionsPage.h>
#include <pages/ForceConversionsConfigPage.h>
#include <pages/ForceThresholdsConfigPage.h>
#include <pages/LanguageConfigPage.h>
#include <pages/MCUConfigPage.h>
#include <pages/MotorConfigPage.h>
#include <pages/PadMovementsPage.h>
#include <pages/ZAlgorithmicLimitsPage.h>
#include <utils.h>

#include <QCloseEvent>
#include <QDoubleSpinBox>
#include <QFile>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

#define SYSTEM_CATEGORY        "System"
#define FORCE_CATEGORY         "Force sensors"
#define Z_CATEGORY             "Z-Axis"
#define ALIGNMENT_CATEGORY     "X/Y/Theta (alignment)"
#define CONVEYORS_CATEGORY     "Conveyors"
#define CAMERAS_CATEGORY       "Cameras"
#define PAD_CATEGORY           "Pad"
#define MISCELLANEOUS_CATEGORY "Miscellaneous"

ConfigWindow::ConfigWindow(QString hwConfigPath, QString processConfigPath, QString adminConfigPath, QWidget *parent) :
    QWidget(parent),
    m_hwConfigPath(std::move(hwConfigPath)),
    m_processConfigPath(std::move(processConfigPath)),
    m_adminConfigPath(std::move(adminConfigPath)),
    ui(new Ui::ConfigWindow)
{
    ui->setupUi(this);

    // Wire the buttons
    connect(ui->btnSave, &QPushButton::clicked, this, &ConfigWindow::onSaveClicked);
    connect(ui->btnReload, &QPushButton::clicked, this, &ConfigWindow::onReloadClicked);
    connect(ui->btnQuit, &QPushButton::clicked, this, &ConfigWindow::onQuitClicked);

    // Wire the two-tier navigation
    connect(ui->categoryList, &QListWidget::currentItemChanged, this, &ConfigWindow::onCategorySelectionChanged);
    connect(ui->itemList, &QListWidget::currentItemChanged, this, &ConfigWindow::onItemSelected);

    onReloadClicked(); // Load at startup
}

ConfigWindow::~ConfigWindow()
{
    delete ui;
}

void ConfigWindow::onQuitClicked()
{
    close(); // Triggering close() will call closeEvent()
}

void ConfigWindow::onReloadClicked()
{
    try
    {
        // Load from disk
        m_hwConfig      = Kub3::Config::ConfigLoader::loadHardwareConfig(m_hwConfigPath.toStdString());
        m_processConfig = Kub3::Config::ConfigLoader::loadProcessConfig(m_processConfigPath.toStdString());
        m_adminConfig   = Kub3::Config::ConfigLoader::loadAdminConfig(m_adminConfigPath.toStdString());

        // Rebuild UI
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
        // Execute all registered save hooks to pull data from the StackedWidget pages into RAM
        for (const auto &hook : m_saveHooks)
        {
            if (hook)
                hook();
        }

        // Serialize to disk atomically
        Kub3::Config::ConfigSaver::saveHardwareConfig(m_hwConfig, m_hwConfigPath.toStdString());
        Kub3::Config::ConfigSaver::saveProcessConfig(m_processConfig, m_processConfigPath.toStdString());
        Kub3::Config::ConfigSaver::saveAdminConfig(m_adminConfig, m_adminConfigPath.toStdString());

#ifdef BUILD_RELEASE
        // SECURE THE FILE: Enforce root-only access for Admin Config in release or deploy mode
        QFile adminFile(m_adminConfigPath);
        if (adminFile.exists())
        {
            adminFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        }
#endif

        QMessageBox::information(this, "Success", "Configurations saved successfully.");
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(this, "Save Error", e.what());
    }
}

void ConfigWindow::closeEvent(QCloseEvent *event)
{
    // Ask for confirmation before discarding potential modifications
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm Exit",
        "Are you sure you want to exit? Any unsaved changes will be lost.",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        event->accept(); // Allows the window to close, shutting down the app
    }
    else
    {
        event->ignore(); // Prevents the window from closing
    }
}

void ConfigWindow::clearUI()
{
    m_saveHooks.clear();
    m_categoryMap.clear();

    ui->categoryList->clear();
    ui->itemList->clear();

    while (ui->stackedWidget->count() > 0)
    {
        QWidget *w = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(w);
        w->deleteLater();
    }
}

int ConfigWindow::addConfigPage(QWidget *pageWidget, std::function<void()> saveCallback)
{
    int index = ui->stackedWidget->addWidget(pageWidget);

    m_saveHooks.push_back(std::move(saveCallback));
    return index; // Return where this page lives in the stack
}

// ---------------------------------------------------------
// CATEGORIZATION LOGIC
// ---------------------------------------------------------
QString ConfigWindow::categorizeMotor(const QString &motorId) const
{
    if (motorId == Z_LEFT_MOTOR || motorId == Z_RIGHT_MOTOR || motorId == Z_BACK_MOTOR)
        return Z_CATEGORY;
    if (motorId == X_STAGE_MOTOR || motorId == Y_STAGE_MOTOR || motorId == THETA_STAGE_MOTOR)
        return ALIGNMENT_CATEGORY;
    if (motorId == MASK_DRAWER_MOTOR || motorId == WAFER_DRAWER_MOTOR)
        return CONVEYORS_CATEGORY;
    if (motorId == DECK_MOTOR ||
        motorId == LEFT_CAMERA_X_MOTOR || motorId == LEFT_CAMERA_Y_MOTOR ||
        motorId == RIGHT_CAMERA_X_MOTOR || motorId == RIGHT_CAMERA_Y_MOTOR)
        return CAMERAS_CATEGORY;
    // else
    return MISCELLANEOUS_CATEGORY;
}

// ---------------------------------------------------------
// UI POPULATION
// ---------------------------------------------------------
void ConfigWindow::populateUI()
{
    // Clear out any old widgets
    clearUI();
    ui->categoryList->blockSignals(true); // Prevent UI thrashing while building

    // -------------------------------------------------------------
    // STATIC PAGE: ADMIN SETTINGS
    // -------------------------------------------------------------
    m_adminPage = new Kub3::Components::AdminConfigPage(this);
    // Give the admin page the data (it caches it, but keeps the UI empty because locked at first)
    m_adminPage->loadInitialData(m_adminConfig);
    connect(m_adminPage, &Kub3::Components::AdminConfigPage::adminUnlocked, this, [this]() {
        m_adminPage->loadInitialData(m_adminConfig); // When the user unlocks the page, retrigger load to display values
    });
    int adminIdx = addConfigPage(m_adminPage, [this]() {
        m_adminPage->pullDataToStruct(m_adminConfig);
    });
    // -------------------------------------------------------------
    // STATIC PAGE: KEYBOARD & LANGUAGE SETTINGS
    // -------------------------------------------------------------
    auto *languagePage = new Kub3::Components::LanguageConfigPage(m_hwConfig);
    int languageIdx    = addConfigPage(languagePage, [this, languagePage]() {
        languagePage->pullDataToStruct(m_hwConfig);
    });
    // -------------------------------------------------------------
    // STATIC PAGE: MCU SETTINGS
    // -------------------------------------------------------------
    auto *mcuPage = new Kub3::Components::MCUConfigPage(m_hwConfig);
    int mcuIdx    = addConfigPage(mcuPage, [this, mcuPage]() {
        mcuPage->pullDataToStruct(m_hwConfig);
    });
    // -------------------------------------------------------------
    // STATIC PAGE: FORCE THRESHOLDS SETTINGS
    // -------------------------------------------------------------
    auto *forceThresholdsPage = new Kub3::Components::ForceThresholdsConfigPage(m_processConfig.contact);
    int forceThresholdsIndex  = addConfigPage(forceThresholdsPage, [this, forceThresholdsPage]() {
        forceThresholdsPage->pullDataToStruct(m_processConfig.contact);
    });
    // -------------------------------------------------------------
    // STATIC PAGE: FORCE CONVERSIONS SETTINGS
    // -------------------------------------------------------------
    auto *forceConversionPage = new Kub3::Components::ForceConversionsConfigPage(m_hwConfig);
    int forceConvIndex        = addConfigPage(forceConversionPage, [this, forceConversionPage]() {
        forceConversionPage->pullDataToStruct(m_hwConfig);
    });
    // -------------------------------------------------------------
    // STATIC PAGE: Z-AXIS SETTINGS
    // -------------------------------------------------------------
    auto *zAxisPage = new Kub3::Components::ZAlgorithmicLimitsPage(m_processConfig);
    int zAxisIndex  = addConfigPage(zAxisPage, [this, zAxisPage]() {
        zAxisPage->pullDataToStruct(m_processConfig);
    });
    // -------------------------------------------------------------
    // STATIC PAGE: ALIGNMENT CALIBRATION SETTINGS
    // -------------------------------------------------------------
    auto *alignPage = new Kub3::Components::AlignmentPositionsPage(m_processConfig.alignment);
    int alignIdx    = addConfigPage(alignPage, [this, alignPage]() {
        alignPage->pullDataToStruct(m_processConfig.alignment);
    });
    // -------------------------------------------------------------
    // STATIC PAGE: CONVEYORS CALIBRATION SETTINGS
    // -------------------------------------------------------------
    auto *drawersPage = new Kub3::Components::DrawersPositionsPage(m_processConfig.drawers);
    int drawersIdx    = addConfigPage(drawersPage, [this, drawersPage]() {
        drawersPage->pullDataToStruct(m_processConfig.drawers);
    });
    QStringList allMotorIds;
    for (const auto &[motorId, _] : m_hwConfig.motors)
        allMotorIds << motorId;
    // -------------------------------------------------------------
    // STATIC PAGE: CAMERAS GENERAL SETTINGS
    // -------------------------------------------------------------
    auto *camGenPage = new Kub3::Components::CameraGeneralPage(m_processConfig.vision);
    int camGenIndex  = addConfigPage(camGenPage, [this, camGenPage]() {
        camGenPage->pullDataToStruct(m_processConfig.vision);
    });

    auto *padMovementsPage = new Kub3::Components::PadMovementsPage(m_processConfig.pad);
    int padMovementIndex   = addConfigPage(padMovementsPage, [this, padMovementsPage]() {
        padMovementsPage->pullDataToStruct(m_processConfig.pad);
    });

    m_categoryMap[SYSTEM_CATEGORY].push_back({"Administrator", adminIdx});
    m_categoryMap[SYSTEM_CATEGORY].push_back({"Locale (Keyboard & Language)", languageIdx});
    m_categoryMap[SYSTEM_CATEGORY].push_back({"Micro-controllers", mcuIdx});
    m_categoryMap[FORCE_CATEGORY].push_back({"Limits & Thresholds", forceThresholdsIndex});
    m_categoryMap[FORCE_CATEGORY].push_back({"Conversions", forceConvIndex});
    m_categoryMap[Z_CATEGORY].push_back({"Algorithmic Limits", zAxisIndex});
    m_categoryMap[ALIGNMENT_CATEGORY].push_back({"Calibration Positions", alignIdx});
    m_categoryMap[CONVEYORS_CATEGORY].push_back({"Process Positions", drawersIdx});
    m_categoryMap[CAMERAS_CATEGORY].push_back({"General Settings", camGenIndex});
    m_categoryMap[PAD_CATEGORY].push_back({"Short presses", padMovementIndex});

    // -------------------------------------------------------------
    // DYNAMIC PAGES: CAMERAS SETTINGS
    // -------------------------------------------------------------
    for (auto &[camId, camConfig] : m_hwConfig.cameras)
    {
        auto *camPage  = new Kub3::Components::CameraConfigPage(camConfig);
        int stackIndex = addConfigPage(camPage, [&camConfig, camPage]() {
            camPage->pullDataToStruct(camConfig);
        });

        m_categoryMap[CAMERAS_CATEGORY].push_back({Kub3::camelToNormal(camId), stackIndex});
    }

    // -------------------------------------------------------------
    // DYNAMIC PAGES: MOTORS
    // -------------------------------------------------------------
    for (auto &[motorId, motorConfig] : m_hwConfig.motors)
    {
        auto kinIt                                   = m_processConfig.kinematic_profiles.find(motorId.toStdString());
        Kub3::Config::KinematicProfiles myKinematics = (kinIt != m_processConfig.kinematic_profiles.end()) ? kinIt->second : Kub3::Config::KinematicProfiles{};
        auto *motorPage                              = new Kub3::Components::MotorConfigPage(motorConfig, myKinematics, allMotorIds);

        connect(motorPage, &Kub3::Components::MotorConfigPage::profileExportRequested, this,
                [this](const QString &targetMotor, const Kub3::Config::kinematic_profile_t &profile) {
                    for (const auto &hook : m_saveHooks)
                    {
                        if (hook)
                            hook();
                    }
                    m_processConfig.kinematic_profiles[targetMotor.toStdString()][profile.id] = profile;
                    populateUI();
                    QMessageBox::information(this, "Export", "Profile exported successfully.");
                });

        int stackIndex = addConfigPage(motorPage, [this, &motorConfig, motorId = motorId.toStdString(), motorPage]() {
            Kub3::Config::KinematicProfiles finalKinematics;
            motorPage->pullDataToStruct(motorConfig, finalKinematics);
            m_processConfig.kinematic_profiles[motorId] = finalKinematics;
        });

        // Add to the appropriate category!
        QString category = categorizeMotor(motorId);
        m_categoryMap[category].push_back({Kub3::camelToNormal(motorId), stackIndex});
    }

    // We add them in a specific order if we want, or just iterate the map.
    QStringList categoryOrder = {
        SYSTEM_CATEGORY,
        FORCE_CATEGORY,
        CONVEYORS_CATEGORY,
        Z_CATEGORY,
        ALIGNMENT_CATEGORY,
        CAMERAS_CATEGORY,
        PAD_CATEGORY,
        MISCELLANEOUS_CATEGORY,
    };

    for (const QString &cat : categoryOrder)
    {
        if (m_categoryMap.contains(cat))
            ui->categoryList->addItem(cat);
    }

    ui->categoryList->blockSignals(false);

    if (ui->categoryList->count() > 0)
    {
        ui->categoryList->setCurrentRow(0); // This will trigger the cascade
    }
}

void ConfigWindow::onCategorySelectionChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        return;

    ui->itemList->blockSignals(true);
    ui->itemList->clear();

    QString category  = current->text();
    const auto &pages = m_categoryMap[category];

    for (const auto &page : pages)
    {
        auto *item = new QListWidgetItem(page.displayName);

        // Store the Stack Index directly inside the List Item
        item->setData(Qt::UserRole, page.stackIndex);
        ui->itemList->addItem(item);
    }

    ui->itemList->blockSignals(false);

    if (ui->itemList->count() > 0)
    {
        ui->itemList->setCurrentRow(0); // Select first item, triggering stacked widget switch
    }
}

void ConfigWindow::onItemSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        return;

    // Retrieve the stored stack index and switch the view
    int stackIndex = current->data(Qt::UserRole).toInt();
    ui->stackedWidget->setCurrentIndex(stackIndex);
}
