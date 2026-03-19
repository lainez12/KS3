#pragma once

#include "HAL/MachineStatus/IMachineStatusRepo.h"

namespace Kub3::HAL::MS
{

#define WAFER_DRAWER_POS "waferDrawerEncoderPosition"
#define MASK_DRAWER_POS  "maskDrawerEncoderPosition"
#define Z_LEFT_POS       "zLeftEncoderPosition"
#define Z_RIGHT_POS      "zRightEncoderPosition"
#define Z_BACK_POS       "zBackEncoderPosition"

#define CW0 "waferCW0Stop"
#define CW1 "waferCW1Stop"
#define CW2 "waferCW2Stop"

#define CM0 "maskCM0Stop"
#define CM1 "maskCM1Stop"
#define CM2 "maskCM2Stop"
#define CM3 "maskCM3Stop"

#define FORCE_LEFT  "leftForceSensor"
#define FORCE_RIGHT "rightForceSensor"
#define FORCE_BACK  "backForceSensor"

#define MASK_VACUUM_VALVE          "maskVacuumValveStatus"
#define WAFER_VACUUM_VALVE         "waferVacuumValveStatus"
#define WAFER_COMPRESSED_AIR_VALVE "waferCompressedAirValveStatus"

#define MASK_VACUUM_ACTIVE          "maskVacuumActive"
#define WAFER_VACUUM_ACTIVE         "waferVacuumActive"
#define WAFER_COMPRESSED_AIR_ACTIVE "waferCompressedAirActive"

    void build_machine_status(IMachineStatusRepo &repo);

}
