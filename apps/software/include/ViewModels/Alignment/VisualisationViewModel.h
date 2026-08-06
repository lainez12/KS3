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

    enum class CameraAxis : uint8_t
    {
        X,
        Y,
    };

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

        Q_OBJECT

    public:
        explicit VisualisationViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~VisualisationViewModel() override = default;

    public:
        void loadConnections(void) override;
        void setContactLimits(double maxForceGF, double toleranceGF);
        void ui_requestCameraManualMovement(CameraId camId, MovementKind kind, CameraDirection dir);
        void ui_requestAlignmentStageManualMovement(AlignmentStageId stageId, MovementKind kind, AlignmentStageDirection dir);
        void ui_requestZManualMovement(MovementKind kind, ZDirection dir);
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
        void s_setContactFormLock(bool lock);
        void s_setContactMaximumTarget(double valueGF);
        void s_setContactTolerance(double valueGF);
        void s_setCompressedAirButtonEnabled(bool enabled);

        void cmdRunCameraManualMovement(CameraId camId, MovementKind kind, CameraDirection dir);
        void cmdRunAlignmentStageManualMovement(AlignmentStageId stageId, MovementKind kind, AlignmentStageDirection dir);
        void cmdRunZManualMovement(MovementKind kind, ZDirection dir);

        void cmdRunCameraAbsoluteMovement(CameraId camId, double xPosMm, double yPosMm);
        void cmdRequestCameraFineMode(CameraId camId, bool active);
        void cmdRequestSubstrateFineMode(bool active);
        void cmdRequestSubstrateCompressedAir(bool enable);
        void cmdRequestExposureMode(void);
        void cmdRequestContact(double targetForceGrams);

    public slots:
        // From UI
        void ui_onPickUpXYClicked(CameraId id);
        void ui_onGoToXYClicked(CameraId id, const coords_2d_t &targetPosMm);
        void ui_onCamSpeedClicked(CameraId id);
        void ui_substrateSpeedClicked();
        void ui_startContactRoutine(double targetForceGrams);

        // From logic layer
        void ps_handleSensorValueChanged(const std::string &key);
        void ps_operationalSubstateKindChanged(MFSM::OperationalStateKind kind);
        void ps_onContactPhaseChanged(MFSM::ContactPhase contactPhase);
        void ps_onCompressedAirAuthorizedChanged(bool authorized);

    private:
        void syncContactUX(void);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;

        // Pad movement related state
        UMap<CameraId, bool> m_activeContinuousCameraMoves                 = {};
        UMap<AlignmentStageId, bool> m_activeContinuousAlignmentStageMoves = {};
        bool m_activeContinuousZMove                                       = false;

        // Displayed data state
        double m_zPositionsMm[3] = {0, 0, 0};
        UMap<CameraId, camera_data_t> m_camerasState;
        bool m_substrateFineMode            = false;
        bool m_substrateVacuumActive        = false;
        bool m_substrateCompressedAirActive = false;

        // Contact related
        double m_maxContactForceGF               = 800.0;
        double m_contactToleranceGF              = 50.0;
        MFSM::ContactPhase m_currentContactPhase = MFSM::ContactPhase::Free;
        bool m_compressedAirAuthorized           = false;
    };

} // namespace Kub3::UI::ViewModels::Alignment
