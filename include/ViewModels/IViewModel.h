#pragma once

namespace Kub3::UI::ViewModels
{

    class IViewModel
    {
    public:
        virtual ~IViewModel() = default;

        virtual void loadConnections(void)   = 0;
        virtual void unloadConnections(void) = 0;
    };

} // Kub3::UI::ViewModels
