#include "Views/ViewsExposure/FavoriteExposureSettingsView.h"
#include "Views/Components/Colors.h"
#include "Views/Components/FavoriteExposureSettingButton.h"
#include "ui_FavoriteExposureSettingsView.h"
#include <QRegularExpression>
#include <QString>

#define ID_BTN_BACK "B"
#define ID_BTN_SAVE "S"

FavoriteExposureSettingsView::FavoriteExposureSettingsView(Unique<FavoriteExposureSettingsViewModel> viewModel, QWidget *parent) :
    ExposureViewBase(std::move(viewModel), parent),
    ui(new Ui::FavoriteExposureSettingsView) {
    ui->setupUi(this);

    createNavButtonsConfigs();
    setNewNavButtonsConfigs();
    setDefaultTitleBar("Exposure Settings");

    connect(ui->newSettingsLabel, &QPushButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW); });
    FavoriteExposureSettingButton *btnFirst = new FavoriteExposureSettingButton(
        "First",
        "Exposure duration: 10s\nExposure power: 20%");

    ui->horizontalLayout->addWidget(btnFirst);
    setUpShawedBoxStyle(btnFirst);
}
FavoriteExposureSettingsView::~FavoriteExposureSettingsView() {
}

void FavoriteExposureSettingsView::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}

void FavoriteExposureSettingsView::setNewNavButtonsConfigs() {

    NavButtonConfig backBtn(
        "Back",
        QColor(BLUE_COLOR),
        QColor(BLUE_COLOR_SHADOW),
        ":/icons/back.svg",
        ID_BTN_BACK,
        std::bind(&FavoriteExposureSettingsView::onBackButtonClicked, this, std::placeholders::_1));
    addNavButton("left", backBtn);
}

void FavoriteExposureSettingsView::onBackButtonClicked(const QString &buttonId) {
    emit s_openView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW);
}

void FavoriteExposureSettingsView::onValidateButtonClicked(const QString &buttonId) {
    // emit s_openView(Kub3::UI::ViewId::VALIDATE_VIEW);
}