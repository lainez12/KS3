#pragma once

#include <utils.h>

namespace Kub3::MFSM
{

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

        [[nodiscard]] bool hasValue(void) const
        {
            return this->newWaferPosture.has_value() ||
                   this->newMaskPosture.has_value() ||
                   this->newVisionPosture.has_value();
        }
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

        void merge(const ExpectedSystemPosture &s)
        {
            if (s.newWaferPosture.has_value())
                this->wafer = s.newWaferPosture.value();
            if (s.newMaskPosture.has_value())
                this->mask = s.newMaskPosture.value();
            if (s.newVisionPosture.has_value())
                this->vision = s.newVisionPosture.value();
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

}
