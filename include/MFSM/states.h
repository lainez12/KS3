#pragma once

#include <string>
#include <variant>

#include <Services/Drawers/IDrawerService.h>
#include <Services/Exposure/IExposureService.h>
#include <Services/Homing/IHomingService.h>
#include <Services/Stowage/IStowageService.h>
#include <utils.h>

// Forward declarations
namespace Kub3::MFSM
{
    enum class DrawerOperation;
}

// ===============================================
// Master Finite State Machine STATES Definitions
// ===============================================

namespace Kub3::MFSM
{

    namespace Payloads
    {
        // ---------------
        // --- PAYLOADS
        // ---------------

        struct HomingOpPayload {
            Services::HomingTarget::Type target;
        };

        struct DrawerOpPayload {
            DrawerOperation kind;          // Insert/Eject
            Services::DrawerTarget target; // Wafer/Mask/Both
        };

        struct StowageOpPayload {};

        // Represents the mechanical phase of the contact process
        enum class ContactPhase
        {
            Free,            // X, Y, Theta OK | Cameras OK | Z Manual OK
            ApplyingContact, // X, Y, Theta LOCKED | Cameras OK | Z Automated ONLY
            InContact,       // X, Y, Theta LOCKED | Cameras OK | Z Manual OK
            Separating       // X, Y, Theta LOCKED | Cameras OK | Z Automated ONLY
        };

        struct AlignmentOpPayload {
            ContactPhase phase = ContactPhase::Free;
            bool isAutoAlignment; // TODO: Implement when creating the automatic alignment algorithm
        };

        struct ExposureOpPayload {
            Services::ExposurePayload payload;
        };

        using OperatingPayload = std::variant<
            HomingOpPayload,
            DrawerOpPayload,
            StowageOpPayload,
            AlignmentOpPayload,
            ExposureOpPayload>;

    } // namespace Payloads

    // ----------------------
    // --- SYSTEM POSTURE
    // ----------------------

    enum class WaferPosture
    {
        Unknown,       // Machine lost track (e.g. after Error, Boot, or E-Stop)
        Ejected,       // Wafer drawer is ejected
        Homed,         // Wafer conveyor is inserted, Z is down
        AlignmentZone, // Z is up at Z2, ready for cameras/contact
    };

    enum class MaskPosture
    {
        Unknown,  // Machine lost track (e.g. after Error, Boot, or E-Stop)
        Ejected,  // Mask drawer is ejected
        Homed,    // Mask conveyor just met the CM2 limit switch
        Exposure, // Mask is vacuum-secured in exposure position
    };

    enum class VisionPosture
    {
        Unknown,       // Machine lost track (e.g. after Error, Boot, or E-Stop)
        Homed,         // Could be named `Exposure`: the position not obstructing led heads
        Visualisation, // Cameras' deck positioned above the mask and alignment stages, cameras in any position
    };

    struct ExpectedSystemPosture {
        Optional<WaferPosture> newWaferPosture   = std::nullopt;
        Optional<MaskPosture> newMaskPosture     = std::nullopt;
        Optional<VisionPosture> newVisionPosture = std::nullopt;
    };

    struct SystemPosture {
        WaferPosture wafer   = WaferPosture::Unknown;
        MaskPosture mask     = MaskPosture::Unknown;
        VisionPosture vision = VisionPosture::Unknown;

        [[nodiscard]] bool isFullyKnown() const
        {
            return wafer != WaferPosture::Unknown &&
                   mask != MaskPosture::Unknown &&
                   vision != VisionPosture::Unknown;
        }

        [[nodiscard]] SystemPosture merge(const ExpectedSystemPosture &s) const
        {
            return SystemPosture{
                .wafer  = s.newWaferPosture.value_or(this->wafer),
                .mask   = s.newMaskPosture.value_or(this->mask),
                .vision = s.newVisionPosture.value_or(this->vision),
            };
        }

        [[nodiscard]] SystemPosture invalidate(const ExpectedSystemPosture &e) const
        {
            return SystemPosture{
                .wafer  = e.newWaferPosture.has_value() ? WaferPosture::Unknown : this->wafer,
                .mask   = e.newMaskPosture.has_value() ? MaskPosture::Unknown : this->mask,
                .vision = e.newVisionPosture.has_value() ? VisionPosture::Unknown : this->vision,
            };
        }
    };

    // -------------
    // --- STATES
    // -------------

    struct StateBooting {
        uint32_t ticksElapsed = 0;
        // Keeps track of how many times each specific subsystem has been retried
        std::unordered_map<std::string, int8_t> retryCounts;
    };

    struct StateWaitingInitialization {};

    struct StateInitialization {};

    struct StateIdle {
        SystemPosture posture;
    };

    struct StateOperating {
        SystemPosture posture;
        Payloads::OperatingPayload payload;
        // On success/error, the posture should be updated or invalidated using these values
        ExpectedSystemPosture expectedSuccess;
    };

    struct StateError {
        SystemPosture posture;
        std::string message;
    };

    struct StateEmergencyStop {
        SystemPosture posture;
        std::string reason;
    };

    struct StatePowerOff {};

    // The Single Source of Truth for the Machine's High-Level State
    using SystemState = std::variant<
        StateBooting,
        StateWaitingInitialization,
        StateInitialization,
        StateIdle,
        StateOperating,
        StateError,
        StateEmergencyStop,
        StatePowerOff>;

} // namespace Kub3::MFSM
