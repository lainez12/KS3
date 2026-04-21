#include "Views/HomeView.h"
#include "ui_HomeView.h"

HomeView::HomeView(Unique<HomeViewModel> viewModel, QWidget *parent) :
    ViewBase(std::move(viewModel), parent),
    ui(new Ui::HomeView)
{
    ui->setupUi(this);

    ui->floodBtn->setup("Flood Exposure", "#8a2be2", ":/icons/settings.svg");     // Purple
    ui->maskExpBtn->setup("Mask Exposure", "#0000cd", ":/icons/settings.svg");    // Blue
    ui->maskAlignBtn->setup("Mask Alignment", "#00ced1", ":/icons/settings.svg"); // Cyan

    this->updateMachineLogo(this->height());
}

HomeView::~HomeView()
{
    delete ui;
}

void HomeView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);

    this->updateMachineLogo(this->height());
    ui->buttonsLayout->setSpacing(this->width() * 0.07); // 7% of window width
}

void HomeView::updateMachineLogo(int h)
{
    if (h <= 0)
        return;

    const int baseFontSize          = h * 0.1;             // 6% of window height
    const int substrateSizeFontSize = baseFontSize * 0.64; // 64% of base
    // We use arg %1 for the logo size, arg %2 for the substrate size, arg %3 for the text
    const QString templateStr =
        "<span style='font-family:\"Art ttnorm\"; font-style:italic;"
        "font-size:%1px; color:#0072ba;'>UV-KUB 3</span>"
        "<span style='font-family:\"Art ttnorm\"; font-style:italic;"
        "font-size:%2px; color:#e85420;'>&nbsp;%3-inch</span>";

    ui->machineName->setText(templateStr.arg(baseFontSize).arg(substrateSizeFontSize).arg(KUB_MODEL_STR));
}
