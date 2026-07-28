#pragma once

#include <QPixmap>
#include <QWidget>

#include <ViewModels/ExposureMenuViewModel.h>
#include <Views/Components/NavButton.h>
#include <Views/Components/SpinningLabel.h>
#include <Views/ViewBase.h>

namespace Ui
{
    class ExposureMenuView;
}

namespace Kub3::UI::Views
{

    class ExposureMenuView final : public ViewBase
    {
        using ExposureMenuViewModel = Kub3::UI::ViewModels::ExposureMenuViewModel;
        using StepStatus            = ExposureMenuViewModel::StepStatus;

        Q_OBJECT

    public:
        explicit ExposureMenuView(Unique<ExposureMenuViewModel> viewModel, QWidget *parent = nullptr);
        ~ExposureMenuView();

    private:
        void createNavButtonsConfigs();
        void configTitleBar();
        void setupConnections();
        void checkPixmaps();
        void updateStatusPixmap(SpinningLabel *lblStatus, StepStatus status);

    private slots:
        // Actions
        void onBtnLoadMaskToExposureClicked();
        void onBtnLoadWaferToExposureClicked();
        void onBtnStartAutoleveling();
        void onBackButtonClicked();
        // Locks
        void setAllLocks(bool locked);
        void setMaskLoadLock(bool locked);
        void setMaskCancelLock(bool locked);
        void setWaferLoadLock(bool locked);
        void setWaferCancelLock(bool locked);
        void setAutolevelLock(bool locked);
        void setAutolevelCancelLock(bool locked);
        void setSaveHorizontalityLocked(bool locked);
        void setLoadHorizontalityLocked(bool locked);
        void setContinueToExposureLocked(bool locked);
        // Status indicators
        void setMaskStatus(StepStatus status);
        void setWaferStatus(StepStatus status);
        void setAutolevelStatus(StepStatus status);
        // Logs
        void appendMaskSequenceLog(const QString &htmlLogLine);
        void appendWaferSequenceLog(const QString &htmlLogLine);
        void appendAutolevelSequenceLog(const QString &htmlLogLine);
        void clearMaskSequenceLogs();
        void clearWaferSequenceLogs();
        void clearAutolevelSequenceLogs();

    private:
        // MVVM related
        Ui::ExposureMenuView *ui;
        ExposureMenuViewModel *m_vm = nullptr;

        // Pixmaps
        QPixmap m_successPixmap;
        QPixmap m_failurePixmap;
        QPixmap m_runningPixmap;
        QPixmap m_idlePixmap;
    };
}

using ExposureMenuView = Kub3::UI::Views::ExposureMenuView;
