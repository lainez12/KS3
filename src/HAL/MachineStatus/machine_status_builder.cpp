#include "HAL/MachineStatus/machine_status_builder.h"

namespace Kub3::HAL::MS
{
    void build_machine_status(IMachineStatusRepo &repo)
    {
        // Encoders
        repo.setSensor<int32_t>(WAFER_DRAWER_POS, 0);
        repo.setSensor<int32_t>(MASK_DRAWER_POS, 0);
        repo.setSensor<int32_t>(Z_LEFT_POS, 0);
        repo.setSensor<int32_t>(Z_RIGHT_POS, 0);
        repo.setSensor<int32_t>(Z_BACK_POS, 0);

        // Stops
        // --- Wafer drawer
        repo.setSensor<bool>(CW0, false);
        repo.setSensor<bool>(CW1, false);
        repo.setSensor<bool>(CW2, false);
        // --- Mask drawer
        repo.setSensor<bool>(CM0, false);
        repo.setSensor<bool>(CM1, false);
        repo.setSensor<bool>(CM2, false);
        repo.setSensor<bool>(CM3, false);

        // Force sensors (raw ADC value)
        repo.setSensor<uint16_t>(FORCE_LEFT, 0);
        repo.setSensor<uint16_t>(FORCE_RIGHT, 0);
        repo.setSensor<uint16_t>(FORCE_BACK, 0);
        // Solenoid valves (open/closed)
        repo.setSensor<bool>(MASK_VACUUM_VALVE, false);
        repo.setSensor<bool>(WAFER_VACUUM_VALVE, false);
        repo.setSensor<bool>(WAFER_COMPRESSED_AIR_VALVE, false);
        // Barometers (threshold-based)
        repo.setSensor<bool>(MASK_VACUUM_ACTIVE, false);
        repo.setSensor<bool>(WAFER_VACUUM_ACTIVE, false);
        repo.setSensor<bool>(WAFER_COMPRESSED_AIR_ACTIVE, false);
    }
}
