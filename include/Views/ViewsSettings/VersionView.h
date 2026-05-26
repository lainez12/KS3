#ifndef VERSIONVIEW_H
#define VERSIONVIEW_H

#include <QWidget>
#include <ViewModels/ViewModelsSettings/VersionViewModel.h>

#include "Views/ViewBase.h"

namespace Ui {
    class VersionView;
} // namespace UI

namespace Kub3::UI::Views::ViewsSettings {
    class VersionView final : public ViewBase {
        using VersionViewModel = Kub3::UI::ViewModels::ViewsModelSettings::VersionViewModel;

        Q_OBJECT

    public:
        explicit VersionView(Unique<VersionViewModel> viewModel, QWidget *parent = nullptr);
        ~VersionView();

    signals:

    public slots:

    private slots:

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void createNavButtonsConfigs();
        void configTitleBar();
        void onHomeButtonClicked(const QString &buttonId);
        void onBackButtonClicked(const QString &buttonId);

    private:
        Ui::VersionView *ui;

        bool m_isFlashingMode = false;
    };

} // namespace Kub3::UI::Views

using VersionView = Kub3::UI::Views::ViewsSettings::VersionView;

#endif