#pragma once

#include <QList>
#include <QObject>
#include <QRect>
#include <QString>

#include <ViewModels/BaseViewModel.h>
#include <ViewModels/BaseVisionViewModel.h>
#include <utils.h>

namespace Kub3::Tools::Tester
{

    struct CameraModuleConfig {
        QString cameraId;
        Optional<QString> focalId;
        Optional<QString> lightId;
    };

    class CamerasTestViewModel : public UI::ViewModels::BaseVisionViewModel
    {
        Q_OBJECT

    public:
        explicit CamerasTestViewModel(QObject *parent = nullptr);
        ~CamerasTestViewModel() override = default;

        void setCameraModules(const QList<CameraModuleConfig> &configs);

    public slots:
        void uiToggleFocal(const QString &focalId, bool en);
        void uiUpdateFocalValue(const QString &focalId, double value);

        void uiToggleLight(const QString &lightId, bool enabled);
        void uiUpdateLightValue(const QString &lightId, double value);

        void uiEmergencyStop();

    signals:
        void s_cameraModulesLoaded(const QList<CameraModuleConfig> &configs);

        // Outbound commands to Controller
        void cmdToggleFocal(const QString &focalId, bool en);
        void cmdUpdateFocalValue(const QString &focalId, double value);
        void cmdToggleLight(const QString &lightId, bool en);
        void cmdUpdateLightValue(const QString &lightId, double value);
        void cmdEmergencyStop();

        // Proxy signals
        void s_exposureSliderValueChanged(const QString &cameraId, double exposureRatio);
        void s_gainSliderValueChanged(const QString &cameraId, double gainRatio);
        void s_framerateValueChanged(const QString &cameraId, double framerate);
        void s_centeredZoomValueChanged(const QString &cameraId, double zoomFactor);
        void s_roiChanged(const QString &cameraId, QRect roi);

    private:
        QList<CameraModuleConfig> m_cameraConfigs;
    };

} // namespace Kub3::Tools::Tester
