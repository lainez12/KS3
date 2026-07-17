#pragma once

#include <Services/IService.h>

namespace Kub3
{
    class TestToken;
}

namespace Kub3::Services
{

    namespace HomingTarget
    {
        enum Type : uint32_t
        {
            NONE             = 0x0,
            MASK_CONVEYOR    = 1u << 0,
            WAFER_CONVEYOR   = 1u << 1,
            Z_MOTORS         = 1u << 2,
            ALIGNMENT_STAGES = 1u << 3,
            CAMERAS          = 1u << 4,
            DECK             = 1u << 5,
            ALL              = MASK_CONVEYOR | WAFER_CONVEYOR | Z_MOTORS | ALIGNMENT_STAGES | CAMERAS | DECK,
        };

        // TODO: handle granular requests:
        // enum Type : uint32_t
        // {
        //     MASK_CONVEYOR         = 1u << 0,
        //     WAFER_CONVEYOR        = 1u << 1,
        //     Z_LEFT                = 1u << 2,
        //     Z_RIGHT               = 1u << 3,
        //     Z_BACK                = 1u << 4,
        //     Z_MOTORS              = Z_LEFT | Z_RIGHT | Z_BACK,
        //     X_ALIGNMENT_STAGE     = 1u << 5,
        //     Y_ALIGNMENT_STAGE     = 1u << 6,
        //     THETA_ALIGNMENT_STAGE = 1u << 7,
        //     ALIGNMENT_STAGES      = X_ALIGNMENT_STAGE | Y_ALIGNMENT_STAGE | THETA_ALIGNMENT_STAGE,
        //     LEFT_CAMERA_X         = 1u << 8,
        //     LEFT_CAMERA_Y         = 1u << 9,
        //     RIGHT_CAMERA_X        = 1u << 10,
        //     RIGHT_CAMERA_Y        = 1u << 11,
        //     CAMERAS               = LEFT_CAMERA_X | LEFT_CAMERA_Y | RIGHT_CAMERA_X | RIGHT_CAMERA_Y,
        //     DECK                  = 1u << 12,
        //     ALL                   = MASK_CONVEYOR | WAFER_CONVEYOR | Z_MOTORS | ALIGNMENT_STAGES | CAMERAS | DECK,
        // };
    }

    class IHomingService : public IService
    {
    public:
        ~IHomingService() = default;

        virtual void initialize()                                        = 0;
        virtual void home(HomingTarget::Type target = HomingTarget::ALL) = 0;

        // Test methods
        virtual void runGranularAction(TestToken, HomingTarget::Type target, bool initialization = true) = 0;
    };

}
