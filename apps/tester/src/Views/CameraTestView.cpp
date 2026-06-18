#include <Views/CameraTestView.h>
#include <Views/Components/FocalTestModule.h>

namespace Kub3::Tools::Tester
{

    CameraTestView::CameraTestView(Shared<CameraTestViewModel> vm, QWidget *parent) :
        UI::Views::ViewBase(vm, parent),
        m_vm(std::move(vm))
    {
        auto *baseLayout = new QVBoxLayout(this);
        baseLayout->setContentsMargins(0, 0, 0, 0);

        auto *scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);

        m_scrollContainer = new QWidget(scrollArea);
        m_mainLayout      = new QVBoxLayout(m_scrollContainer);
        m_mainLayout->setAlignment(Qt::AlignTop);

        // --- 1. Camera Focals Section ---
        m_focalsGroup  = new QGroupBox("Camera Focals", m_scrollContainer);
        m_focalsLayout = new QVBoxLayout(m_focalsGroup);
        m_mainLayout->addWidget(m_focalsGroup);

        // --- 2. Camera Lights Section ---
        m_lightsGroup  = new QGroupBox("Camera Lighting LEDs", m_scrollContainer);
        m_lightsLayout = new QVBoxLayout(m_lightsGroup);
        m_mainLayout->addWidget(m_lightsGroup);

        scrollArea->setWidget(m_scrollContainer);
        baseLayout->addWidget(scrollArea);

        bindViewModel();
    }

    void CameraTestView::bindViewModel()
    {
        if (!m_vm)
            return;

        // Focals wiring
        connect(m_vm.get(), &CameraTestViewModel::s_availableFocalsLoaded, this, &CameraTestView::buildFocalModules);

        // Lights wiring
        connect(m_vm.get(), &CameraTestViewModel::s_availableLightsLoaded, this, &CameraTestView::buildLightModules);
    }

    void CameraTestView::buildFocalModules(const QStringList &focalIds)
    {
        // Clear existing modules if any
        QLayoutItem *child;
        while ((child = m_focalsLayout->takeAt(0)) != nullptr)
        {
            if (auto *widget = child->widget())
                widget->deleteLater();
            delete child;
        }

        // Instantiate a module for each focal
        for (const QString &focalId : focalIds)
        {
            auto *moduleWidget = new FocalTestModule(focalId, m_focalsGroup);

            // Wire Module UI intents -> View Model
            connect(moduleWidget, &FocalTestModule::s_valueChanged, m_vm.get(), &CameraTestViewModel::uiUpdateFocalValue);
            connect(moduleWidget, &FocalTestModule::s_toggled, m_vm.get(), &CameraTestViewModel::uiToggleFocal);

            m_focalsLayout->addWidget(moduleWidget);
        }
    }

    void CameraTestView::buildLightModules(const QStringList &lightIds)
    {
        // Clear existing modules if any
        QLayoutItem *child;
        while ((child = m_lightsLayout->takeAt(0)) != nullptr)
        {
            if (auto *widget = child->widget())
                widget->deleteLater();
            delete child;
        }

        // Instantiate a module for each lighting LED
        for (const QString &lightId : lightIds)
        {
            // Reusing FocalTestModule since it is basically a generic Analog Actuator UI (Slider + Enable Checkbox)
            auto *moduleWidget = new FocalTestModule(lightId, m_lightsGroup);

            // Wire Module UI intents -> View Model Light-specific handlers
            connect(moduleWidget, &FocalTestModule::s_valueChanged, m_vm.get(), &CameraTestViewModel::uiUpdateLightValue);
            connect(moduleWidget, &FocalTestModule::s_toggled, m_vm.get(), &CameraTestViewModel::uiToggleLight);

            m_lightsLayout->addWidget(moduleWidget);
        }
    }

} // namespace Kub3::Tools::Tester
