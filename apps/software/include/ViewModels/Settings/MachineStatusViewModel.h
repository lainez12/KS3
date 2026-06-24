#pragma once

#include <QRect>

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
        // Proxy signals
        void s_exposureSliderValueChanged(const QString &cameraId, double exposureRatio);
        void s_gainSliderValueChanged(const QString &cameraId, double gainRatio);
        void s_framerateValueChanged(const QString &cameraId, double framerate);
        void s_centeredZoomValueChanged(const QString &cameraId, double zoomFactor);
        void s_roiChanged(const QString &cameraId, QRect roi);

    public slots:
        void ps_handleSensorValueChanged(const std::string &key);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

}
