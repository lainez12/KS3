#ifndef VISUALISATIONVIEW_H
#define VISUALISATIONVIEW_H

#include <Common/Enums.h>
#include <QLabel>
#include <QWidget>
#include <ViewModels/Alignment/VisualisationViewModel.h>
#include <Views/AlignmentViewBase.h>

#include <ViewModels/Alignment/AlignmentParametersPersistence.h>
#include <Views/Components/HardForceContactForm.h>
#include <Views/Components/NavButton.h>
#include <Views/Components/RealPositionCameras.h>
#include <Views/KeyboardConnections.h>
#include <Views/Traits/PadReceiverViewTrait.h>
#include <Views/ViewBase.h>

namespace Ui
{
    class VisualisationView;
}

namespace Kub3::UI::Views
{

    class VisualisationView final : public AlignmentViewBase, public PadReceiverViewTrait
    {
        using CameraAxis             = ViewModels::Alignment::CameraAxis;
        using VisualisationViewModel = ViewModels::Alignment::VisualisationViewModel;
        using coords_2d_t            = ViewModels::Alignment::coords_2d_t;
        using alignment_parameter_t  = ViewModels::Alignment::Persistence::alignment_parameter_t;

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
        void onMaskingDistanceUpdate(double distMm);
        void onCameraPositionUpdate(CameraId camId, CameraAxis axis, double value);
        void onVacuumUpdate(bool active);
        void onCompressedAirUpdate(bool active);
        void onPickedUpCoordinatesUpdated(CameraId camId, const coords_2d_t &coordinatesMm);
        void onCamerasFineModeUpdated(CameraId camId, bool fineModeActive);
        void onSubstrateFineModeUpdated(bool active);
        void onGoToBtnClicked(CameraId camId);
        void onPreparingExposureMode(void);

    private:
        // Setup routines splitting up the constructor
        void setupUI();
        void setupConnections();
        void setupBindings();
        void setupNavButtons();

        // Hardware movement dispatchers
        void cameraMovement(CameraId camId, MovementKind kind, CameraDirection dir);
        void alignmentStageMovement(AlignmentStageId stageId, MovementKind kind, AlignmentStageDirection dir);

        // Nav configurations
        void setNewNavButtonsConfigs();
        void onValidateButtonClicked() override;
        void onBackButtonClicked() override;
        void onSaveButtonClicked();
        void onLoadButtonClicked();
        void onScreenshotButtonClicked();
        void onHardForceContButtonClicked(const QString &buttonId);
        void onSpeedMotorSubstButtonClicked();
        void onSwitchVacumAirButtonClicked();
        void onAntiCollisionButtonClicked();
        void onVisualMarkButtonClicked(const QString &buttonId);
        void onMeasurementButtonClicked();
        void closeHardForceContactFormIfNeeded();

        alignment_parameter_t getAlignmentParameter();

        // UI helpers
        void updateOverlayPositions();

    private:
        Ui::VisualisationView *ui;
        KeyboardConnections m_keyboard;

        // Custom widget artifacts
        RealPositionCameras *m_mapPositionCameras;
        HardForceContactForm *m_hardForceContactForm;
    };

}

using VisualisationView = Kub3::UI::Views::VisualisationView;

#endif
