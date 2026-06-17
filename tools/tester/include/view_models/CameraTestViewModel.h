#pragma once

#include <QList>
#include <QObject>
#include <QString>

#include <ViewModels/BaseViewModel.h>

namespace Kub3::Tools::Tester
{

    class CameraTestViewModel : public QObject, public UI::ViewModels::BaseViewModel
    {
        Q_OBJECT

    public:
        explicit CameraTestViewModel(QObject *parent = nullptr);
        ~CameraTestViewModel() override = default;

        void setAvailableFocals(const QList<QString> &focals);
        void setAvailableLights(const QList<QString> &focals);

    public slots:
        void uiToggleFocal(const QString &focalId, bool en);
        void uiUpdateFocalValue(const QString &focalId, uint16_t value);

        void uiToggleLight(const QString &lightId, bool enabled);
        void uiUpdateLightValue(const QString &lightId, uint16_t value);

        void uiEmergencyStop();

    signals:
        void s_availableFocalsLoaded(const QStringList &focalIds);
        void s_availableLightsLoaded(const QStringList &lightIds);

        // Outbound commands to Controller
        void cmdToggleFocal(const QString &focalId, bool en);
        void cmdUpdateFocalValue(const QString &focalId, uint16_t value);
        void cmdToggleLight(const QString &lightId, bool en);
        void cmdUpdateLightValue(const QString &lightId, uint16_t value);
        void cmdEmergencyStop();

    private:
        QList<QString> m_focalCatalog; // <FocalID>
        QList<QString> m_lightCatalog; // <LightID>
    };

} // namespace Kub3::Tools::Tester