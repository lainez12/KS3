#include <ViewModels/CamerasTestViewModel.h>

namespace Kub3::Tools::Tester
{

    CamerasTestViewModel::CamerasTestViewModel(QObject *parent) :
        UI::ViewModels::BaseVisionViewModel(parent) {}

    void CamerasTestViewModel::setCameraModules(const QList<CameraModuleConfig> &configs)
    {
        m_cameraConfigs = configs;
        emit s_cameraModulesLoaded(m_cameraConfigs);
    }

    void CamerasTestViewModel::uiToggleFocal(const QString &focalId, bool en)
    {
        emit cmdToggleFocal(focalId, en);
    }

    void CamerasTestViewModel::uiUpdateFocalValue(const QString &focalId, double value)
    {
        emit cmdUpdateFocalValue(focalId, value);
    }

    void CamerasTestViewModel::uiToggleLight(const QString &lightId, bool en)
    {
        emit cmdToggleLight(lightId, en);
    }

    void CamerasTestViewModel::uiUpdateLightValue(const QString &lightId, double value)
    {
        emit cmdUpdateLightValue(lightId, value);
    }

    void CamerasTestViewModel::uiEmergencyStop()
    {
        emit cmdEmergencyStop();
    }

} // namespace Kub3::Tools::Tester
