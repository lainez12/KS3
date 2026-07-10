#pragma once

#include <unordered_map>

#include <Common/Enums.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <ViewModels/BaseVisionViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{

    class VisualisationViewModel final : public BaseVisionViewModel
    {
        Q_OBJECT

    public:
        explicit VisualisationViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~VisualisationViewModel() override;

    signals:
        void cmdRunCameraMovement(CameraId camId, MovementKind kind, CameraDirection dir);
        void cmdRunAlignmentStageMovement(AlignmentStageId stageId, MovementKind kind, AlignmentStageDirection dir);

    public:
        void uiRequestCameraMovement(CameraId camId, MovementKind kind, CameraDirection dir);
        void uiRequestAlignmentStageMovement(AlignmentStageId stageId, MovementKind kind, AlignmentStageDirection dir);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        std::unordered_map<CameraId, bool> m_activeContinuousCameraMoves                 = {};
        std::unordered_map<AlignmentStageId, bool> m_activeContinuousAlignmentStageMoves = {};
    };

} // namespace Kub3::UI::ViewModels::Alignment
