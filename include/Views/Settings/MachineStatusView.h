#ifndef MACHINE_STATUS_VIEW_H
#define MACHINE_STATUS_VIEW_H

#include <QLabel>
#include <QWidget>
#include <unordered_map>
#include <utils.h>

#include <ViewModels/Settings/MachineStatusViewModel.h>

#include <Views/ViewBase.h>

namespace Ui
{
    class MachineStatusView;
}

namespace Kub3::UI::Views
{

    class MachineStatusView final : public ViewBase
    {
        using MachineStatusViewModel = Kub3::UI::ViewModels::MachineStatusViewModel;

        Q_OBJECT

    public:
        explicit MachineStatusView(Unique<MachineStatusViewModel> viewModel, QWidget *parent = nullptr);
        ~MachineStatusView() override;

    signals:
        void s_home(void);

    public slots:
        void ps_booleanSensorUpdate(const QString &sensorId, bool value);
        void ps_integerSensorUpdate(const QString &sensorId, int32_t value);
        void ps_unsignedIntegerSensorUpdate(const QString &sensorId, uint32_t value);

    private slots:
        void on_goBackBtn_clicked(void);
        void on_openCamerasBtn_clicked(void);

    private:
        void populateBoolSensorsMap(void);
        void populateIntegerSensorsMap(void);
        void populateUnsignedIntegerSensorsMap(void);

        void updateBoolSensorsText(QLabel *label, const bool state);
        void updateIntSensorsText(QLabel *label, const int32_t state);
        void updateUIntSensorsText(QLabel *label, const uint32_t state);

        void createNavButtonsConfigs();
        void configTitleBar();

    private:
        Ui::MachineStatusView *ui;
        std::unordered_map<QString, QLabel *> m_boolSensorsMap;
        std::unordered_map<QString, QLabel *> m_intSensorsMap;
        std::unordered_map<QString, QLabel *> m_uintSensorsMap;
    };

}

using MachineStatusView = Kub3::UI::Views::MachineStatusView;

#endif // MACHINE_STATUS_VIEW_H
