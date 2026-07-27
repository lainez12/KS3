#include <Views/Components/Colors.h>
#include <Views/ExposureMenuView.h>

#include "ui_ExposureMenuView.h"

ExposureMenuView::ExposureMenuView(Unique<ExposureMenuViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::ExposureMenuView),
    m_vm(static_cast<ViewModels::ExposureMenuViewModel *>(m_viewModel.get())),
    m_successPixmap(":/icons/check-green.svg"),
    m_failurePixmap(":/icons/abandon-red.svg"),
    m_runningPixmap(":/icons/loading-orange.svg"),
    m_idlePixmap(":/icons/line-squared-blue.svg")
{
    ui->setupUi(this);
    updateStatusPixmap(ui->lblMaskStatus, StepStatus::NotStarted);
    updateStatusPixmap(ui->lblWaferStatus, StepStatus::NotStarted);
    updateStatusPixmap(ui->lblAutolevelStatus, StepStatus::NotStarted);

    this->createNavButtonsConfigs();
    this->configTitleBar();
    this->setupConnections();
    this->checkPixmaps();
}

ExposureMenuView::~ExposureMenuView()
{
    delete ui;
}

void ExposureMenuView::createNavButtonsConfigs()
{
    NavButtonConfig backBtn(
        "Back",
        ":/icons/back.svg",
        "B",
        std::bind(&ExposureMenuView::onBackButtonClicked, this));

    addNavButton("left", backBtn);
}

void ExposureMenuView::configTitleBar()
{
    setTitleBar(TitleBarConfig{
        .viewTitle      = "Exposure Menu",
        .textColor      = QColor("#FFF"),
        .bgColor        = QColor(BLUE_COLOR),
        .iconPath       = "",
        .sectionTitle   = "",
        .showTitleBar   = true,
        .m_showLeftLogo = true,
    });
}

void ExposureMenuView::setupConnections()
{
    if (m_vm)
    {
        // Buttons' locks
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_setMaskLoadLock, this, &ExposureMenuView::setMaskLoadLock);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_setMaskCancelLock, this, &ExposureMenuView::setMaskCancelLock);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_setWaferLoadLock, this, &ExposureMenuView::setWaferLoadLock);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_setWaferCancelLock, this, &ExposureMenuView::setWaferCancelLock);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_setAutolevelLock, this, &ExposureMenuView::setAutolevelLock);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_setAutolevelCancelLock, this, &ExposureMenuView::setAutolevelCancelLock);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_setSaveHorizontalityLock, this, &ExposureMenuView::setSaveHorizontalityLocked);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_setLoadHorizontalityLock, this, &ExposureMenuView::setLoadHorizontalityLocked);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_setContinueToExposureLock, this, &ExposureMenuView::setContinueToExposureLocked);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_setAllLocks, this, &ExposureMenuView::setAllLocks);
        // Progress indicators
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_setMaskStatus, this, &ExposureMenuView::setMaskStatus);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_setWaferStatus, this, &ExposureMenuView::setWaferStatus);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_setAutolevelStatus, this, &ExposureMenuView::setAutolevelStatus);
        // Update logs
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_appendMaskSequenceLog, this, &ExposureMenuView::appendMaskSequenceLog);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_appendWaferSequenceLog, this, &ExposureMenuView::appendWaferSequenceLog);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_appendAutolevelSequenceLog, this, &ExposureMenuView::appendAutolevelSequenceLog);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_clearMaskSequenceLogs, this, &ExposureMenuView::clearMaskSequenceLogs);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_clearWaferSequenceLogs, this, &ExposureMenuView::clearWaferSequenceLogs);
        connect(m_vm, &ViewModels::ExposureMenuViewModel::s_clearAutolevelSequenceLogs, this, &ExposureMenuView::clearAutolevelSequenceLogs);
    }

    // Connect UI buttons
    connect(ui->btnLoadMaskToExposure, &QPushButton::clicked, this, &ExposureMenuView::onBtnLoadMaskToExposureClicked);
    connect(ui->btnLoadWaferToExposure, &QPushButton::clicked, this, &ExposureMenuView::onBtnLoadWaferToExposureClicked);
    connect(ui->btnStartAutoleveling, &QPushButton::clicked, this, &ExposureMenuView::onBtnStartAutoleveling);
    connect(ui->btnCancelMaskLoadToExposure, &QPushButton::clicked, [this]() { if (m_vm) m_vm->ui_requestCancel(); });
    connect(ui->btnCancelWaferLoadToExposure, &QPushButton::clicked, [this]() { if (m_vm) m_vm->ui_requestCancel(); });
    connect(ui->btnCancelAutolevel, &QPushButton::clicked, [this]() { if (m_vm) m_vm->ui_requestCancel(); });
    connect(ui->btnExposureMode, &QPushButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::EXPOSURE_MODE_VIEW); });
}

void ExposureMenuView::checkPixmaps()
{
    if (m_successPixmap.isNull())
        throw std::runtime_error("Failed to load pixmap: ExposureMenuView::m_successPixmap");
    if (m_failurePixmap.isNull())
        throw std::runtime_error("Failed to load pixmap: ExposureMenuView::m_failurePixmap");
    if (m_runningPixmap.isNull())
        throw std::runtime_error("Failed to load pixmap: ExposureMenuView::m_runningPixmap");
    if (m_idlePixmap.isNull())
        throw std::runtime_error("Failed to load pixmap: ExposureMenuView::m_idlePixmap");
}

void ExposureMenuView::onBtnLoadMaskToExposureClicked()
{
    auto vm = getViewModel<ExposureMenuViewModel>();

    if (vm)
    {
        vm->ui_requestLoadMaskToExposure();
    }
}

void ExposureMenuView::onBtnLoadWaferToExposureClicked()
{
    auto vm = getViewModel<ExposureMenuViewModel>();

    if (vm)
    {
        vm->ui_requestLoadWaferToExposure();
    }
}

void ExposureMenuView::onBtnStartAutoleveling()
{
    auto vm = getViewModel<ExposureMenuViewModel>();

    if (vm)
    {
        vm->ui_requestAutoleveling();
    }
}

void ExposureMenuView::onBackButtonClicked()
{
    emit s_openView(Kub3::UI::ViewId::HOME_VIEW);
}

void ExposureMenuView::setMaskLoadLock(bool locked)
{
    ui->btnLoadMaskToExposure->setEnabled(!locked);
}

void ExposureMenuView::setMaskCancelLock(bool locked)
{
    ui->btnCancelMaskLoadToExposure->setEnabled(!locked);
}

void ExposureMenuView::setWaferLoadLock(bool locked)
{
    ui->btnLoadWaferToExposure->setEnabled(!locked);
}

void ExposureMenuView::setWaferCancelLock(bool locked)
{
    ui->btnCancelWaferLoadToExposure->setEnabled(!locked);
}

void ExposureMenuView::setAutolevelLock(bool locked)
{
    ui->btnStartAutoleveling->setEnabled(!locked);
}

void ExposureMenuView::setAutolevelCancelLock(bool locked)
{
    ui->btnCancelAutolevel->setEnabled(!locked);
}

void ExposureMenuView::setSaveHorizontalityLocked(bool locked)
{
    ui->btnSaveHorizontality->setEnabled(!locked);
}

void ExposureMenuView::setLoadHorizontalityLocked(bool locked)
{
    ui->btnLoadHorizontality->setEnabled(!locked);
}

void ExposureMenuView::setContinueToExposureLocked(bool locked)
{
    ui->btnExposureMode->setEnabled(!locked);
}

void ExposureMenuView::updateStatusPixmap(SpinningLabel *lblStatus, StepStatus status)
{
    // Spinning
    if (status == StepStatus::Running)
    {
        lblStatus->setRawPixmap(m_runningPixmap);
        lblStatus->startSpinning();
        return;
    }
    // Non-spinning states
    if (status == StepStatus::Success)
        lblStatus->setRawPixmap(m_successPixmap);
    else if (status == StepStatus::NotStarted)
        lblStatus->setRawPixmap(m_idlePixmap);
    else if (status == StepStatus::Error)
        lblStatus->setRawPixmap(m_failurePixmap);
    lblStatus->stopSpinning();
}

void ExposureMenuView::setMaskStatus(StepStatus status)
{
    this->updateStatusPixmap(ui->lblMaskStatus, status);
}

void ExposureMenuView::setWaferStatus(StepStatus status)
{
    this->updateStatusPixmap(ui->lblWaferStatus, status);
}

void ExposureMenuView::setAutolevelStatus(StepStatus status)
{
    this->updateStatusPixmap(ui->lblAutolevelStatus, status);
}

void ExposureMenuView::appendMaskSequenceLog(const QString &htmlLogLine)
{
    ui->maskModuleLogs->append(htmlLogLine);
}

void ExposureMenuView::appendWaferSequenceLog(const QString &htmlLogLine)
{
    ui->waferModuleLogs->append(htmlLogLine);
}

void ExposureMenuView::appendAutolevelSequenceLog(const QString &htmlLogLine)
{
    ui->autoLevelingModuleLogs->append(htmlLogLine);
}

void ExposureMenuView::clearMaskSequenceLogs()
{
    ui->maskModuleLogs->clear();
}

void ExposureMenuView::clearWaferSequenceLogs()
{
    ui->waferModuleLogs->clear();
}

void ExposureMenuView::clearAutolevelSequenceLogs()
{
    ui->autoLevelingModuleLogs->clear();
}

void ExposureMenuView::setAllLocks(bool locked)
{
    this->setMaskLoadLock(locked);
    this->setMaskCancelLock(locked);
    this->setWaferLoadLock(locked);
    this->setWaferCancelLock(locked);
    this->setAutolevelLock(locked);
    this->setAutolevelCancelLock(locked);
    this->setSaveHorizontalityLocked(locked);
    this->setLoadHorizontalityLocked(locked);
    this->setContinueToExposureLocked(locked);
}
