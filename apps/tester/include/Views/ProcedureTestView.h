#pragma once

#include <QLabel>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>

#include <Common/Enums.h>
#include <Config/hardware.h>
#include <ViewModels/ProcedureTestViewModel.h>
#include <Views/Traits/PadReceiverViewTrait.h>
#include <Views/ViewBase.h>
#include <utils.h>

namespace Ui
{
    class ProcedureTestView;
}

namespace Kub3::Tools::Tester
{

    /**
     * @brief Main view responsible for displaying and controlling the hardware procedure test UI.
     *
     * Binds tightly to the `ProcedureTestViewModel` and captures keyboard events to
     * actuate hardware cameras manually.
     */
    class ProcedureTestView final : public UI::Views::ViewBase, public UI::Views::PadReceiverViewTrait
    {
        Q_OBJECT

    public:
        /**
         * @brief Constructs the ProcedureTestView.
         * @param viewModel Shared pointer to the backing view model.
         * @param parent The parent widget.
         */
        explicit ProcedureTestView(Shared<ProcedureTestViewModel> viewModel, const Config::hardware_config_t &hwConf, QWidget *parent = nullptr);
        ~ProcedureTestView() override;

        void loadConfigValues(const Config::hardware_config_t &conf);

    protected:
        /**
         * @brief Ensures the view claims focus when displayed to capture keyboard events immediately.
         */
        void showEvent(QShowEvent *event) override;

    private slots:
        void booleanSensorUpdate(const QString &sensorId, bool value);
        void integerSensorUpdate(const QString &sensorId, int32_t value);
        void uint16SensorUpdate(const QString &sensorId, uint16_t value);

    private:
        // Initialization helpers to keep the constructor clean
        void initializeSensorMaps();
        void setupButtonBindings();
        void setupViewModelBindings();
        void setupRealtimeCurves();
        void setupPadInputsCallbacks();

        // Command helpers
        void cameraMovement(CameraId camId, MovementKind kind, CameraDirection dir);
        void alignmentStageMovement(AlignmentStageId, MovementKind kind, AlignmentStageDirection dir);

        // UI Updaters
        void updateBoolSensorsText(QLabel *label, const bool state);
        void updateLabelText(QLabel *label, const QString &text);

    private:
        typedef struct force_sensor_conf_s {
            Optional<double> adcToGFRatio;
            QString tareId;
        } force_sensor_conf_t;

        Unique<Ui::ProcedureTestView> ui;
        Shared<ProcedureTestViewModel> m_procedureViewModel;

        // Configuration values
        std::unordered_map<QString, force_sensor_conf_t> m_forceSensorIdToConfMap = {};
        std::unordered_map<QString, Config::motor_config_t> m_motorsHwConf        = {};

        std::unordered_map<QString, QLabel *> m_boolSensorsMap;
        std::unordered_map<QString, QLabel *> m_intSensorsMap;
        std::unordered_map<QString, QLabel *> m_uint16SensorsMap;
    };

} // namespace Kub3::Tools::Tester
