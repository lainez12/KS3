#pragma once

#include <QLabel>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>

#include <Common/Enums.h>
#include <ViewModels/ProcedureTestViewModel.h>
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
    class ProcedureTestView final : public UI::Views::ViewBase
    {
        Q_OBJECT

    public:
        /**
         * @brief Constructs the ProcedureTestView.
         * @param viewModel Shared pointer to the backing view model.
         * @param parent The parent widget.
         */
        explicit ProcedureTestView(Shared<ProcedureTestViewModel> viewModel, QWidget *parent = nullptr);
        ~ProcedureTestView() override;

    protected:
        /**
         * @brief Ensures the view claims focus when displayed to capture keyboard events immediately.
         */
        void showEvent(QShowEvent *event) override;

    private slots:
        void booleanSensorUpdate(const QString &sensorId, bool value);
        void integerSensorUpdate(const QString &sensorId, int32_t value);

        // Keyboard event handlers
        void handleKeyPressed(Qt::Key keyCode, Qt::KeyboardModifiers modifiers);
        void handleKeyHeld(Qt::Key keyCode, Qt::KeyboardModifiers modifiers);
        void handleKeyReleased(Qt::Key keyCode, Qt::KeyboardModifiers modifiers);

    private:
        // Initialization helpers to keep the constructor clean
        void initializeSensorMaps();
        void setupButtonBindings();
        void setupViewModelBindings();

        // Keyboard mapping and command helpers
        Optional<std::pair<CameraId, CameraDirection>> mapKeyEvtToCameraCmd(Qt::Key keyCode) const;
        void cameraMovement(CameraId camId, CameraMovementKind kind, CameraDirection dir);

        // UI Updaters
        void updateBoolSensorsText(QLabel *label, const bool state);
        void updateIntSensorsText(QLabel *label, const int32_t state);

    private:
        Unique<Ui::ProcedureTestView> ui;
        Shared<ProcedureTestViewModel> m_procedureViewModel;

        std::unordered_map<QString, QLabel *> m_boolSensorsMap;
        std::unordered_map<QString, QLabel *> m_intSensorsMap;
    };

} // namespace Kub3::Tools::Tester
