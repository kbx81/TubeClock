#pragma once

#include <cstdint>

// Firmware version string, always "YY.MM.PP" (all components zero-padded to 2 digits).
// Fixed character positions: [0][1]=year, [3][4]=month, [6][7]=patch.
// Defined in BuildInfo.cpp. Include this header for read-only access.
extern const char kFirmwareVersion[];
extern const uint16_t kFirmwareBuild;
