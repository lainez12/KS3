#include <view_models/FocalTestViewModel.h>

namespace Kub3::Tools::Tester
{

    FocalTestViewModel::FocalTestViewModel(QObject *parent) : QObject(parent) {}

    void FocalTestViewModel::setAvailableFocals(const QList<QString> &focals)
    {
        m_focalCatalog = focals;
        emit s_availableFocalsLoaded(m_focalCatalog);
    }

    void FocalTestViewModel::uiToggleFocal(const QString &focalId, bool en)
    {
        emit cmdToggleFocal(focalId, en);
    }

    void FocalTestViewModel::uiUpdateFocalValue(const QString &focalId, uint16_t value)
    {
        emit cmdUpdateFocalValue(focalId, value);
    }

    void FocalTestViewModel::uiEmergencyStop()
    {
        emit cmdEmergencyStop();
    }

} // namespace Kub3::Tools::Tester
