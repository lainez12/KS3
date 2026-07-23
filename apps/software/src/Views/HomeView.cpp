#include "ui_HomeView.h"
#include <Views/HomeView.h>

namespace
{
    constexpr const char *ID_BTN_SETTINGS = "S";
}

namespace Kub3::UI::Views
{
    HomeView::HomeView(Unique<ViewModels::HomeViewModel> viewModel, QWidget *parent) :
        ViewBase(std::move(viewModel), parent),
        ui(new Ui::HomeView),
        m_vm(static_cast<ViewModels::HomeViewModel *>(m_viewModel.get())) // Cache typed pointer
    {
        ui->setupUi(this);

        m_showCentralLogo = true;
        createNavButtonsConfigs();
        configTitleBar();
        updateMachineLogo(this->height());

        setupConnections();
    }

    HomeView::~HomeView()
    {
        delete ui;
    }

    void HomeView::setupConnections()
    {
        // ==========================================
        // 1. TIER 1 -> TIER 2 (User Intent Routing)
        // ==========================================
        connect(ui->btnInitialization, &QPushButton::clicked, this, [this]() {
            m_vm->uiRequestInitialization();
        });
        connect(ui->btnWaferInsert, &QPushButton::clicked, this, [this]() {
            m_vm->uiRequestDrawerOperation(DrawerTarget::Wafer, false);
        });
        connect(ui->btnMaskInsert, &QPushButton::clicked, this, [this]() {
            m_vm->uiRequestDrawerOperation(DrawerTarget::Mask, false);
        });
        connect(ui->btnWaferEject, &QPushButton::clicked, this, [this]() {
            m_vm->uiRequestDrawerOperation(DrawerTarget::Wafer, true);
        });
        connect(ui->btnMaskEject, &QPushButton::clicked, this, [this]() {
            m_vm->uiRequestDrawerOperation(DrawerTarget::Mask, true);
        });

        // Local UI Navigation (No business logic required, safe to route directly to MainWindow)
        connect(ui->mainSubmenuExposureBtn, &QPushButton::clicked, this, [this]() {
            emit s_openView(Kub3::UI::ViewId::EXPOSURE_MENU_VIEW);
        });

        // ==========================================
        // 2. TIER 2 -> TIER 1 (Data Binding)
        // ==========================================

        connect(m_vm, &ViewModels::HomeViewModel::s_setDrawerActionsLock, this, &HomeView::setDrawerActionsLocked);
        connect(m_vm, &ViewModels::HomeViewModel::s_setExposureModeLock, this, &HomeView::setExposureModeLocked);
        connect(m_vm, &ViewModels::HomeViewModel::s_setHomingLock, this, &HomeView::setHomingLocked);
        connect(m_vm, &ViewModels::HomeViewModel::s_setInitializationLock, this, &HomeView::setInitializationLocked);
    }

    void HomeView::setDrawerActionsLocked(bool locked)
    {
        ui->btnMaskInsert->setEnabled(!locked);
        ui->btnWaferInsert->setEnabled(!locked);
        ui->btnMaskEject->setEnabled(!locked);
        ui->btnWaferEject->setEnabled(!locked);
    }

    void HomeView::setExposureModeLocked(bool locked)
    {
        ui->mainSubmenuExposureBtn->setEnabled(!locked);
    }

    void HomeView::setHomingLocked(bool locked)
    {
        ui->mainSubmenuStandByBtn->setEnabled(!locked);
    }

    void HomeView::setInitializationLocked(bool locked)
    {
        ui->btnInitialization->setEnabled(!locked);
    }

    void HomeView::createNavButtonsConfigs()
    {
        // Using a lambda here as well keeps local UI navigation encapsulated
        NavButtonConfig settingsBtn(
            "Settings",
            QColor("#0072BA"),
            QColor("#B2D4F4"),
            ":/icons/settings.svg",
            ID_BTN_SETTINGS,
            [this]() { emit s_openView(Kub3::UI::ViewId::SETTINGS_VIEW); });
        addNavButton("left", settingsBtn);
    }

    void HomeView::configTitleBar()
    {
        setTitleBar(TitleBarConfig{});
    }

    void HomeView::resizeEvent(QResizeEvent *ev)
    {
        QWidget::resizeEvent(ev);
        this->updateMachineLogo(this->height());
    }

    void HomeView::updateMachineLogo(int h)
    {
        if (h <= 0)
            return;

        const int baseFontSize          = h * 0.11;
        const int substrateSizeFontSize = baseFontSize * 0.45;

        const QString templateStr =
            "<span style='font-family:\"Art ttnorm\"; font-style:italic;"
            "font-size:%1px; color:#0072ba; letter-spacing: 0px;'>&nbsp;UV-KUB 3</span>"
            "<span style='font-family:\"Art ttnorm\"; font-style:italic;"
            "font-size:%1px; color:#e85420; letter-spacing: 0px;'> - </span>"
            "<span style='font-family:\"Art ttnorm\"; font-style:italic;"
            "font-size:%1px; color:#e85420; letter-spacing: 3px;'>%3</span>"
            "<span style='font-family:\"Art ttnorm\"; font-style:italic;"
            "font-size:%2px; color:#e85420; letter-spacing: 0px;'>-inch</span>";

        ui->mainTitle->setText(templateStr.arg(baseFontSize).arg(substrateSizeFontSize).arg(KUB_MODEL_STR));
    }

} // namespace Kub3::UI::Views
