#ifndef MACHINE_STATUS_VIEW_H
#define MACHINE_STATUS_VIEW_H

#include <QLabel>
#include <QWidget>
#include <unordered_map>
#include <utils.h>

#include <ViewModels/ViewModelsSettings/MachineStatusViewModel.h>

#include <Views/ViewBase.h>

namespace Ui {
    class MachineStatusView;
}

namespace Kub3::UI::Views {

    class MachineStatusView final : public ViewBase {
        using MachineStatusViewModel = Kub3::UI::ViewModels::MachineStatusViewModel;

        Q_OBJECT

    public:
        explicit MachineStatusView(Unique<MachineStatusViewModel> viewModel, QWidget *parent = nullptr);
        ~MachineStatusView();

    signals:
        void s_home(void);

    public slots:
        void ps_booleanSensorUpdate(const char *sensorId, bool value);

    private slots:
        void on_goBackBtn_clicked(void);

    private:
        void updateBoolSensorsText(QLabel *label, const bool state);
        void createNavButtonsConfigs();
        void configTitleBar();

    private:
        Ui::MachineStatusView *ui;
        std::unordered_map<const char *, QLabel *> m_boolSensorsMap;
    };

}

using MachineStatusView = Kub3::UI::Views::MachineStatusView;

#endif // MACHINE_STATUS_VIEW_H
