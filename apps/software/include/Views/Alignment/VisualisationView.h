#ifndef VISUALISATIONVIEW_H
#define VISUALISATIONVIEW_H

#include <QLabel>
#include <QWidget>
#include <ViewModels/Alignment/VisualisationViewModel.h>
#include <Views/AlignmentViewBase.h>

#include <Views/Components/HardForceContactForm.h>
#include <Views/Components/NavButton.h>
#include <Views/Components/RealPositionCameras.h>
#include <Views/KeyboardConnections.h>
#include <Views/ViewBase.h>

namespace Ui
{
    class VisualisationView;
}

namespace Kub3::UI::Views::ViewsAlignment
{

    class VisualisationView final : public AlignmentViewBase
    {
        using VisualisationViewModel = Kub3::UI::ViewModels::Alignment::VisualisationViewModel;
        Q_OBJECT

    public:
        explicit VisualisationView(Unique<VisualisationViewModel> viewModel, QWidget *parent = nullptr);
        ~VisualisationView();

    public slots:
        void mapPositionCamerasOpenMap();
        void mapPositionCamerasCloseMap();

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private slots:
        void leftCamConfigToggled(bool checked);
        void rightCamConfigToggled(bool checked);
        void navButtonToggled(NavButton *button, QWidget *widget);

    private:
        void setNewNavButtonsConfigs();
        void onValidateButtonClicked(const QString &buttonId) override;
        void onBackButtonClicked(const QString &buttonId) override;
        void onSaveButtonClicked(const QString &buttonId);
        void onLoadButtonClicked(const QString &buttonId);
        void onScreenshotButtonClicked(const QString &buttonId);
        void onHardForceContButtonClicked(const QString &buttonId);
        void onSpeedMotorSubstButtonClicked(const QString &buttonId);
        void onswitchVacumAirButtonClicked(const QString &buttonId);
        void onAntiCollisionButtonClicked(const QString &buttonId);
        void onVisualMarkButtonClicked(const QString &buttonId);
        void onMeasurementButtonClicked(const QString &buttonId);
        void closeHardForceContactFormIfNeeded();

        // UI helpers
        void updateOverlayPositions();

    private:
        Ui::VisualisationView *ui;
        KeyboardConnections m_keyboard;

        // Custom widget artifacts kept
        RealPositionCameras *m_mapPositionCameras;
        HardForceContactForm *m_hardForceContactForm;
    };

}

using VisualisationView = Kub3::UI::Views::ViewsAlignment::VisualisationView;

#endif
