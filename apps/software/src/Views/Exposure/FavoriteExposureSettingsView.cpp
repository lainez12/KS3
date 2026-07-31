#include "ui_FavoriteExposureSettingsView.h"
#include <QHBoxLayout>
#include <QLayoutItem>
#include <QRegularExpression>
#include <QStackedWidget>
#include <QString>
#include <QVBoxLayout>
#include <QVector>
#include <Views/Components/Colors.h>
#include <Views/Components/FavoriteExposureSettingButton.h>
#include <Views/Exposure/FavoriteExposureSettingsView.h>
#include <algorithm>

#define ID_BTN_BACK "B"
#define ID_BTN_SAVE "S"

namespace
{
    constexpr int FAVORITES_PER_PAGE   = 8;
    constexpr int FAVORITES_PER_COLUMN = 4;

    void clearLayout(QLayout *layout)
    {
        if (!layout)
            return;

        while (QLayoutItem *item = layout->takeAt(0))
        {
            if (QWidget *widget = item->widget())
            {
                widget->setParent(nullptr);
                widget->deleteLater();
            }
            else if (QLayout *childLayout = item->layout())
            {
                clearLayout(childLayout);
                delete childLayout;
            }

            delete item;
        }
    }

    struct FavoritePageWidgets {
        QWidget *page            = nullptr;
        QVBoxLayout *leftLayout  = nullptr;
        QVBoxLayout *rightLayout = nullptr;
    };

    FavoritePageWidgets createFavoritePage(QStackedWidget *stackedWidget)
    {
        auto *page       = new QWidget(stackedWidget);
        auto *pageLayout = new QHBoxLayout(page);
        pageLayout->setContentsMargins(0, 0, 0, 0);
        pageLayout->setSpacing(0);

        auto *leftContainer  = new QWidget(page);
        auto *rightContainer = new QWidget(page);
        auto *leftLayout     = new QVBoxLayout(leftContainer);
        auto *rightLayout    = new QVBoxLayout(rightContainer);

        leftLayout->setSpacing(0);
        leftLayout->setContentsMargins(50, 0, 50, 0);
        rightLayout->setSpacing(0);
        rightLayout->setContentsMargins(50, 0, 50, 0);

        pageLayout->addWidget(leftContainer, 1);
        pageLayout->addWidget(rightContainer, 1);

        stackedWidget->addWidget(page);

        return {page, leftLayout, rightLayout};
    }
}

FavoriteExposureSettingsView::FavoriteExposureSettingsView(Unique<FavoriteExposureSettingsViewModel> viewModel, QWidget *parent) :
    ExposureViewBase(std::move(viewModel), parent),
    ui(new Ui::FavoriteExposureSettingsView)
{
    ui->setupUi(this);

    createNavButtonsConfigs();
    setNewNavButtonsConfigs();
    setDefaultTitleBar("Exposure settings");

    connect(ui->newSettingsLabel, &QPushButton::clicked, this, [this]() { emit s_openView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW); });

    connect(ui->pushButtonLeft, &QPushButton::clicked, this, [this]() {
        const int previousIndex = ui->stackedFavorites->currentIndex() - 1;
        if (previousIndex >= 0)
            ui->stackedFavorites->setCurrentIndex(previousIndex);
        updateFavoritePageNavigation();
    });

    connect(ui->pushButtonRight, &QPushButton::clicked, this, [this]() {
        const int nextIndex = ui->stackedFavorites->currentIndex() + 1;
        if (nextIndex < ui->stackedFavorites->count())
            ui->stackedFavorites->setCurrentIndex(nextIndex);
        updateFavoritePageNavigation();
    });

    connect(ui->stackedFavorites, &QStackedWidget::currentChanged, this, [this](int) { updateFavoritePageNavigation(); });
}
FavoriteExposureSettingsView::~FavoriteExposureSettingsView()
{
}

void FavoriteExposureSettingsView::showEvent(QShowEvent *event)
{
    populateViewWithCurrentPreset();
    ExposureViewBase::showEvent(event);
}

void FavoriteExposureSettingsView::setNewNavButtonsConfigs()
{

    NavButtonConfig backBtn(
        "Back",
        ":/icons/back.svg",
        ID_BTN_BACK,
        std::bind(&FavoriteExposureSettingsView::onBackButtonClicked, this));
    addNavButton("left", backBtn);
}

void FavoriteExposureSettingsView::populateStackedFavorite(const QList<FavoriteExposureSettingButton *> &favoriteButtons)
{
    if (!ui || !ui->stackedFavorites)
        return;

    while (ui->stackedFavorites->count() > 0)
    {
        QWidget *page = ui->stackedFavorites->widget(0);
        if (!page)
            break;

        ui->stackedFavorites->removeWidget(page);
        if (page != ui->page)
            page->deleteLater();
    }

    ui->stackedFavorites->addWidget(ui->page);

    clearLayout(ui->firstPageLeft);
    clearLayout(ui->firstPageRight);

    const int totalButtons = favoriteButtons.size();
    const int pageCount    = std::max(1, (totalButtons + FAVORITES_PER_PAGE - 1) / FAVORITES_PER_PAGE);

    QVector<FavoritePageWidgets> extraPages;
    extraPages.reserve(std::max(0, pageCount - 1));

    for (int pageIndex = 1; pageIndex < pageCount; ++pageIndex)
    {
        extraPages.append(createFavoritePage(ui->stackedFavorites));
    }
    for (int buttonIndex = 0; buttonIndex < totalButtons; ++buttonIndex)
    {
        const int pageIndex       = buttonIndex / FAVORITES_PER_PAGE;
        const int indexInPage     = buttonIndex % FAVORITES_PER_PAGE;
        QVBoxLayout *targetLayout = nullptr;

        if (pageIndex == 0)
        {
            targetLayout = indexInPage < FAVORITES_PER_COLUMN ? ui->firstPageLeft : ui->firstPageRight;
        }
        else
        {
            const FavoritePageWidgets &pageWidgets = extraPages.at(pageIndex - 1);
            targetLayout                           = indexInPage < FAVORITES_PER_COLUMN ? pageWidgets.leftLayout : pageWidgets.rightLayout;
        }
        if (FavoriteExposureSettingButton *button = favoriteButtons.at(buttonIndex))
        {
            targetLayout->addWidget(button);
            setUpShadowedBoxStyle(button);
            connect(button, &QPushButton::clicked, this, [this, button]() {
                this->userChoseExposurePreset(button);
            });
        }
    }

    ui->stackedFavorites->setCurrentIndex(0);
    updateFavoritePageNavigation();
}

void FavoriteExposureSettingsView::updateFavoritePageNavigation()
{
    if (!ui || !ui->stackedFavorites)
        return;

    const int pageCount    = ui->stackedFavorites->count();
    const int currentIndex = ui->stackedFavorites->currentIndex();

    ui->pushButtonLeft->setEnabled(pageCount > 1 && currentIndex > 0);
    ui->pushButtonRight->setEnabled(pageCount > 1 && currentIndex < pageCount - 1);
}

void FavoriteExposureSettingsView::onBackButtonClicked()
{
    emit s_openView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW);
}

void FavoriteExposureSettingsView::onValidateButtonClicked()
{
}

void FavoriteExposureSettingsView::populateViewWithCurrentPreset()
{
    auto *vm = getViewModel<FavoriteExposureSettingsViewModel>();

    const auto res = vm->getAllExposureSettings();

    if (res)
    {
        populateStackedFavorite(*res);
    }
    else
    {
        qWarning() << "Failed to populate with current preset:" << res.unwrap_err();
    }
}

void FavoriteExposureSettingsView::userChoseExposurePreset(FavoriteExposureSettingButton *button)
{
    if (!button)
        return;

    auto *vm = getViewModel<FavoriteExposureSettingsViewModel>();
    if (!vm)
        return;

    QString presetName = button->titleText();

    if (vm->uiLoadExposurePreset(presetName))
    {
        emit s_openView(Kub3::UI::ViewId::RECAP_EXPOSURE_SETTINGS_VIEW);
    }
}