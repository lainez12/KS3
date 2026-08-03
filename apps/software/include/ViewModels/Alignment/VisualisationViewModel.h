#pragma once

#include <unordered_map>

#include <Common/Enums.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <ViewModels/BaseVisionViewModel.h>
#include <utils.h>

namespace Kub3::UI::ViewModels::Alignment
{

    enum class CameraAxis : uint8_t
    {
        X,
        Y,
    };

    class VisualisationViewModel final : public BaseVisionViewModel
    {
        Q_OBJECT

    public:
        explicit VisualisationViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~VisualisationViewModel() override = default;

    public:
        void loadConnections(void) override;
        void uiRequestCameraMovement(CameraId camId, MovementKind kind, CameraDirection dir);
        void uiRequestAlignmentStageMovement(AlignmentStageId stageId, MovementKind kind, AlignmentStageDirection dir);

    signals:
        void s_maskingDistanceUpdate(double distMm);
        void s_cameraPositionUpdate(CameraId camId, CameraAxis axis, double value);
        void s_compressedAirUpdate(bool active);
        void s_vacuumUpdate(bool active);

        void cmdRunCameraMovement(CameraId camId, MovementKind kind, CameraDirection dir);
        void cmdRunAlignmentStageMovement(AlignmentStageId stageId, MovementKind kind, AlignmentStageDirection dir);

    public slots:
        void ps_handleSensorValueChanged(const std::string &key);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        std::unordered_map<CameraId, bool> m_activeContinuousCameraMoves                 = {};
        std::unordered_map<AlignmentStageId, bool> m_activeContinuousAlignmentStageMoves = {};
        double m_zPositionsMm[3]                                                         = {0, 0, 0};
    };

} // namespace Kub3::UI::ViewModels::Alignment
