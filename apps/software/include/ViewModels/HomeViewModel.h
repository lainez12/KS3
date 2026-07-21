#pragma once

#include <Common/Enums.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <MFSM/MasterFSM.h>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels
{
    class HomeViewModel final : public QObject, public BaseViewModel
    {
        Q_OBJECT
    public:
        explicit HomeViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~HomeViewModel() override;

    signals:
        void s_cancelOperation();
        void s_initializationRequest();

    public slots:
        void uiRequestCancel();
        void uiRequestInitialization();
        void uiRequestDrawerInsertion(DrawerTarget tgt);
        void uiRequestDrawerEjection(DrawerTarget tgt);

    public slots:
        void ps_errorOccurred(const Kub3::MFSM::ErrorPayload &payload);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };
}