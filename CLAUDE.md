# CLAUDE.md

The aim of this file is to describe common mistakes and confusion points that agents might encounter as they work in this project. If you ever encounter something in the project that confuses and/or surprises you, please alert the developer working with you and suggest an edit to the `CLAUDE.md` file to help prevent future agents from having the same issue.

## Project Overview

- Microcontroller: STM32F072 (Cortex-M0)
- Depends on `libopencm3` checked out as a sibling directory to this project
- Nixie tube clock with six tubes which display time, date, and temperature with extensive features including:
  - GPS sync
  - Remote control via:
    - Serial (external MCU)
    - Infrared (IR)
    - DMX-512
  - Automatic display brightness adjustment
  - Capacitive touch input
  - Timer
  - Alarms

## Building the Firmware

Run builds in the `src/v1/` directory. Builds only take a few seconds, so running `make clean` first is harmless and recommended.

```bash
cd src/v1
make                    # Build with default profile (US/North America settings)
make PROFILE=1          # Build with European profile (24h, Celsius, no DST)
make PROFILE=2          # Build with Minimal profile (silent, basic display)
make PROFILE=3          # Build with kbx preferred profile
make clean              # Clean build artifacts
```

Before building on behalf of the user, ask which value is preferred for `PROFILE` if it's otherwise unclear.

Output binary files are generated in `../../build/v1/`:
- `TubeClock.elf` - ELF executable with debug symbols
- `TubeClock.bin` - Raw binary for flashing
- `TubeClock.map` - Memory map file

Typically, just use `make stats` which will automatically run `arm-none-eabi-size` post-build to determine the binary's composition. Avoid using `git stash` for before/after build comparisons; instead, run the build before making changes, then again afterwards to compare.

## Version Constants in version.h

`FIRMWARE_VERSION_MAJOR`, `FIRMWARE_VERSION_MINOR`, and `FIRMWARE_VERSION_PATCH` in `src/v1/version.h` **must remain `#define` macros** — do not convert them to `constexpr`. The build script `scripts/bump_build.sh` parses these values with `sed` to generate `version_build.h` (which holds the zero-padded string forms and the auto-incrementing build number). Converting to `constexpr` silently breaks the script, causing version_build.h to emit "00.00.00" and the device to display the wrong firmware version.

## Important Implementation Notes

This is an embedded system with limited processing power/speed, storage (flash memory) and RAM. Implementations should be as lean as possible.
- Avoid patterns which result in excessive/unnecessary construction of temporary data structures, particularly to make minor changes (for example, appending a single character to a long string).
- Avoid unnecessary copying of data passed between methods/functions.
- Avoid libraries which construct large data structures.

### Real-Time Constraints

Several interrupt handlers have real-time requirements:
- Timer 2 ISR (display PWM): Most critical and must complete quickly, runs at ~15.4 KHz, **timing consistency is CRITICAL to avoid visible flickering on the nixie tubes.** Other interrupts are given lower priorities to ensure that this interrupt and HV5622 latch strobes are not delayed.
- DMX-512 timing (Timer 15/16): Protocol-critical timing
- TSC (touch sensing): Must not miss touch events

Avoid long-running operations in ISR contexts; prefer deferring them to the main application loop.

### Hardware & Interface Prioritization

Due to the time-critical nature of software PWM, SPI bus access is managed by a simple request queuing mechanism which prioritizes bus access in this order:
- Display hardware/tube drivers (HV5622)
- RTC
- Temperature sensor(s)
- Other peripherals

As this is ultimately a clock, time accuracy is critical. As such, hardware RTC selection is prioritized as follows:
- GPS (used to sync DS323x)
- DS323x
- STM32's internal RTC

### Hardware Versions

There is currently only one hardware version (v1).

### Fixed-Point Scale Convention for RgbLed and NixieTube

Both classes use a **65536 = 100%** fixed-point scale for intensity and blend operations, shifted with `>> 16`. Do not use raw magic numbers — both classes provide a `pct()` helper that converts an integer percentage at compile time:

```cpp
led.adjustIntensity(RgbLed::pct(20));       // 20% of current intensity
tube.adjustIntensityAll(NixieTube::pct(75)); // 75% of all glyph intensities
```

**NixieTube intensity API** — there is no single-argument `adjustIntensity`. Use:
- `adjustIntensity(glyph, scaleFactor)` — scale one specific glyph
- `adjustIntensityAll(scaleFactor)` — scale all glyphs (prefer this during crossfades where multiple glyphs may have non-zero intensity)

**NixieTube LERP API:**
- `lerpToward(blendFactor, target)` — interpolates `*this` toward `target`; `blendFactor=0` = no change, `65535` = fully `target`
- `setFromLerp(blendFactor, from, to)` — sets `*this` to the interpolation of two other tubes; `blendFactor=0` = `from`, `65535` = `to`

Note: since all blend/scale parameters are typed `uint16_t` (max 65535), a value of exactly 65536 can never be passed. Guards like `if (x >= 65536)` are unreachable — do not add them.

### `cViewDescriptor` and `menuItemDisplayNumber` Are Hardware-Visible

`menuItemDisplayNumber` values in `cViewDescriptor[]` (Application.cpp) are displayed directly on the nixie tubes when navigating the clock's menu. They are not arbitrary internal identifiers — changing them changes what the user sees on the physical hardware. Do **not** renumber or reorder existing display numbers without explicit guidance from the developer. When inserting a new entry that requires shifting existing numbers, ask the developer first to confirm the new numbering scheme is acceptable.

### Static Objects with Virtual Destructors Require `_fini` Stub

Static-duration objects with virtual destructors (common with polymorphic View classes) register via `__cxa_atexit`, which pulls in `__libc_fini_array`, which references `_fini`. On bare metal this symbol is not provided by the runtime. A stub is present in `Stubs.cpp`, along with a minimal `__verbose_terminate_handler` and no-op syscall stubs (`_close`, `_lseek`, `_read`, `_write`) that suppress linker warnings about unimplemented newlib syscalls.

If new static polymorphic objects are added elsewhere and the linker complains about `_fini`, this is why. Do not add additional stubs — the ones in `Stubs.cpp` cover the whole binary.
