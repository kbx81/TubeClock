#pragma once

#include <cstdint>

// Firmware version string, always "M.m.PP" (patch zero-padded to 2 digits).
// Fixed character positions: [0]=major, [2]=minor, [4]=patch tens, [5]=patch ones.
// Defined in BuildInfo.cpp. Include this header for read-only access.
extern const char     kFirmwareVersion[];
extern const uint16_t kFirmwareBuild;
