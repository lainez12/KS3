#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels
{

    BaseViewModel::~BaseViewModel()
    {
        unloadConnections();
    }

    void BaseViewModel::loadConnections(void)
    {
        if (!m_activeConnections.empty())
            return;

        for (const auto &connBuilder : m_connectionBuilders)
            m_activeConnections.push_back(connBuilder());
    }

    void BaseViewModel::unloadConnections(void)
    {
        if (m_activeConnections.empty())
            return;

        for (const auto &conn : m_activeConnections)
            QObject::disconnect(conn);
        m_activeConnections.clear();
    }

}
