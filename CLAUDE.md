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
