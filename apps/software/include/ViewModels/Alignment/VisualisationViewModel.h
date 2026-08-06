#pragma once

#include <Common/Enums.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <MFSM/states.operational.h>
#include <ViewModels/Alignment/AlignmentParametersPersistence.h>
#include <ViewModels/BaseVisionViewModel.h>
#include <utils.h>

namespace Kub3::UI::ViewModels::Alignment
{
    typedef struct coords_2d_s {
        double x;
        double y;
    } coords_2d_t;

    typedef struct camera_data_s {
        coords_2d_t currentPositionMm     = {.x = 0.0, .y = 0.0};
        coords_2d_t pickedUpCoordinatesMm = {.x = 0.0, .y = 0.0};
        bool fineSpeedSelected            = false;
    } camera_data_t;

    class VisualisationViewModel final : public BaseVisionViewModel
    {
        using alignment_parameter_t = Persistence::alignment_parameter_t;
        using CameraAxis            = Kub3::CameraAxis;

        Q_OBJECT

    public:
        explicit VisualisationViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~VisualisationViewModel() override = default;

    public:
        void loadConnections(void) override;
        void ui_requestCameraMovement(CameraId camId, MovementKind kind, CameraDirection dir);
        void ui_requestAlignmentStageMovement(AlignmentStageId stageId, MovementKind kind, AlignmentStageDirection dir);
        void ui_requestSaveParameters(const alignment_parameter_t &parameter);
        void ui_compressedAirSwitchClicked();
        void ui_proceedToExposure();

    signals:
        void s_maskingDistanceUpdate(double distMm);
        void s_cameraPositionUpdate(CameraId camId, CameraAxis axis, double value);
        void s_compressedAirUpdate(bool active);
        void s_vacuumUpdate(bool active);
        void s_pickedUpCoordinatesUpdated(CameraId camId, const coords_2d_t &coordinatesMm);
        void s_camerasFineModeUpdated(CameraId camId, bool active);
        void s_substrateFineModeUpdated(bool fineModeActive);
        void s_saveParametersAlignment(const alignment_parameter_t &parameter);
        void s_preparingExposureMode(void);

        void cmdRunCameraMovement(CameraId camId, MovementKind kind, CameraDirection dir);
        void cmdRunAlignmentStageMovement(AlignmentStageId stageId, MovementKind kind, AlignmentStageDirection dir);
        void cmdRunCameraAbsoluteMovement(CameraId camId, double xPosMm, double yPosMm);
        void cmdRequestCameraFineMode(CameraId camId, bool active);
        void cmdRequestSubstrateFineMode(bool active);
        void cmdRequestSubstrateCompressedAir(bool enable);
        void cmdRequestExposureMode(void);

    public slots:
        // From UI
        void ui_onPickUpXYClicked(CameraId id);
        void ui_onGoToXYClicked(CameraId id, const coords_2d_t &targetPosMm);
        void ui_onCamSpeedClicked(CameraId id);
        void ui_substrateSpeedClicked();
        void ps_handleLoadParameterSelected(const alignment_parameter_t &parameter);

        // From logic layer
        void ps_handleSensorValueChanged(const std::string &key);
        void ps_operationalSubstateKindChanged(MFSM::OperationalStateKind kind);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;

        // Pad movement related state
        UMap<CameraId, bool> m_activeContinuousCameraMoves                 = {};
        UMap<AlignmentStageId, bool> m_activeContinuousAlignmentStageMoves = {};

        // Displayed data state
        double m_zPositionsMm[3] = {0, 0, 0};
        UMap<CameraId, camera_data_t> m_camerasState;
        bool m_substrateFineMode            = false;
        bool m_substrateVacuumActive        = false;
        bool m_substrateCompressedAirActive = false;
    };

} // namespace Kub3::UI::ViewModels::Alignment
