#pragma once

#include <QWidget>
#include <ViewModels/HomeViewModel.h>
#include <Views/ViewBase.h>

namespace Ui
{
    class HomeView;
}

namespace Kub3::UI::Views
{
    class HomeView final : public ViewBase
    {
        Q_OBJECT

    public:
        explicit HomeView(Unique<ViewModels::HomeViewModel> viewModel, QWidget *parent = nullptr);
        ~HomeView() override;

        void resizeEvent(QResizeEvent *event) override;

    public slots:
        // Triggered by the view model
        void setDrawerActionsLocked(bool locked);
        void setExposureModeLocked(bool locked);
        void setHomingLocked(bool locked);
        void setInitializationLocked(bool locked);

    private:
        void setupConnections();
        void updateMachineLogo(int h);
        void createNavButtonsConfigs() override;
        void configTitleBar() override;

    private:
        Ui::HomeView *ui;
        // Typed, non-owning pointer for fast, clean access.
        // Guaranteed valid because ViewBase owns the unique_ptr.
        ViewModels::HomeViewModel *m_vm;
    };
} // namespace Kub3::UI::Views

using HomeView = Kub3::UI::Views::HomeView;
