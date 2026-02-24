#include "version.h"
#include "version_build.h"

// Stringify helpers — expand macro value to a string literal.
#define STR_(x) #x
#define STR(x)  STR_(x)

// Compile-time version string: "M.m.BB" — build is always zero-padded to 2 digits
// so character positions are fixed: [0]=major, [2]=minor, [4]=build tens, [5]=build ones.
extern const char kFirmwareVersion[] =
    STR(FIRMWARE_VERSION_MAJOR) "."
    STR(FIRMWARE_VERSION_MINOR) "."
    FIRMWARE_BUILD_NUMBER_STR;
