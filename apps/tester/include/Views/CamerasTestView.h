#pragma once

#include <QGridLayout>
#include <QVBoxLayout>

#include <ViewModels/CamerasTestViewModel.h>
#include <Views/ViewBase.h>
#include <utils.h>

namespace Ui
{
    class CamerasTestView;
}

namespace Kub3::Tools::Tester
{
    class CamerasTestView final : public UI::Views::ViewBase
    {
        Q_OBJECT
    public:
        explicit CamerasTestView(Shared<CamerasTestViewModel> vm, QWidget *parent = nullptr);
        ~CamerasTestView() override;

    private slots:
        void buildCameraModules(const QList<CameraModuleConfig> &configs);

    private:
        void bindViewModel();
        void addCameraModule(const CameraModuleConfig &config);

    private:
        Ui::CamerasTestView *ui;
        Shared<CamerasTestViewModel> m_vm;
        int m_cameraCount = 0;
    };

} // namespace Kub3::Tools::Tester
