#ifndef DEFINES_H
#define DEFINES_H

#include <cstdint>

namespace sys_cfg {

    // Parametros de imagen (RAW RGB888, 1080p)
    const int IMG_WIDTH    = 1920;
    const int IMG_HEIGHT   = 1080;
    const int IMG_CHANNELS = 3;

    const unsigned int RGB_SIZE  = IMG_WIDTH * IMG_HEIGHT * IMG_CHANNELS; // 6,220,800 B
    const unsigned int GRAY_SIZE = IMG_WIDTH * IMG_HEIGHT;                // 2,073,600 B

    // Mapeo de memoria
    // RAM: 64 MB
    const uint64_t RAM_BASE_ADDR   = 0x00000000ULL;
    const uint64_t RAM_SIZE        = 64ULL * 1024 * 1024;
    const uint64_t INPUT_IMG_ADDR  = 0x00000000ULL; // imagen RGB entrada en RAM
    const uint64_t OUTPUT_IMG_ADDR = 0x00600000ULL; // imagen Gray salida en RAM

    //disk_storage
    const uint64_t DISK_BASE_ADDR  = 0x40000000ULL;

    // registros de control aceleraor
    const uint64_t ACCEL_BASE_ADDR  = 0x10000000ULL;
    const uint64_t REG_BASE_IN      = ACCEL_BASE_ADDR + 0x00;
    const uint64_t REG_BASE_OUT     = ACCEL_BASE_ADDR + 0x04;
    const uint64_t REG_NUM_PIXELS   = ACCEL_BASE_ADDR + 0x08;
    const uint64_t REG_CONTROL      = ACCEL_BASE_ADDR + 0x0C;
    const uint64_t REG_STATUS       = ACCEL_BASE_ADDR + 0x10;
    const uint64_t ACCEL_REG_SIZE   = 0x100;

} // namespace sys_cfg

#endif // DEFINES_H