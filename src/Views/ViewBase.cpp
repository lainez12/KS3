#include <Views/ViewBase.h>

namespace Kub3::UI::Views
{

    ViewBase::ViewBase(Shared<ViewModels::IViewModel> viewModel, QWidget *parent) :
        QWidget(parent),
        m_viewModel(std::move(viewModel))
    {}

    void ViewBase::showEvent(QShowEvent *event)
    {
        if (m_viewModel)
        {
            m_viewModel->loadConnections();
        }
        QWidget::showEvent(event);
    }

    void ViewBase::hideEvent(QHideEvent *event)
    {
        QWidget::hideEvent(event);
        if (m_viewModel)
        {
            m_viewModel->unloadConnections();
        }
    }

} // namespace Kub3::UI::Views
