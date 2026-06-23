#ifndef VISUALISATIONVIEW_H
#define VISUALISATIONVIEW_H

#include <QLabel>
#include <QWidget>
#include <ViewModels/Alignment/VisualisationViewModel.h>
#include <Views/AlignmentViewBase.h>

#include <Views/Components/HardForceContactForm.h>
#include <Views/Components/NavButton.h>
#include <Views/Components/RealPositionCameras.h>
#include <Views/KeyboardConnections.h>
#include <Views/ViewBase.h>

namespace Ui
{
    class VisualisationView;
} // namespace UI

namespace Kub3::UI::Views::ViewsAlignment
{
    class VisualisationView final : public AlignmentViewBase
    {
        using VisualisationViewModel = Kub3::UI::ViewModels::Alignment::VisualisationViewModel;

        Q_OBJECT

    public:
        explicit VisualisationView(Unique<VisualisationViewModel> viewModel, QWidget *parent = nullptr);
        ~VisualisationView();

    public:
        void resizeEvent(QResizeEvent *event) override;

    public slots:
        void mapPositionCamerasOpenMap(void);
        void mapPositionCamerasCloseMap(void);

    private slots:
        void leftCamConfigToggled(bool checked);
        void rightCamConfigToggled(bool checked);
        void navButtonToggled(NavButton *button, QWidget *widget);

    private:
        void setNewNavButtonsConfigs();
        void onValidateButtonClicked(const QString &buttonId) override;
        void onBackButtonClicked(const QString &buttonId) override;
        void onSaveButtonClicked(const QString &buttonId);
        void onLoadButtonClicked(const QString &buttonId);
        void onScreenshotButtonClicked(const QString &buttonId);
        void onHardForceContButtonClicked(const QString &buttonId);
        void onSpeedMotorSubstButtonClicked(const QString &buttonId);
        void onMaskingDistanceButtonClicked(const QString &buttonId);
        void onAntiCollisionButtonClicked(const QString &buttonId);
        void onVisualMarkButtonClicked(const QString &buttonId);
        void onMeasurementButtonClicked(const QString &buttonId);

    private:
        Ui::VisualisationView *ui;
        KeyboardConnections m_keyboard;
        QWidget *m_maskingDistanceWidget;
        RealPositionCameras *m_mapPositionCameras;
        HardForceContactForm *m_hardForceContactForm;
        QLabel *m_labelText;
    };

} // namespace Kub3::UI::Views

using VisualisationView = Kub3::UI::Views::ViewsAlignment::VisualisationView;

#endif