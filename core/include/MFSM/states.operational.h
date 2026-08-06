#pragma once

#include <qstringliteral.h>
#include <type_traits>
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
        ExpectedSystemPosture expectedAbort;
    };

    struct StateStowing {
        StowageTarget target;
        ExpectedSystemPosture expectedSuccess;
        ExpectedSystemPosture expectedAbort;
    };

    struct StateUnstowing {
        StowageTarget target;
        ExpectedSystemPosture expectedSuccess;
        ExpectedSystemPosture expectedAbort;
    };

    struct StateAutoleveling {
        ExpectedSystemPosture expectedSuccess;
        ExpectedSystemPosture expectedAbort;
    };

    struct StateRetractingZ {
        ExpectedSystemPosture expectedSuccess;
        ExpectedSystemPosture expectedAbort;
    };

    struct StateAlignment {
        ContactPhase phase;
        bool isAutoAlignment = false; // TODO: Implement when creating the automatic alignment algorithm
    };

    struct StatePreparingAlignment {
        StateAlignment alignment;
        ExpectedSystemPosture expectedSuccess;
        ExpectedSystemPosture expectedAbort;
    };

    struct StateExposureReady {
        ContactPhase savedContactPhase;
    };

    struct StatePreparingExposure {
        ContactPhase savedContactPhase;
        ExpectedSystemPosture expectedSuccess;
        ExpectedSystemPosture expectedAbort;
    };

    struct StateExposing {
        ContactPhase savedContactPhase;
        Services::ExposurePayload payload;
    };

    enum class OperationalStateKind
    {
        Idle,
        DrawerOp,
        Stowing,
        Unstowing,
        Autoleveling,
        RetractingZ,
        PreparingAlignment,
        Alignment,
        PreparingExposure,
        ExposureReady,
        Exposing
    };

    using OperationalState = std::variant<
        StateIdle,
        StateDrawerOp,
        StateStowing,
        StateUnstowing,
        StateAutoleveling,
        StateRetractingZ,
        StatePreparingAlignment,
        StateAlignment,
        StatePreparingExposure,
        StateExposureReady,
        StateExposing>;

    inline OperationalStateKind kindOf(const OperationalState &state)
    {
        return std::visit(
            [](const auto &s) -> OperationalStateKind {
                using T = std::decay_t<decltype(s)>;

                if constexpr (std::is_same_v<T, StateIdle>)
                    return OperationalStateKind::Idle;
                else if constexpr (std::is_same_v<T, StateDrawerOp>)
                    return OperationalStateKind::DrawerOp;
                else if constexpr (std::is_same_v<T, StateStowing>)
                    return OperationalStateKind::Stowing;
                else if constexpr (std::is_same_v<T, StateAutoleveling>)
                    return OperationalStateKind::Autoleveling;
                else if constexpr (std::is_same_v<T, StateRetractingZ>)
                    return OperationalStateKind::RetractingZ;
                else if constexpr (std::is_same_v<T, StateUnstowing>)
                    return OperationalStateKind::Unstowing;
                else if constexpr (std::is_same_v<T, StatePreparingAlignment>)
                    return OperationalStateKind::PreparingAlignment;
                else if constexpr (std::is_same_v<T, StateAlignment>)
                    return OperationalStateKind::Alignment;
                else if constexpr (std::is_same_v<T, StatePreparingExposure>)
                    return OperationalStateKind::PreparingExposure;
                else if constexpr (std::is_same_v<T, StateExposureReady>)
                    return OperationalStateKind::ExposureReady;
                else
                    return OperationalStateKind::Exposing;
            },
            state);
    }

    inline QString toString(const OperationalState &state)
    {
        return std::visit(
            [](const auto &s) -> QString {
                using T = std::decay_t<decltype(s)>;

                if constexpr (std::is_same_v<T, StateIdle>)
                    return QStringLiteral("Idle");
                else if constexpr (std::is_same_v<T, StateDrawerOp>)
                    return QStringLiteral("DrawerOp");
                else if constexpr (std::is_same_v<T, StateStowing>)
                    return QStringLiteral("Stowing");
                else if constexpr (std::is_same_v<T, StateAutoleveling>)
                    return QStringLiteral("Autoleveling");
                else if constexpr (std::is_same_v<T, StateRetractingZ>)
                    return QStringLiteral("RetractingZ");
                else if constexpr (std::is_same_v<T, StateUnstowing>)
                    return QStringLiteral("Unstowing");
                else if constexpr (std::is_same_v<T, StatePreparingAlignment>)
                    return QStringLiteral("PreparingAlignment");
                else if constexpr (std::is_same_v<T, StateAlignment>)
                    return QStringLiteral("Alignment");
                else if constexpr (std::is_same_v<T, StatePreparingExposure>)
                    return QStringLiteral("PreparingExposure");
                else if constexpr (std::is_same_v<T, StateExposureReady>)
                    return QStringLiteral("ExposureReady");
                else
                    return QStringLiteral("Exposing");
            },
            state);
    }

} // namespace Kub3::MFSM

Q_DECLARE_METATYPE(Kub3::MFSM::OperationalState)
Q_DECLARE_METATYPE(Kub3::MFSM::OperationalStateKind)
