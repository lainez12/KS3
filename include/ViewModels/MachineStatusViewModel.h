#pragma once

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <ViewModels/IViewModel.h>

namespace Kub3::UI::ViewModels
{
    class MachineStatusViewModel final : public QObject, public IViewModel
    {
        Q_OBJECT
    public:
        explicit MachineStatusViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~MachineStatusViewModel() override;

        void loadConnections(void) override;
        void unloadConnections(void) override;

    signals:
        void s_booleanSensorUpdate(const QString &key, bool val);
        void s_integerSensorUpdate(const QString &key, int32_t val);
        void s_unsignedIntegerSensorUpdate(const QString &key, uint32_t val);

    private slots:
        void handleSensorValueChanged(const QString &key);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };
}
