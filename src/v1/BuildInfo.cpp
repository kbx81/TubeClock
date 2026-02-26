#include <cstdint>
#include "version.h"
#include "version_build.h"

// Stringify helpers — expand macro value to a string literal.
#define STR_(x) #x
#define STR(x)  STR_(x)

// Compile-time version string: "YY.MM.PP" — all components zero-padded to 2 digits.
// Fixed character positions: [0][1]=year, [3][4]=month, [6][7]=patch.
extern const char kFirmwareVersion[] =
    FIRMWARE_VERSION_MAJOR_STR "."
    FIRMWARE_VERSION_MINOR_STR "."
    FIRMWARE_VERSION_PATCH_STR;

extern const uint16_t kFirmwareBuild = FIRMWARE_BUILD_NUMBER;
