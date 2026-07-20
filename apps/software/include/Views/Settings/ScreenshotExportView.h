#ifndef SCREENSHOTEXPORTVIEW_H
#define SCREENSHOTEXPORTVIEW_H

#include <QGridLayout>
#include <QResizeEvent>
#include <QVector>
#include <QWidget>
#include <ViewModels/Settings/ScreenshotExportViewModel.h>
#include <Views/SettingsViewBase.h>

#include <Views/ViewBase.h>

namespace Ui
{
    class ScreenshotExportView;
} // namespace UI

class ScreenshotThumbnailWidget;

namespace Kub3::UI::Views::ViewsSettings
{
    class ScreenshotExportView final : public SettingsViewBase
    {
        using ScreenshotExportViewModel = Kub3::UI::ViewModels::Settings::ScreenshotExportViewModel;

        Q_OBJECT

    public:
        explicit ScreenshotExportView(Unique<ScreenshotExportViewModel> viewModel, QWidget *parent = nullptr);
        ~ScreenshotExportView();

    private:
        void createNewNavButtonConfigs();
        void refreshScreenshots();
        void clearScreenshotGrid();
        void setAllThumbnailsSelected(bool selected);
        QString screenshotsDirectoryPath() const;

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private slots:
        void onSelectAllButtonClicked();
        void onDeleteButtonClicked();
        void onDeselectAllButtonClicked();
        void onLoadOnUsbButtonClicked();

    private:
        Ui::ScreenshotExportView *ui;
        QWidget *m_scrollAreaContents = nullptr;
        QGridLayout *m_gridLayout     = nullptr;
        QVector<ScreenshotThumbnailWidget *> m_thumbnailCards;
    };

} // namespace Kub3::UI::Views

using ScreenshotExportView = Kub3::UI::Views::ViewsSettings::ScreenshotExportView;

#endif