#pragma once

#include <QDebug>

#include <utils.h>

namespace Kub3::MFSM
{

    // ----------------------
    // --- SYSTEM POSTURE
    // ----------------------

    enum class WaferPosture
    {
        Unknown, // Machine lost track (e.g. after Error, Boot, or E-Stop)
        Ejected, // Wafer drawer is ejected
        DrawerMidway,
        Homed, // Wafer conveyor is inserted, Z is down
        ElevatorMidway,
        AlignmentZone, // Z is up at Z2, ready for cameras/contact
    };

    enum class MaskPosture
    {
        Unknown, // Machine lost track (e.g. after Error, Boot, or E-Stop)
        Ejected, // Mask drawer is ejected
        DrawerMidway,
        Homed, // Mask conveyor just met the CM2 limit switch
        ExposureMidway,
        Exposure, // Mask is vacuum-secured in exposure position
    };

    enum class VisionPosture
    {
        Unknown, // Machine lost track (e.g. after Error, Boot, or E-Stop)
        Homed,   // Could be named `Exposure`: the position not obstructing led heads
        Midway,
        Visualisation, // Cameras' deck positioned above the mask and alignment stages, cameras in any position
    };

    struct ExpectedSystemPosture {
        Optional<WaferPosture> newWaferPosture   = std::nullopt;
        Optional<MaskPosture> newMaskPosture     = std::nullopt;
        Optional<VisionPosture> newVisionPosture = std::nullopt;
        Optional<bool> newLevelingValid          = std::nullopt;

        [[nodiscard]] bool hasValue(void) const
        {
            return this->newWaferPosture.has_value() ||
                   this->newMaskPosture.has_value() ||
                   this->newVisionPosture.has_value() ||
                   this->newLevelingValid.has_value();
        }
    };

    struct SystemPosture {
        WaferPosture wafer   = WaferPosture::Unknown;
        MaskPosture mask     = MaskPosture::Unknown;
        VisionPosture vision = VisionPosture::Unknown;
        bool isLevelingValid = false;

        [[nodiscard]] bool isFullyKnown() const
        {
            return wafer != WaferPosture::Unknown &&
                   mask != MaskPosture::Unknown &&
                   vision != VisionPosture::Unknown;
        }

        void merge(const ExpectedSystemPosture &s)
        {
            if (s.newWaferPosture.has_value())
                this->wafer = s.newWaferPosture.value();
            if (s.newMaskPosture.has_value())
                this->mask = s.newMaskPosture.value();
            if (s.newVisionPosture.has_value())
                this->vision = s.newVisionPosture.value();
            if (s.newLevelingValid.has_value())
                this->isLevelingValid = s.newLevelingValid.value();
        }

        [[nodiscard]] SystemPosture invalidate(const ExpectedSystemPosture &e) const
        {
            return SystemPosture{
                .wafer           = e.newWaferPosture.has_value() ? WaferPosture::Unknown : this->wafer,
                .mask            = e.newMaskPosture.has_value() ? MaskPosture::Unknown : this->mask,
                .vision          = e.newVisionPosture.has_value() ? VisionPosture::Unknown : this->vision,
                .isLevelingValid = e.newLevelingValid.has_value() ? false : this->isLevelingValid,
            };
        }
    };

    inline QDebug operator<<(QDebug dbg, WaferPosture v)
    {
        QDebugStateSaver saver(dbg);
        switch (v)
        {
        case WaferPosture::Unknown:
            dbg.nospace() << "Unknown";
            break;
        case WaferPosture::Ejected:
            dbg.nospace() << "Ejected";
            break;
        case WaferPosture::DrawerMidway:
            dbg.nospace() << "DrawerMidway";
            break;
        case WaferPosture::Homed:
            dbg.nospace() << "Homed";
            break;
        case WaferPosture::ElevatorMidway:
            dbg.nospace() << "ElevatorMidway";
            break;
        case WaferPosture::AlignmentZone:
            dbg.nospace() << "AlignmentZone";
            break;
        }
        return dbg;
    }

    inline QDebug operator<<(QDebug dbg, MaskPosture v)
    {
        QDebugStateSaver saver(dbg);
        switch (v)
        {
        case MaskPosture::Unknown:
            dbg.nospace() << "Unknown";
            break;
        case MaskPosture::Ejected:
            dbg.nospace() << "Ejected";
            break;
        case MaskPosture::DrawerMidway:
            dbg.nospace() << "DrawerMidway";
            break;
        case MaskPosture::Homed:
            dbg.nospace() << "Homed";
            break;
        case MaskPosture::ExposureMidway:
            dbg.nospace() << "ExposureMidway";
            break;
        case MaskPosture::Exposure:
            dbg.nospace() << "Exposure";
            break;
        }
        return dbg;
    }

    inline QDebug operator<<(QDebug dbg, VisionPosture v)
    {
        QDebugStateSaver saver(dbg);
        switch (v)
        {
        case VisionPosture::Unknown:
            dbg.nospace() << "Unknown";
            break;
        case VisionPosture::Homed:
            dbg.nospace() << "Homed";
            break;
        case VisionPosture::Midway:
            dbg.nospace() << "Midway";
            break;
        case VisionPosture::Visualisation:
            dbg.nospace() << "Visualisation";
            break;
        }
        return dbg;
    }

}
