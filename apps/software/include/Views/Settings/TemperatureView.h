#ifndef TEMPERATUREVIEW_H
#define TEMPERATUREVIEW_H

#include <QWidget>
#include <ViewModels/Settings/TemperatureViewModel.h>
#include <Views/SettingsViewBase.h>

#include <Views/ViewBase.h>

namespace Ui
{
    class TemperatureView;
} // namespace UI

namespace Kub3::UI::Views::ViewsSettings
{
    class TemperatureView final : public SettingsViewBase
    {
        using TemperatureViewModel = Kub3::UI::ViewModels::Settings::TemperatureViewModel;

        Q_OBJECT

    public:
        explicit TemperatureView(Unique<TemperatureViewModel> viewModel, QWidget *parent = nullptr);
        ~TemperatureView();

    public:
        void resizeEvent(QResizeEvent *event) override;

    private:
        Ui::TemperatureView *ui;
    };

} // namespace Kub3::UI::Views

using TemperatureView = Kub3::UI::Views::ViewsSettings::TemperatureView;

#endif