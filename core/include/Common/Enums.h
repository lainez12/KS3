#pragma once

#include <stdint.h>
#include <type_traits>

#define impl_bin_operators_for_enum(EnumClass)                                                  \
    inline EnumClass operator&(EnumClass lhs, EnumClass rhs)                                    \
    {                                                                                           \
        return static_cast<EnumClass>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)); \
    }                                                                                           \
    inline EnumClass operator|(EnumClass lhs, EnumClass rhs)                                    \
    {                                                                                           \
        return static_cast<EnumClass>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); \
    }

namespace Kub3
{

    template <typename EnumT>
    constexpr bool has_flag(EnumT value, EnumT flag)
    {
        using U = std::underlying_type_t<EnumT>;
        return (static_cast<U>(value) & static_cast<U>(flag)) != 0;
    }

    enum class LogLevel
    {
        Info,
        Success,
        Warning,
        Error
    };

    enum class ErrorKind
    {
        Common,
        Global,
    };

    enum class ErrorSeverity
    {
        Info,
        Warning,
        Critical,
        Fatal,
    };

    enum class ErrorAction : uint32_t
    {
        None            = 0,
        Dismiss         = 1u << 0, // Close the popup
        Acknowledge     = 1u << 1, // Close the popup and signal FSM to continue
        Recover         = 1u << 2, // Ask FSM to attempt autonomous recovery
        ResetMachine    = 1u << 3, // Drops to StateWaitingInitialization
        RetryConnection = 1u << 4, // Drops to StateBooting
        PowerOff        = 1u << 5  // Emits EvPowerOff
    };
    impl_bin_operators_for_enum(ErrorAction);

    enum class DrawerTarget : uint32_t
    {
        None  = 0,
        Wafer = 1u << 0,
        Mask  = 1u << 1,
        Both  = Wafer | Mask
    };
    impl_bin_operators_for_enum(DrawerTarget);

    enum class StowageTarget : uint32_t
    {
        None  = 0,
        Mask  = 1u << 0,
        Wafer = 1u << 1,
        Both  = Mask | Wafer,
    };
    impl_bin_operators_for_enum(StowageTarget);

    enum class ForceSensor : uint32_t
    {
        None  = 0,
        Left  = 1u << 0,
        Right = 1u << 1,
        Back  = 1u << 2,
        All   = Left | Right | Back,
    };
    impl_bin_operators_for_enum(ForceSensor);

    enum class CameraId : uint32_t
    {
        LEFT,
        RIGHT,
    };

    enum class CameraDirection : uint32_t
    {
        UP,
        DOWN,
        LEFT,
        RIGHT
    };

    enum class AlignmentStageId : uint32_t
    {
        X     = 0x0,
        Y     = 0x1,
        THETA = 0x2
    };

    enum class ZElevatorId : uint32_t
    {
        LEFT,
        RIGHT,
        BACK,
    };

    enum class AlignmentStageDirection : uint32_t
    {
        X_LEFT,
        X_RIGHT,
        Y_FRONT,
        Y_BACK,
        THETA_CCW,
        THETA_CW,
    };

    enum class ZDirection : uint32_t
    {
        UP,
        DOWN,
    };

    enum class MovementKind : uint32_t
    {
        GRANULAR,
        CONTINUOUS,
        STOP
    };

} // Kub3
