#include <views/FocalTestView.h>
#include <views/components/FocalTestModule.h>

namespace Kub3::Tools::Tester
{

    FocalTestView::FocalTestView(Shared<FocalTestViewModel> vm, QWidget *parent) :
        UI::Views::ViewBase(vm, parent),
        m_vm(std::move(vm))
    {
        auto *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 0);

        auto *scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);

        m_scrollContainer = new QWidget(scrollArea);
        m_modulesLayout   = new QVBoxLayout(m_scrollContainer);
        m_modulesLayout->setAlignment(Qt::AlignTop);

        scrollArea->setWidget(m_scrollContainer);
        mainLayout->addWidget(scrollArea);

        bindViewModel();
    }

    void FocalTestView::bindViewModel()
    {
        if (!m_vm)
            return;

        connect(m_vm.get(), &FocalTestViewModel::s_availableFocalsLoaded, this, &FocalTestView::buildDynamicModules);
    }

    void FocalTestView::buildDynamicModules(const QStringList &focalIds)
    {
        // Clear existing modules if any
        QLayoutItem *child;
        while ((child = m_modulesLayout->takeAt(0)) != nullptr)
        {
            delete child->widget();
            delete child;
        }

        // Instantiate a module for each focal
        for (const QString &focalId : focalIds)
        {
            auto *moduleWidget = new FocalTestModule(focalId, m_scrollContainer);

            // Wire Module UI intents -> View Model
            connect(moduleWidget, &FocalTestModule::s_valueChanged, m_vm.get(), &FocalTestViewModel::uiUpdateFocalValue);
            connect(moduleWidget, &FocalTestModule::s_toggled, m_vm.get(), &FocalTestViewModel::uiToggleFocal);

            m_modulesLayout->addWidget(moduleWidget);
        }
    }

} // namespace Kub3::Tools::Tester
