#pragma once

#include <variant>

#include <Common/Enums.h>
#include <MFSM/events.h>
#include <MFSM/posture.h>
#include <Services/Drawers/IDrawerService.h>
#include <Services/Exposure/IExposureService.h>
#include <Services/Stowage/IStowageService.h>

namespace Kub3::MFSM
{

    // Represents the mechanical phase of the contact process
    enum class ContactPhase
    {
        Free,            // X, Y, Theta OK | Cameras OK | Z Manual OK
        ApplyingContact, // X, Y, Theta LOCKED | Cameras OK | Z Automated ONLY
        InContact,       // X, Y, Theta LOCKED | Cameras OK | Z Manual OK
        Separating       // X, Y, Theta LOCKED | Cameras OK | Z Automated ONLY
    };

    struct StateIdle {};

    struct StateDrawerOp {
        DrawerOperation kind;
        DrawerTarget target;
        ExpectedSystemPosture expectedSuccess;
    };

    struct StateStowing {
        Services::StowageTarget target;
        ExpectedSystemPosture expectedSuccess;
    };

    struct StateUnstowing {
        Services::StowageTarget target;
        ExpectedSystemPosture expectedSuccess;
    };

    struct StateAlignment {
        ContactPhase phase;
        bool isAutoAlignment = false; // TODO: Implement when creating the automatic alignment algorithm
    };

    struct StatePreparingAlignment {
        StateAlignment alignment;
        ExpectedSystemPosture expectedSuccess;
    };

    struct StateExposureReady {
        ContactPhase savedContactPhase;
    };

    struct StatePreparingExposure {
        ContactPhase savedContactPhase;
        ExpectedSystemPosture expectedSuccess;
    };

    struct StateExposing {
        ContactPhase savedContactPhase;
        Services::ExposurePayload payload;
    };

    using OperationalState = std::variant<
        StateIdle,
        StateDrawerOp,
        StateStowing,
        StateUnstowing,
        StatePreparingAlignment,
        StateAlignment,
        StatePreparingExposure,
        StateExposureReady,
        StateExposing>;

} // namespace Kub3::MFSM
