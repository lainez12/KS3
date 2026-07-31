#include <MFSM/states.macro.h>
#include <ViewModels/ExposureModeViewModel.h>

namespace Kub3::UI::ViewModels
{

    ExposureModeViewModel::ExposureModeViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(std::move(repo)) {}

    void ExposureModeViewModel::ps_onSystemStateChanged(MFSM::SystemStateKind stateKind)
    {
        m_currentSysState = stateKind;
        updateView();
    }

    void ExposureModeViewModel::ps_onPostureChanged(const MFSM::SystemPosture &posture)
    {
        m_posture = posture;
        updateView();
    }

    void ExposureModeViewModel::updateView()
    {
        bool floodExposureLocked = false;
        bool alignmentLocked     = false;

        if (m_currentSysState != MFSM::SystemStateKind::Operational)
        {
            floodExposureLocked = true;
            alignmentLocked     = true;
        }
        else
        {
            alignmentLocked = (m_posture.mask != MFSM::MaskPosture::Exposure) ||
                              (m_posture.wafer != MFSM::WaferPosture::AlignmentZone) ||
                              !m_posture.isLevelingValid;
        }

        emit s_setFloodExposureLock(floodExposureLocked);
        emit s_setAlignmentViewLock(alignmentLocked);
    }
}
