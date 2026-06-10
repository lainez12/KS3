#pragma once

#include <QLabel>
#include <memory>
#include <unordered_map>

#include <utils.h>
#include <view_models/ProcedureTestViewModel.h>
#include <views/ViewBase.h>

namespace Ui
{
    class ProcedureTestView;
}

namespace Kub3::Tools::Tester
{

    class ProcedureTestView final : public UI::Views::ViewBase
    {
        Q_OBJECT

    public:
        explicit ProcedureTestView(Shared<ProcedureTestViewModel> viewModel, QWidget *parent = nullptr);
        ~ProcedureTestView() override;

    private slots:
        void ps_booleanSensorUpdate(const QString &sensorId, bool value);
        void ps_integerSensorUpdate(const QString &sensorId, int32_t value);

    private:
        void bindViewModel();
        void updateBoolSensorsText(QLabel *label, const bool state);
        void updateIntSensorsText(QLabel *label, const int32_t state);

    private:
        Unique<Ui::ProcedureTestView> ui;
        Shared<ProcedureTestViewModel> m_procedureViewModel; // Typed convenience ptr

        // Sensor UI maps
        std::unordered_map<QString, QLabel *> m_boolSensorsMap;
        std::unordered_map<QString, QLabel *> m_intSensorsMap;
    };

} // namespace Kub3::Tools::Tester
