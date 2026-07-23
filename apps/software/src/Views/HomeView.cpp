#include <Views/HomeView.h>

#include "ui_HomeView.h"

#define ID_BTN_SETTINGS "S"
#define ID_BTN_OPEN     "O"
#define ID_BTN_CLOSE    "C"
#define BUTTONS_SIZE    280

HomeView::HomeView(Unique<HomeViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::HomeView)
{
    ui->setupUi(this);

    createNavButtonsConfigs();
    m_showCentralLogo = true;
    configTitleBar();

    this->updateMachineLogo(this->height());

    ps_setDrawerActionsLock(true);
    ps_setExposureModeLock(true);
    ps_setHomingLock(true);
    ps_setInitializationLock(false); // Only allow init at boot

    connect(ui->btnInitialization, &QPushButton::clicked, this, &HomeView::onInitializationClicked);
    connect(ui->btnMaskInsert, &QPushButton::clicked, this, &HomeView::onMaskInsertBtnClicked);
    connect(ui->btnWaferInsert, &QPushButton::clicked, this, &HomeView::onWaferInsertBtnClicked);
    connect(ui->btnMaskEject, &QPushButton::clicked, this, &HomeView::onMaskEjectBtnClicked);
    connect(ui->btnWaferEject, &QPushButton::clicked, this, &HomeView::onWaferEjectBtnClicked);
    connect(ui->mainSubmenuExposureBtn, &QPushButton::clicked, this, &HomeView::onExposureMenuClicked);

    HomeViewModel *vm = static_cast<HomeViewModel *>(m_viewModel.get());

    if (vm)
    {
        connect(vm, &HomeViewModel::s_setDrawerActionsLock, this, &HomeView::ps_setDrawerActionsLock);
        connect(vm, &HomeViewModel::s_setExposureModeLock, this, &HomeView::ps_setExposureModeLock);
        connect(vm, &HomeViewModel::s_setHomingLock, this, &HomeView::ps_setHomingLock);
        connect(vm, &HomeViewModel::s_setInitializationLock, this, &HomeView::ps_setInitializationLock);
    }
}

HomeView::~HomeView()
{
    delete ui;
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

    const int baseFontSize          = h * 0.11;            // 6% of window height
    const int substrateSizeFontSize = baseFontSize * 0.45; // 64% of base
    // We use arg %1 for the logo size, arg %2 for the substrate size, arg %3 for the text
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

void HomeView::createNavButtonsConfigs()
{
    NavButtonConfig settingsBtn(
        "Settings",
        QColor("#0072BA"),
        QColor("#B2D4F4"),
        ":/icons/settings.svg",
        ID_BTN_SETTINGS,
        std::bind(&HomeView::onSettingsButtonClicked, this));
    addNavButton("left", settingsBtn);
}
void HomeView::configTitleBar()
{
    setTitleBar(TitleBarConfig{});
}

void HomeView::ps_setDrawerActionsLock(bool lock)
{
    ui->btnMaskInsert->setEnabled(!lock);
    ui->btnWaferInsert->setEnabled(!lock);
    ui->btnMaskEject->setEnabled(!lock);
    ui->btnWaferEject->setEnabled(!lock);
}

void HomeView::ps_setExposureModeLock(bool lock)
{
    ui->mainSubmenuExposureBtn->setEnabled(!lock);
}

void HomeView::ps_setHomingLock(bool lock)
{
    ui->mainSubmenuStandByBtn->setEnabled(!lock);
}

void HomeView::ps_setInitializationLock(bool lock)
{
    ui->btnInitialization->setEnabled(!lock);
}

void HomeView::onSettingsButtonClicked()
{
    emit s_openView(Kub3::UI::ViewId::SETTINGS_VIEW);
}

void HomeView::onExposureMenuClicked()
{
    emit s_openView(Kub3::UI::ViewId::EXPOSURE_MENU_VIEW);
}

void HomeView::onMaskAlignmentClicked()
{
    emit s_openView(Kub3::UI::ViewId::ALIGNMENT_DISTANCE_VIEW);
}

void HomeView::onInitializationClicked()
{
    HomeViewModel *vm = static_cast<HomeViewModel *>(m_viewModel.get());

    if (vm)
    {
        vm->uiRequestInitialization();
    }
    else
    {
        qCritical() << "[HomeView::onInitializationClicked] Failed to retrieve view model.";
    }
}

void HomeView::onWaferInsertBtnClicked()
{
    HomeViewModel *vm = static_cast<HomeViewModel *>(m_viewModel.get());

    if (!vm)
        return;
    vm->uiRequestDrawerOperation(DrawerTarget::Wafer, false);
}

void HomeView::onMaskInsertBtnClicked()
{
    HomeViewModel *vm = static_cast<HomeViewModel *>(m_viewModel.get());

    if (!vm)
        return;
    vm->uiRequestDrawerOperation(DrawerTarget::Mask, false);
}

void HomeView::onWaferEjectBtnClicked()
{
    HomeViewModel *vm = static_cast<HomeViewModel *>(m_viewModel.get());

    if (!vm)
        return;
    vm->uiRequestDrawerOperation(DrawerTarget::Wafer, true);
}

void HomeView::onMaskEjectBtnClicked()
{
    HomeViewModel *vm = static_cast<HomeViewModel *>(m_viewModel.get());

    if (!vm)
        return;
    vm->uiRequestDrawerOperation(DrawerTarget::Mask, true);
}
