#include <Views/ExposureModeView.h>

#include "ui_ExposureModeView.h"

#define ID_BTN_SETTINGS "S"
#define ID_BTN_OPEN     "O"
#define ID_BTN_CLOSE    "C"
#define ID_BTN_BACK     "B"

ExposureModeView::ExposureModeView(Unique<HomeViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::ExposureModeView)
{
    ui->setupUi(this);

    createNavButtonsConfigs();
    configTitleBar();

    ui->floodBtn->setup("Flood Exposure", "#8a2be2", ":/icons/flood_icon.svg"); // Purple
    // ui->maskExpBtn->setup("Mask Exposure", "#0000cd", ":/icons/mask-exposure_icon.svg");     // Blue
    ui->maskAlignBtn->setup("Mask Alignment\nMask Exposure", "#00ced1", ":/icons/mask-alignment_icon.svg"); // Cyan

    connect(ui->floodBtn, &ActionBox::clicked, this, &ExposureModeView::onFloodExposureClicked);
    // connect(ui->maskExpBtn, &ActionBox::clicked, this, &ExposureModeView::onMaskExposureClicked);
    connect(ui->maskAlignBtn, &ActionBox::clicked, this, &ExposureModeView::onMaskAlignmentClicked);

    this->updateMachineLogo(this->height());
}

ExposureModeView::~ExposureModeView()
{
    delete ui;
}

void ExposureModeView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);

    this->updateMachineLogo(this->height());
    ui->buttonsLayout->setSpacing(this->width() * 0.25); // 7% of window width
}

void ExposureModeView::updateMachineLogo(int h)
{
}

void ExposureModeView::createNavButtonsConfigs()
{
    NavButtonConfig backMainBtn(
        "Back to Main",
        QColor("#FFF"),
        QColor("#B2D4F4"),
        ":/icons/picto.png",
        ID_BTN_BACK,
        std::bind(&ExposureModeView::onBackButtonClicked, this, std::placeholders::_1));
    addNavButton("left", backMainBtn);
}
void ExposureModeView::configTitleBar()
{
    setTitleBar(TitleBarConfig{
        .viewTitle      = "Exposure Mode",
        .textColor      = QColor("#FFF"),
        .bgColor        = QColor(BLUE_COLOR),
        .iconPath       = "",
        .sectionTitle   = "",
        .showTitleBar   = true,
        .m_showLeftLogo = true,
    });
}

void ExposureModeView::onBackButtonClicked(const QString &buttonId)
{
    emit s_openView(Kub3::UI::ViewId::EXPOSURE_MENU_VIEW);
}

void ExposureModeView::onFloodExposureClicked()
{
    emit s_openView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW);
}

void ExposureModeView::onMaskAlignmentClicked()
{
    emit s_openView(Kub3::UI::ViewId::ALIGNMENT_CONTACT_SELECTION_VIEW);
}
