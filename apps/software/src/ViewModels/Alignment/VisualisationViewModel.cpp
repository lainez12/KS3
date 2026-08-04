#include <Common/Enums.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <HAL/MachineStatus/virtual_labels.h>
#include <MFSM/states.operational.h>
#include <ViewModels/Alignment/VisualisationViewModel.h>
#include <utils.h>

namespace
{
    constexpr uint32_t leftCamXEncHash       = Kub3::Utils::ConstexprStringHash::hash(LEFT_CAMERA_X_ENCODER_MM);
    constexpr uint32_t leftCamYEncHash       = Kub3::Utils::ConstexprStringHash::hash(LEFT_CAMERA_Y_ENCODER_MM);
    constexpr uint32_t rightCamXEncHash      = Kub3::Utils::ConstexprStringHash::hash(RIGHT_CAMERA_X_ENCODER_MM);
    constexpr uint32_t rightCamYEncHash      = Kub3::Utils::ConstexprStringHash::hash(RIGHT_CAMERA_Y_ENCODER_MM);
    constexpr uint32_t zLeftEncHash          = Kub3::Utils::ConstexprStringHash::hash(Z_LEFT_ENCODER_MM);
    constexpr uint32_t zRightEncHash         = Kub3::Utils::ConstexprStringHash::hash(Z_RIGHT_ENCODER_MM);
    constexpr uint32_t zBackEncHash          = Kub3::Utils::ConstexprStringHash::hash(Z_BACK_ENCODER_MM);
    constexpr uint32_t waferVacuumActiveHash = Kub3::Utils::ConstexprStringHash::hash(WAFER_VACUUM_ACTIVE);
    constexpr uint32_t waferAirActiveHash    = Kub3::Utils::ConstexprStringHash::hash(WAFER_COMPRESSED_AIR_ACTIVE);
}

namespace Kub3::UI::ViewModels::Alignment
{

    VisualisationViewModel::VisualisationViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseVisionViewModel(parent),
        m_repo(repo)
    {
    }

    void VisualisationViewModel::loadConnections(void)
    {
        BaseViewModel::loadConnections();

        // Update values on view displayed
        ps_handleSensorValueChanged(Z_LEFT_ENCODER_MM);
        ps_handleSensorValueChanged(LEFT_CAMERA_X_ENCODER_MM);
        ps_handleSensorValueChanged(LEFT_CAMERA_Y_ENCODER_MM);
        ps_handleSensorValueChanged(RIGHT_CAMERA_X_ENCODER_MM);
        ps_handleSensorValueChanged(RIGHT_CAMERA_Y_ENCODER_MM);
        ps_handleSensorValueChanged(WAFER_VACUUM_ACTIVE);
        ps_handleSensorValueChanged(WAFER_COMPRESSED_AIR_ACTIVE);
    }

    void VisualisationViewModel::ui_requestCameraMovement(CameraId camId, MovementKind kind, CameraDirection dir)
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

    void VisualisationViewModel::ui_requestAlignmentStageMovement(
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

    void VisualisationViewModel::ui_requestSaveParameters(const alignment_parameter_t &parameter)
    {
        emit s_saveParametersAlignment(parameter);
    }

    void VisualisationViewModel::ui_proceedToExposure()
    {
        emit cmdRequestExposureMode();
    }

    void VisualisationViewModel::ui_onPickUpXYClicked(CameraId id)
    {
        const coords_2d_t currentPos = m_camerasState[id].currentPositionMm;

        m_camerasState[id].pickedUpCoordinatesMm = currentPos;
        emit s_pickedUpCoordinatesUpdated(id, currentPos);
    }

    void VisualisationViewModel::ui_onGoToXYClicked(CameraId id, const coords_2d_t &targetPosMm)
    {
        emit cmdRunCameraAbsoluteMovement(id, targetPosMm.x, targetPosMm.y);
    }

    void VisualisationViewModel::ui_onCamSpeedClicked(CameraId id)
    {
        const bool newValue = !m_camerasState[id].fineSpeedSelected;

        m_camerasState[id].fineSpeedSelected = newValue; // Internal udpate
        emit cmdRequestCameraFineMode(id, newValue);     // To logic layer
        emit s_camerasFineModeUpdated(id, newValue);     // To view
    }

    void VisualisationViewModel::ui_substrateSpeedClicked()
    {
        m_substrateFineMode = !m_substrateFineMode;            // Internal update
        emit cmdRequestSubstrateFineMode(m_substrateFineMode); // To logic layer
        emit s_substrateFineModeUpdated(m_substrateFineMode);  // To view
    }

    void VisualisationViewModel::ps_handleSensorValueChanged(const std::string &key)
    {
        const auto sendCamPosUpdate = [&](CameraId id, CameraAxis axis, double &valueHolder) {
            if (auto valOpt = HAL::MS::tryRead<double>(m_repo, key); valOpt.has_value())
            {
                valueHolder = valOpt.value();
                emit s_cameraPositionUpdate(id, axis, valOpt.value());
            }
        };

        const auto emitBoolUpdateSignal = [&](void (VisualisationViewModel::*_signal)(bool)) {
            if (auto valOpt = HAL::MS::tryRead<bool>(m_repo, key); valOpt.has_value())
            {
                emit(this->*_signal)(valOpt.value());
            }
        };

        const auto onZPositionUpdate = [&](double &valueHolder, const char *refValKey, bool update = false) {
            if (!refValKey)
                return;

            auto refValOpt = HAL::MS::tryRead<double>(m_repo, refValKey);
            auto valOpt    = HAL::MS::tryRead<double>(m_repo, key);

            if (!refValOpt.has_value() || !valOpt.has_value())
                return;

            valueHolder = valOpt.value();
            if (update)
            {
                emit s_maskingDistanceUpdate(refValOpt.value() - valueHolder);
            }
        };

        switch (Utils::ConstexprStringHash::hash(key))
        {
        case leftCamXEncHash:
            sendCamPosUpdate(CameraId::LEFT, CameraAxis::X, m_camerasState[CameraId::LEFT].currentPositionMm.x);
            break;
        case leftCamYEncHash:
            sendCamPosUpdate(CameraId::LEFT, CameraAxis::Y, m_camerasState[CameraId::LEFT].currentPositionMm.y);
            break;
        case rightCamXEncHash:
            sendCamPosUpdate(CameraId::RIGHT, CameraAxis::X, m_camerasState[CameraId::RIGHT].currentPositionMm.x);
            break;
        case rightCamYEncHash:
            sendCamPosUpdate(CameraId::RIGHT, CameraAxis::Y, m_camerasState[CameraId::RIGHT].currentPositionMm.y);
            break;
        case zLeftEncHash:
            onZPositionUpdate(m_zPositionsMm[0], V_Z_LEFT_MASK_POSITION_MM, true);
            break;
        case waferVacuumActiveHash:
            emitBoolUpdateSignal(&VisualisationViewModel::s_vacuumUpdate);
            break;
        case waferAirActiveHash:
            emitBoolUpdateSignal(&VisualisationViewModel::s_compressedAirUpdate);
            break;
        default:
            break;
        }
    }

    void VisualisationViewModel::ps_operationalSubstateKindChanged(MFSM::OperationalStateKind kind)
    {
        if (kind == MFSM::OperationalStateKind::PreparingExposure)
        {
            emit s_preparingExposureMode();
        }
    }

} // namespace Kub3::UI::ViewModels::Alignment