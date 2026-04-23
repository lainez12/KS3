#pragma once

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <MFSM/MasterFSM.h>
#include <ViewModels/BaseVisionViewModel.h>

namespace Kub3::UI::ViewModels
{

    class MachineStatusViewModel final : public BaseVisionViewModel
    {
        Q_OBJECT
    public:
        explicit MachineStatusViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);

        void loadConnections(void) override;

    signals:
        void s_booleanSensorUpdate(const QString &key, bool val);
        void s_integerSensorUpdate(const QString &key, int32_t val);
        void s_unsignedIntegerSensorUpdate(const QString &key, uint32_t val);

    public slots:
        void ps_handleSensorValueChanged(const std::string &key);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

}
