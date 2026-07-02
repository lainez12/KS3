#pragma once

#include <QObject>
#include <QString>

#include <Common/Enums.h>
#include <ViewModels/BaseViewModel.h>
#include <utils.h>

namespace Kub3::HAL::MS
{
    class IMachineStatusRepo;
}

namespace Kub3::Tools::Tester
{

    class ProcedureTestViewModel : public QObject, public UI::ViewModels::BaseViewModel
    {
        Q_OBJECT

    public:
        explicit ProcedureTestViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~ProcedureTestViewModel() override = default;

        [[nodiscard]] bool isRunning() const noexcept
        {
            return m_isRunning;
        }
        [[nodiscard]] bool hasError() const noexcept
        {
            return m_hasError;
        }
        [[nodiscard]] QString currentProcedure() const noexcept
        {
            return m_currentProcedure;
        }
        [[nodiscard]] QString lastStatusMessage() const noexcept
        {
            return m_lastStatusMessage;
        }

    public slots:
        // UI Action Inputs (Intents)
        void uiEmergencyStop();
        void uiRequestHoming(int targetBits);
        void uiRequestDrawerOperation(DrawerTarget target, bool eject);
        void uiRequestStowage(int targetInt);
        void uiRequestUnstowage(int targetInt);
        void uiRequestInitialization(void);
        void uiRequestInitStages(void);
        void uiRequestCenterStages(void);
        void uiRequestInitCameras(void);
        void uiRequestInitDeck(void);
        void uiRequestInitVision(void);
        void uiRequestInitZAxes(void);
        void uiRequestHomeZAxes(void);
        void uiRequestAutolevel(void);
        void uiRequestCameraMovement(CameraId camId, CameraMovementKind kind, CameraDirection dir);

        // Inbound Service Updates (from Logic Tier)
        void onProcedureStarted(const QString &procedureName);
        void onProcedureCompleted(const QString &procedureName);
        void onProcedureFailed(const QString &procedureName, const QString &reason);
        void onHandleSensorValueChanged(const std::string &key);

    signals:
        // UI Property Notifiers
        void s_isRunningChanged();
        void s_statusMessageChanged();
        void s_booleanSensorUpdate(const QString &key, bool val);
        void s_integerSensorUpdate(const QString &key, int32_t val);

        // Outbound Commands to Logic Thread Controller
        void cmdEmergencyStop();
        void cmdRunHoming(int targetBits);
        void cmdRunDrawerOperation(DrawerTarget target, bool eject);
        void cmdRunStowage(int targetInt);
        void cmdRunUnstowage(int targetInt);
        void cmdRunInitialization(void);
        void cmdRunInitStages(void);
        void cmdRunCenterStages(void);
        void cmdRunInitCameras(void);
        void cmdRunInitDeck(void);
        void cmdRunInitVision(void);
        void cmdRunInitZAxes(void);
        void cmdRunHomeZAxes(void);
        void cmdRunAutolevel(void);
        void cmdRunCameraMovement(CameraId camId, CameraMovementKind kind, CameraDirection dir);

    private:
        void setStatus(const QString &msg, bool isError);

        Shared<HAL::MS::IMachineStatusRepo> m_repo;

        bool m_isRunning = false;
        bool m_hasError  = false;
        QString m_currentProcedure;
        QString m_lastStatusMessage;
    };

} // namespace Kub3::Tools::Tester
