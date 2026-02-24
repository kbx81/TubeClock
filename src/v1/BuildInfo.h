#pragma once

#include <cstdint>

// Firmware version string, always "M.m.BB" (build zero-padded to 2 digits).
// Fixed character positions: [0]=major, [2]=minor, [4]=build tens, [5]=build ones.
// Defined in BuildInfo.cpp. Include this header for read-only access.
extern const char kFirmwareVersion[];
