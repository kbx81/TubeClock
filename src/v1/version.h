#pragma once

// Calendar-based versioning: YY.MM.patch
//   Major = 2-digit year, Minor = month (1–12, NO leading zero — avoids octal literals),
//   Patch = sequential release within the month (also no leading zero).
// Build number increments automatically at compile time (see scripts/bump_build.sh)
// and resets when major (year) or minor (month) changes.
// NOTE: These must remain #defines — scripts/bump_build.sh parses them via sed.
#define FIRMWARE_VERSION_MAJOR 26
#define FIRMWARE_VERSION_MINOR 5
#define FIRMWARE_VERSION_PATCH 0
