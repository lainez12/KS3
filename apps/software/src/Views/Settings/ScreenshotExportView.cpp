#include "ui_ScreenshotExportView.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontMetrics>
#include <QFrame>
#include <QGridLayout>
#include <QImageReader>
#include <QLabel>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QVBoxLayout>

#include <Views/Components/Colors.h>
#include <Views/Components/ScreenshotThumbnailWidget.h>
#include <Views/Settings/ScreenshotExportView.h>

constexpr int SCREENSHOT_COLUMNS = 5;

ScreenshotExportView::ScreenshotExportView(Unique<ScreenshotExportViewModel> viewModel, QWidget *parent) :
    SettingsViewBase(std::move(viewModel), parent),
    ui(new Ui::ScreenshotExportView)
{
    ui->setupUi(this);

    setDefaultTitleBar("Screenshot Export");

    m_scrollAreaContents = ui->scrollAreaWidgetContents;
    m_gridLayout         = new QGridLayout(m_scrollAreaContents);
    m_gridLayout->setContentsMargins(24, 24, 24, 24);
    m_gridLayout->setHorizontalSpacing(18);
    m_gridLayout->setVerticalSpacing(18);
    m_scrollAreaContents->setLayout(m_gridLayout);

    createNewNavButtonConfigs();
    refreshScreenshots();
}
ScreenshotExportView::~ScreenshotExportView()
{
}

void ScreenshotExportView::createNewNavButtonConfigs()
{
    NavButtonConfig selectAll(
        "Select all",
        ":/icons/select-all.svg",
        "R",
        std::bind(&ScreenshotExportView::onSelectAllButtonClicked, this, std::placeholders::_1));
    addNavButton("right", selectAll);

    NavButtonConfig deleteButton(
        "Delete",
        ":/icons/delete.svg",
        "D",
        std::bind(&ScreenshotExportView::onDeleteButtonClicked, this, std::placeholders::_1));
    addNavButton("right", deleteButton);

    NavButtonConfig deselectAll(
        "Deselect all",
        ":/icons/deselect-all.svg",
        "E",
        std::bind(&ScreenshotExportView::onDeselectAllButtonClicked, this, std::placeholders::_1));
    addNavButton("right", deselectAll);

    NavButtonConfig loadOnUsb(
        "Load on USB",
        ":/icons/load-on-usb.svg",
        "L",
        std::bind(&ScreenshotExportView::onLoadOnUsbButtonClicked, this, std::placeholders::_1));
    addNavButton("right", loadOnUsb);
}

QString ScreenshotExportView::screenshotsDirectoryPath() const
{
    return QDir::home().filePath("screenshots");
}

void ScreenshotExportView::clearScreenshotGrid()
{
    while (QLayoutItem *item = m_gridLayout->takeAt(0))
    {
        if (QWidget *widget = item->widget())
        {
            widget->deleteLater();
        }
        delete item;
    }

    m_thumbnailCards.clear();
}

void ScreenshotExportView::refreshScreenshots()
{
    clearScreenshotGrid();

    QDir screenshotsDir(screenshotsDirectoryPath());
    const QStringList nameFilters = {"*.png", "*.jpg", "*.jpeg", "*.bmp", "*.webp"};
    const QFileInfoList files     = screenshotsDir.entryInfoList(nameFilters, QDir::Files, QDir::Name | QDir::IgnoreCase);

    if (files.isEmpty())
    {
        auto *placeholder = new QLabel("No screenshots found", m_scrollAreaContents);
        placeholder->setAlignment(Qt::AlignCenter);
        placeholder->setStyleSheet("color: #6B7C8F; font-size: 22px; padding: 48px;");
        m_gridLayout->addWidget(placeholder, 0, 0, 1, SCREENSHOT_COLUMNS, Qt::AlignCenter);
        return;
    }

    int row              = 0;
    int column           = 0;
    const int cardWidth  = qMax(240, (ui->scrollArea->viewport()->width() - 24 * 2 - (SCREENSHOT_COLUMNS - 1) * 18) / SCREENSHOT_COLUMNS);
    const int cardHeight = qMax(240, static_cast<int>(cardWidth * 0.82));

    for (const QFileInfo &fileInfo : files)
    {
        auto *card = new ScreenshotThumbnailWidget(fileInfo.absoluteFilePath(), m_scrollAreaContents);
        card->setFixedSize(cardWidth, cardHeight);
        m_gridLayout->addWidget(card, row, column);
        m_thumbnailCards.append(card);

        ++column;
        if (column >= SCREENSHOT_COLUMNS)
        {
            column = 0;
            ++row;
        }
    }
}

void ScreenshotExportView::setAllThumbnailsSelected(bool selected)
{
    for (ScreenshotThumbnailWidget *card : m_thumbnailCards)
    {
        card->setSelected(selected);
    }
}

void ScreenshotExportView::resizeEvent(QResizeEvent *event)
{
    SettingsViewBase::resizeEvent(event);

    if (!m_gridLayout || m_thumbnailCards.isEmpty())
    {
        return;
    }

    const int cardWidth  = qMax(240, (ui->scrollArea->viewport()->width() - 24 * 2 - (SCREENSHOT_COLUMNS - 1) * 18) / SCREENSHOT_COLUMNS);
    const int cardHeight = qMax(240, static_cast<int>(cardWidth * 0.82));

    for (ScreenshotThumbnailWidget *card : m_thumbnailCards)
    {
        card->setFixedSize(cardWidth, cardHeight);
        card->updatePreviewSize();
        card->updateElidedName();
    }
}

void ScreenshotExportView::onSelectAllButtonClicked(const QString &buttonId)
{
    Q_UNUSED(buttonId);
    setAllThumbnailsSelected(true);
}

void ScreenshotExportView::onDeleteButtonClicked(const QString &buttonId)
{
    Q_UNUSED(buttonId);

    bool removedAny = false;
    for (ScreenshotThumbnailWidget *card : m_thumbnailCards)
    {
        if (!card->isSelected())
        {
            continue;
        }

        removedAny = QFile::remove(card->filePath()) || removedAny;
    }

    if (removedAny)
    {
        refreshScreenshots();
    }
}

void ScreenshotExportView::onDeselectAllButtonClicked(const QString &buttonId)
{
    Q_UNUSED(buttonId);
    setAllThumbnailsSelected(false);
}

void ScreenshotExportView::onLoadOnUsbButtonClicked(const QString &buttonId)
{
    Q_UNUSED(buttonId);
}