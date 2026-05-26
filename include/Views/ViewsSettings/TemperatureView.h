#ifndef TEMPERATUREVIEW_H
#define TEMPERATUREVIEW_H

#include <QWidget>
#include <ViewModels/ViewModelsSettings/TemperatureViewModel.h>

#include "Views/ViewBase.h"

namespace Ui {
    class TemperatureView;
} // namespace UI

namespace Kub3::UI::Views::ViewsSettings {
    class TemperatureView final : public ViewBase {
        using TemperatureViewModel = Kub3::UI::ViewModels::ViewsModelSettings::TemperatureViewModel;

        Q_OBJECT

    public:
        explicit TemperatureView(Unique<TemperatureViewModel> viewModel, QWidget *parent = nullptr);
        ~TemperatureView();

    signals:

    public slots:

    private slots:

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void createNavButtonsConfigs();
        void configTitleBar();
        void onHomeButtonClicked(const QString &buttonId);
        void onBackButtonClicked(const QString &buttonId);

    private:
        Ui::TemperatureView *ui;

        bool m_isFlashingMode = false;
    };

} // namespace Kub3::UI::Views

using TemperatureView = Kub3::UI::Views::ViewsSettings::TemperatureView;

#endif