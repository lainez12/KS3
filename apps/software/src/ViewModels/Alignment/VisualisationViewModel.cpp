#include <ViewModels/Alignment/VisualisationViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{

    VisualisationViewModel::VisualisationViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseVisionViewModel(parent),
        m_repo(repo)
    {
    }

    VisualisationViewModel::~VisualisationViewModel()
    {
    }

    void VisualisationViewModel::uiRequestCameraMovement(CameraId camId, MovementKind kind, CameraDirection dir)
    {
        if (kind == MovementKind::CONTINUOUS)
            m_activeContinuousCameraMoves[camId] = true;
        else if (kind == MovementKind::STOP)
        {
            auto it = m_activeContinuousCameraMoves.find(camId);

            if (it == m_activeContinuousCameraMoves.end() || !m_activeContinuousCameraMoves.at(camId))
                return; // Block stop signal here if movement was not continuous
            m_activeContinuousCameraMoves[camId] = false;
        }

        // Send request for movement
        emit cmdRunCameraMovement(camId, kind, dir);
    }

    void VisualisationViewModel::uiRequestAlignmentStageMovement(
        AlignmentStageId stageId,
        MovementKind kind,
        AlignmentStageDirection dir)
    {
        if (kind == MovementKind::CONTINUOUS)
            m_activeContinuousAlignmentStageMoves[stageId] = true;
        else if (kind == MovementKind::STOP)
        {
            auto it = m_activeContinuousAlignmentStageMoves.find(stageId);

            if (it == m_activeContinuousAlignmentStageMoves.end() || !m_activeContinuousAlignmentStageMoves.at(stageId))
                return; // Block stop signal here if movement was not continuous
            m_activeContinuousAlignmentStageMoves[stageId] = false;
        }

        // Send request for movement
        emit cmdRunAlignmentStageMovement(stageId, kind, dir);
    }

} // namespace Kub3::UI::ViewModels::Alignment