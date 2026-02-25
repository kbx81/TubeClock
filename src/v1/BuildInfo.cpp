#include <cstdint>
#include "version.h"
#include "version_build.h"

// Stringify helpers — expand macro value to a string literal.
#define STR_(x) #x
#define STR(x)  STR_(x)

// Compile-time version string: "M.m.PP" — patch zero-padded to 2 digits
// so character positions are fixed: [0]=major, [2]=minor, [4]=patch tens, [5]=patch ones.
extern const char kFirmwareVersion[] =
    STR(FIRMWARE_VERSION_MAJOR) "."
    STR(FIRMWARE_VERSION_MINOR) "."
    FIRMWARE_VERSION_PATCH_STR;

extern const uint16_t kFirmwareBuild = FIRMWARE_BUILD_NUMBER;
