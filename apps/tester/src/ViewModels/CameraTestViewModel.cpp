#include <ViewModels/CameraTestViewModel.h>

namespace Kub3::Tools::Tester
{

    CameraTestViewModel::CameraTestViewModel(QObject *parent) : QObject(parent) {}

    void CameraTestViewModel::setAvailableFocals(const QList<QString> &focals)
    {
        m_focalCatalog = focals;
        emit s_availableFocalsLoaded(m_focalCatalog);
    }

    void CameraTestViewModel::setAvailableLights(const QList<QString> &lights)
    {
        m_lightCatalog = lights;
        emit s_availableLightsLoaded(m_lightCatalog);
    }

    void CameraTestViewModel::uiToggleFocal(const QString &focalId, bool en)
    {
        emit cmdToggleFocal(focalId, en);
    }

    void CameraTestViewModel::uiUpdateFocalValue(const QString &focalId, uint16_t value)
    {
        emit cmdUpdateFocalValue(focalId, value);
    }

    void CameraTestViewModel::uiToggleLight(const QString &lightId, bool en)
    {
        emit cmdToggleLight(lightId, en);
    }

    void CameraTestViewModel::uiUpdateLightValue(const QString &lightId, uint16_t value)
    {
        emit cmdUpdateLightValue(lightId, value);
    }

    void CameraTestViewModel::uiEmergencyStop()
    {
        emit cmdEmergencyStop();
    }

} // namespace Kub3::Tools::Tester
