#pragma once

#include <QList>
#include <QObject>
#include <QString>

#include <ViewModels/BaseViewModel.h>

namespace Kub3::Tools::Tester
{

    class FocalTestViewModel : public QObject, public UI::ViewModels::BaseViewModel
    {
        Q_OBJECT

    public:
        explicit FocalTestViewModel(QObject *parent = nullptr);
        ~FocalTestViewModel() override = default;

        void setAvailableFocals(const QList<QString> &focals);

    public slots:
        void uiToggleFocal(const QString &focalId, bool en);
        void uiUpdateFocalValue(const QString &focalId, uint16_t value);
        void uiEmergencyStop();

    signals:
        void s_availableFocalsLoaded(const QStringList &focalIds);

        // Outbound commands to Controller
        void cmdToggleFocal(const QString &focalId, bool en);
        void cmdUpdateFocalValue(const QString &focalId, uint16_t value);
        void cmdEmergencyStop();

    private:
        QList<QString> m_focalCatalog; // <FocalID>
    };

} // namespace Kub3::Tools::Tester