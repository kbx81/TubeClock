# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is firmware for a Nixie tube clock built on the STM32F072 (Cortex-M0) microcontroller using libopencm3. The clock displays time, date, and temperature on Nixie tubes with extensive features including GPS sync, DMX-512 control, IR remote control, capacitive touch input, a timer, alarms, and automatic brightness adjustment.

## Build System

### Building the Firmware

All builds happen from the `src/v1/` directory:

```bash
cd src/v1
make                    # Build with default profile (US/North America settings)
make PROFILE=1          # Build with European profile (24h, Celsius, no DST)
make PROFILE=2          # Build with Minimal profile (silent, basic display)
make PROFILE=3          # Build with kbx preferred profile
make clean              # Clean build artifacts
```

Output binary files are generated in `../../build/v1/`:
- `TubeClock.elf` - ELF executable with debug symbols
- `TubeClock.bin` - Raw binary for flashing
- `TubeClock.map` - Memory map file

### Settings Profiles

The build system supports compile-time configuration via `PROFILE` parameter (0-3):
- Profile 0 (default): US/North America - DST enabled, 12-hour format, Fahrenheit
- Profile 1: European - 24-hour format, Celsius, no DST handling
- Profile 2: Minimal - Silent mode with no beeps/effects, basic display
- Profile 3: Kbx preferred - Slower fades, quieter beeps

Before building on behalf of the user, ask which value is preferred for `PROFILE` if it's otherwise unclear.

### Hardware Version

The code requires `-DHARDWARE_VERSION=1` to compile. This is already set in the Makefile. This is currently the only hardware version.

## Flashing and Debugging

### Flashing to Hardware

Use ST-Link to flash the compiled binary:

```bash
st-flash write ../../build/v1/TubeClock.bin 0x08000000
```

### Debugging with GDB

For interactive debugging:

```bash
# Terminal 1: Start st-util
st-util --n

# Terminal 2: Connect GDB
arm-none-eabi-gdb ../../build/v1/TubeClock.elf --init-eval-command="target remote 127.0.0.1:4242"
```

## Architecture

### Core Components

The firmware is structured around several major subsystems:

**Hardware Layer** ([Hardware.cpp](src/v1/Hardware.cpp), [Hardware.h](src/v1/Hardware.h))
- Abstracts all MCU peripherals and external hardware
- Manages ADC, DMA, SPI, I2C, USARTs, timers, RTC, TSC, GPIO, watchdog
- Hardware version-specific definitions in [Hardware_v1.h](src/v1/Hardware_v1.h)
- Provides unified interface for automatically-detected RTC (internal STM32, external DS323x, or GPS)
- Automatically detects and handles multiple temperature sensor options (DS3231, DS3234, DS1722, LM74, etc.)

**Application Layer** ([Application.cpp](src/v1/Application.cpp), [Application.h](src/v1/Application.h))
- Main application logic and state machine
- Manages operating modes (40+ modes for various functions)
- Handles DST calculations and time zone adjustments
- Coordinates between hardware and display subsystems
- Manages settings persistence to flash memory

**Display Management** ([DisplayManager.cpp](src/v1/DisplayManager.cpp), [DisplayManager.h](src/v1/DisplayManager.h))
- Controls Nixie tube drivers via SPI (HV5622 high-voltage ICs)
- Implements software PWM for smooth fading/crossfading effects
- Master intensity control with automatic ambient light adjustment
- Manages status RGB LED

**Entry Point** ([TubeClock.cpp](src/v1/TubeClock.cpp))
- Interrupt vector configuration
- Hardware and application initialization
- Main application loop entry

### Key Subsystems

**DMX-512 Control** ([Dmx-512-Controller.cpp](src/v1/Dmx-512-Controller.cpp), [Dmx-512-Rx.cpp](src/v1/Dmx-512-Rx.cpp))
- RS-485 receiver on USART2 at 250kbaud, 2 stop bits
- Allows remote control of individual tubes from lighting console
- Uses Timer 15 and 16 for protocol timing supervision

**GPS Integration** ([GpsReceiver.cpp](src/v1/GpsReceiver.cpp))
- Supports LIV3F module, Adafruit GPS breakout, or other external serial-attached GPS module
- Connected via USART1 at 115200 baud with auto-baud detection
- Automatic time/date synchronization when fix acquired

**Touch Input** ([Keys.cpp](src/v1/Keys.cpp))
- Six capacitive touch keys using STM32 TSC peripheral
- Touch keys: U (up), D (down), E (enter/exit), as well as A, B, and C for miscellaneous use
- Key repeat handling for long presses

**IR Remote Control** ([InfraredRemote.cpp](src/v1/InfraredRemote.cpp))
- Timer 7 used for signal timing/decoding
- Enables display control from across the room

**Display Effects** ([Animator.cpp](src/v1/Animator.cpp))
- Various visual effects and animations for tubes
- Configurable duration and frequency

**Serial Remote Control** ([SerialRemote.cpp](src/v1/SerialRemote.cpp), [SerialRemote.h](src/v1/SerialRemote.h))
- NMEA-style ASCII protocol (`$TC{C|S}<category><action>[<data>]*XX\n`) with XOR checksum
- USART1 (shared with GPS, full duplex), USART3 (RX on PB10 with TX/RX swap), USART4 (TX on PA0) at 115200 baud
- Interrupt-driven I/O (no DMA - all 7 channels already allocated)
- ISR-driven RX state machine with double-buffered parsing, independent from GPS parser
- Command categories: P (page/mode), K (keys), H (hardware/ADC/HV), T (time), M (temperature), I (intensity), S (settings)
- Unsolicited notifications for mode changes and key press/release events
- Enables external MCU (WiFi, BLE, etc.) to monitor and control the clock

**Alarms** ([AlarmHandler.cpp](src/v1/AlarmHandler.cpp))
- Up to 8 configurable alarm slots
- Per-slot beep/blink configuration
- Optional hourly chime with binary-encoded time

### Display Architecture

The display system uses a layered approach:

1. **NixieGlyph** - Represents a single displayed digit/symbol with intensity
2. **NixieTube** - Represents a physical tube, manages glyphs and crossfading
3. **NixieGlyphCrossfader** - Handles smooth fading between glyph intensities
4. **Display** - Collection of tubes representing the complete display
5. **DisplayManager** - Hardware interface, manages SPI communication and PWM refresh

PWM refresh occurs via Timer 2 at ~15.4 KHz (256-step cycle = 60 Hz visible refresh), providing smooth intensity control and digit fading effects.

### Peripheral Usage Summary

- **ADC**: Light sensor (phototransistor), CPU temperature, VRef (system) and VBatt (battery) voltages
- **CRC**: Settings checksum validation
- **DMA**: SPI, I2C, and USART data transfers
- **FLASH**: Non-volatile settings storage in dedicated block
- **GPIO**: Status RGB LED, buzzer, buttons, miscellaneous control signals
- **IWDG**: Watchdog for crash recovery
- **RTC**: Timekeeping (internal or DS323x external)
- **SPI**: Tube drivers (HV5622), external RTC/temp sensors
- **Timer 1**: Buzzer PWM for tones
- **Timer 2**: Display PWM refresh at ~15.4 KHz
- **Timer 3**: Status LED RGB PWM
- **Timer 7**: IR receiver signal timing
- **Timer 15/16**: DMX-512 protocol supervision
- **TSC**: Capacitive touch sensing
- **USART1**: GPS module RX and serial remote TX/RX (115200 baud, auto-baud enabled)
- **USART2**: RS-485/DMX-512 (250000 baud, 2 stop bits, driver enable)
- **USART3**: Serial remote RX on PB10 (115200 baud, interrupt-driven, TX/RX swap enabled)
- **USART4**: Serial remote TX on PA0 (115200 baud, interrupt-driven)

## View System

The firmware implements a view-based UI architecture with multiple view types managing different display modes:

- **MainMenuView**: Menu navigation
- **TimeDateTempView**: Primary time/date/temperature display with optional automatic toggling
- **TimerCounterView**: Timer/counter functionality
- **Dmx512View**: DMX-512 controlled display
- **SetTimeDateView**: Time and date configuration
- **SetBitsView**: Binary options configuration
- **SetValueView**: Numeric value configuration
- **SystemStatusView**: Hardware status and diagnostics

Views are managed by Application layer and coordinate with DisplayManager to render output.

## Code Navigation

Start reading from [TubeClock.cpp](src/v1/TubeClock.cpp) to understand initialization and interrupt routing, then follow through [Application.cpp](src/v1/Application.cpp) for main logic and [Hardware.cpp](src/v1/Hardware.cpp) for peripheral details.

The codebase follows a namespace structure with `kbxTubeClock` as the root namespace containing sub-namespaces for major components (Application, Hardware, DisplayManager, etc.).

## Important Implementation Notes

This is an embedded system with limited storage (flash memory) and RAM. Implementations should be as lean as possible; avoid:
- Libraries which construct large data structures
- Unnecessary copying of data passed between methods/functions

### Real-Time Constraints

Several interrupt handlers have real-time requirements:
- Timer 2 ISR (display PWM): Must complete quickly, runs at ~15.4 KHz, **timing consistency is CRITICAL to avoid visible flickering on the nixie tubes.** SysTick is set to lower priority (128) than the DMA-complete ISR (64) to ensure HV5622 latch strobes are not delayed.
- DMX-512 timing (Timer 15/16): Protocol-critical timing
- TSC (touch sensing): Must not miss touch events

Avoid long-running operations in ISR contexts.

### Hardware & Interface Prioritization

Due to the time-critical nature of software PWM, SPI bus access is managed by a simple request queuing mechanism which prioritizes bus access in this order:
- Display hardware/tube drivers (HV5622)
- RTC
- Temperature sensor(s)
- Other peripherals

As this is ultimately a clock, time accuracy is critical. As such, hardware RTC selection is prioritized as follows:
- GPS
- DS323x
- STM32's internal RTC

### Display Refresh Strategy

The display uses software PWM at ~15.4 KHz (Timer 2) to emulate intensity control for the Nixie tubes. This enables smooth crossfading effects between digits. The DisplayManager handles tube intensity calculations and SPI transfers to the HV5622 driver ICs. The SPI bus caches the active slave selection to avoid costly reconfiguration between consecutive same-slave transfers, which is critical for consistent ISR timing.

### Flash Settings Storage

Settings are persisted to a reserved flash block defined in the custom linker script [stm32f07xzb-tubeclock.ld](src/v1/stm32f07xzb-tubeclock.ld). Settings include CRC validation to detect absence/corruption.

## Dependencies

- **libopencm3**: Hardware abstraction library located at `../../../../libopencm3` relative to src/v1/
- **arm-none-eabi-gcc**: Compiler toolchain with C++11 support
- **st-flash/st-util**: ST-Link utilities for flashing and debugging

The project expects libopencm3 to be checked out as a sibling directory to this project.
