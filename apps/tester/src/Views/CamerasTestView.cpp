#include "ui_CamerasTestView.h"

#include <Views/CamerasTestView.h>
#include <Views/Components/CameraTestModule.h>

namespace Kub3::Tools::Tester
{
    CamerasTestView::CamerasTestView(Shared<CamerasTestViewModel> vm, QWidget *parent) :
        UI::Views::ViewBase(vm, parent),
        ui(new Ui::CamerasTestView),
        m_vm(std::move(vm)),
        m_cameraCount(0)
    {
        ui->setupUi(this);
        bindViewModel();
    }

    CamerasTestView::~CamerasTestView()
    {
        delete ui;
    }

    void CamerasTestView::bindViewModel()
    {
        if (!m_vm)
            return;
        connect(m_vm.get(), &CamerasTestViewModel::s_cameraModulesLoaded, this, &CamerasTestView::buildCameraModules);
    }

    void CamerasTestView::buildCameraModules(const QList<CameraModuleConfig> &configs)
    {
        for (const auto &config : configs)
        {
            addCameraModule(config);
        }
    }

    void CamerasTestView::addCameraModule(const CameraModuleConfig &config)
    {
        if (!m_vm)
            return;

        auto *module = new CameraTestModule(config.cameraId, config.cameraId, config.focalId, config.lightId, this);

        // 1. Wire Control signals -> View Model
        connect(module, &CameraTestModule::s_exposureChanged, m_vm.get(), &CamerasTestViewModel::s_exposureSliderValueChanged);
        connect(module, &CameraTestModule::s_gainChanged, m_vm.get(), &CamerasTestViewModel::s_gainSliderValueChanged);
        connect(module, &CameraTestModule::s_framerateChanged, m_vm.get(), &CamerasTestViewModel::s_framerateValueChanged);
        connect(module, &CameraTestModule::s_zoomChanged, m_vm.get(), &CamerasTestViewModel::s_centeredZoomValueChanged);
        connect(module, &CameraTestModule::s_roiChanged, m_vm.get(), &CamerasTestViewModel::s_roiChanged);
        // --- Focals and lighting leds
        connect(module, &CameraTestModule::s_focalToggled, m_vm.get(), &CamerasTestViewModel::uiToggleFocal);
        connect(module, &CameraTestModule::s_focalChanged, m_vm.get(), &CamerasTestViewModel::uiUpdateFocalValue);
        connect(module, &CameraTestModule::s_lightToggled, m_vm.get(), &CamerasTestViewModel::uiToggleLight);
        connect(module, &CameraTestModule::s_lightChanged, m_vm.get(), &CamerasTestViewModel::uiUpdateLightValue);

        // 2. Filter generic Frame Ready signal for THIS specific module
        connect(m_vm.get(), &CamerasTestViewModel::s_frameReady, module,
                [targetId = config.cameraId, w = module->streamWidget()](const QString &incomingId, const QImage &frame) {
                    if (incomingId == targetId)
                    {
                        w->ps_onFrameUpdated(frame);
                    }
                });

        // 3. Grid placement (2 columns)
        int row = m_cameraCount / 2;
        int col = m_cameraCount % 2;
        ui->camerasLayout->addWidget(module, row, col);

        m_cameraCount++;
    }

} // namespace Kub3::Tools::Tester
