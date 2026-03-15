# Serial Remote Control API

This document describes the serial protocol used to monitor and control the TubeClock from an external microcontroller (for example, ESP32 or RP2040). The interface enables remote access to all major clock functions: display mode selection, time/date, temperature, intensity, settings, hardware status, and key events.

## Physical Layer

| Parameter | Value |
|-----------|-------|
| Baud rate | 115200 |
| Data bits | 8 |
| Parity | None |
| Stop bits | 1 |
| Logic level | 3.3V |

### USART Channels

The clock uses three USARTs for the serial remote interface:

| USART | Direction | Pin | Notes |
|-------|-----------|-----|-------|
| USART1 | TX + RX | TX: PB6, RX: PA10 | Shared with GPS. GPS uses `$GP` prefix; serial remote uses `$TC`. Both parsers operate independently on every received byte. |
| USART3 | RX only | PB10 | Dedicated serial remote input. Uses `USART_CR2_SWAP` (PB10 is normally a TX pin but is hardware-swapped to function as RX). |
| USART4 | TX only | PA0 | Dedicated serial remote output. |

**Recommended wiring:** Connect the external MCU's TX to USART3 RX (PB10) and the external MCU's RX to USART4 TX (PA0). This avoids contention with GPS traffic on USART1. USART1 may also be used for bidirectional communication if GPS is not connected.

All responses are transmitted simultaneously on both USART1 TX and USART4 TX.

## Protocol Format

The protocol uses NMEA-style ASCII sentences:

```
$TC{C|S}<category><action>[<data>]*XX\n
```

| Field | Description |
|-------|-------------|
| `$TC` | Protocol header (4 bytes including direction) |
| `C` | **Command** -- external MCU to clock |
| `S` | **Status** -- clock to external MCU (solicited response or unsolicited notification) |
| `<category>` | Single character identifying the command category |
| `<action>` | Category-specific action and data |
| `*` | Checksum delimiter |
| `XX` | XOR checksum as two uppercase hex digits |
| `\n` | Sentence terminator (newline, 0x0A) |

### Checksum Calculation

The checksum is the XOR of all bytes between `$` (exclusive) and `*` (exclusive). For example, for the sentence `$TCCP*XX\n`, the checksum covers the bytes `T`, `C`, `C`, `P`.

```
checksum = 'T' ^ 'C' ^ 'C' ^ 'P'
         = 0x54 ^ 0x43 ^ 0x43 ^ 0x50
         = 0x54 ^ 0x50       (0x43 ^ 0x43 = 0x00)
         = 0x04
```

Result: `$TCCP*04\n`

Sentences received without a checksum (terminated by `\n` before `*`) are also accepted.

### Error Responses

When a `$TC` command is received but the category character is not recognized, or the payload is too short to parse, the clock sends an error response:

```
$TCSE<category>*XX\n
```

where `<category>` is the unrecognized category character echoed back. If the payload is too short to contain a category, `?` is used instead.

Examples:
```
-> $TCCZTEST*XX\n      (unknown category 'Z')
<- $TCSE Z*XX\n

-> $TCC*XX\n           (payload too short)
<- $TCSE?*XX\n
```

If a checksum is present but does not match, the clock sends a checksum error response:

```
$TCSECHK*XX\n
```

Malformed headers and other framing errors are silently discarded with no response.

### Numeric Values

All numeric values in commands and responses are decimal ASCII. Leading zeros may be present in fixed-width fields (for example, time: `093015` for 09:30:15). Negative values are prefixed with `-`.

## Command Reference

### P -- Page/Mode

Controls the clock's current display mode (operating mode) and optional sub-page (view mode).

#### Get Current Page

```
-> $TCCP*04\n
<- $TCSP1*42\n
```

The response includes the current operating mode number. If the view mode (sub-page) is non-zero, it is appended:

```
<- $TCSP8P2*XX\n     (page 8, sub-page 2)
```

#### Set Page

```
-> $TCCP3*XX\n        (switch to page 3)
<- $TCSP3*XX\n        (confirmation)
```

#### Set Page with Sub-Page

```
-> $TCCP8P1*XX\n      (switch to page 8, sub-page 1)
<- $TCSP8P1*XX\n      (confirmation)
```

#### Operating Mode Values

| Value | Mode | Description |
|-------|------|-------------|
| 0 | MainMenu | Main menu navigation |
| 1 | FixedDisplay | Fixed time/date/temp display |
| 2 | ToggleDisplay | Auto-toggling display |
| 3 | TimerCounter | Timer/counter |
| 4 | Dmx512Display | DMX-512 controlled display |
| 5 | SetClock | Set time |
| 6 | SetDate | Set date |
| 7 | SetTimerResetValue | Set timer reset value |
| 8 | SystemStatusView | System status/diagnostics |
| 9 | SetSystemOptions | System options (binary flags) |
| 10 | SlotBeepConfig | Per-slot beep enable/disable |
| 11 | SlotBlinkConfig | Per-slot blink enable/disable |
| 12 | SlotOnOffConfig | Per-slot color change enable/disable |
| 13 | SlotPMIndicatorRGBConfig | PM indicator LED RGB color |
| 14 | SetDurationClock | Clock display duration |
| 15 | SetDurationDate | Date display duration |
| 16 | SetDurationTemp | Temperature display duration |
| 17 | SetDurationFade | Digit fade duration |
| 18 | DstBeginMonth | DST start month |
| 19 | DstBeginDowOrdinal | DST start day ordinal |
| 20 | DstEndMonth | DST end month |
| 21 | DstEndDowOrdinal | DST end day ordinal |
| 22 | DstSwitchDayOfWeek | DST switch day of week |
| 23 | DstSwitchHour | DST switch hour |
| 24 | SetEffectDuration | Display effect duration |
| 25 | SetEffectFrequency | Display effect frequency |
| 26 | SetMinimumIntensity | Minimum display intensity |
| 27 | SetBeeperVolume | Beeper volume |
| 28 | SetTempCalibration | Temperature calibration |
| 29 | SetIdleTimeout | Idle timeout |
| 30 | SetDateFormat | Date format |
| 31 | SetTimeZone | Time zone |
| 32 | SetColonBehavior | Colon/separator behavior |
| 33 | SetDMX512Address | DMX-512 address |
| 34-41 | Slot1Time-Slot8Time | Set slot 1-8 time |

#### View Mode (Sub-Page) Values

| Value | Mode |
|-------|------|
| 0 | ViewMode0 (default) |
| 1 | ViewMode1 |
| 2 | ViewMode2 |
| 3 | ViewMode3 |
| 4 | ViewMode4 |
| 5 | ViewMode5 |
| 6 | ViewMode6 |
| 7 | ViewMode7 |
| 8 | ViewMode8 |
| 9 | ViewMode9 |

The meaning of each view mode is context-dependent on the active operating mode.

---

### K -- Keys

Queries the current state of the capacitive touch keys.

#### Get Key State

```
-> $TCCK*XX\n
<- $TCSK<bitmask>*XX\n
```

The response is a decimal bitmask of currently pressed keys:

| Bit | Key |
|-----|-----|
| 0 (1) | U (Up) |
| 1 (2) | D (Down) |
| 2 (4) | E (Enter/Exit) |
| 3 (8) | C |
| 4 (16) | B |
| 5 (32) | A |

Example: `$TCSK5*XX\n` means keys U (1) and E (4) are pressed simultaneously.

A response of `$TCSK0*XX\n` means no keys are pressed.

---

### H -- Hardware

Hardware status queries and control.

#### Get ADC Data

```
-> $TCCHADC*XX\n
<- $TCSHADC<light>,<vdda>,<vbatt>*XX\n
```

Returns three comma-separated decimal values:
- **light**: Ambient light level (raw ADC, phototransistor)
- **vdda**: VddA supply voltage in millivolts
- **vbatt**: Battery voltage in millivolts

Example: `$TCSHADC1782,3300,2791*XX\n`

This response is also sent as an **unsolicited status notification** whenever a new ADC reading is obtained and at least one value has changed. See [ADC Update](#adc-update) in the Unsolicited Notifications section.

#### Get Connected Components

```
-> $TCCHCON*XX\n
<- $TCSHCON<bitmask>*XX\n
```

Returns a decimal bitmask of detected hardware:

| Bit | Component |
|-----|-----------|
| 0 (1) | DS3234 temperature sensor |
| 1 (2) | DS1722 temperature sensor |
| 2 (4) | LM74 temperature sensor |
| 3 (8) | GPS connected |
| 4 (16) | GPS fix valid |

Example: `$TCSHCON24*XX\n` means GPS connected (8) + GPS valid (16).

#### Get HV State

```
-> $TCCHV*XX\n
<- $TCSHV<0|1>*XX\n
```

Returns the current state of the high-voltage supply: `1` = enabled, `0` = disabled.

#### HV Power On

```
-> $TCCHVON*XX\n
<- $TCSHV1*XX\n
```

Enables the high-voltage supply for the Nixie tubes.

#### HV Power Off

```
-> $TCCHVOF*XX\n
<- $TCSHV0*XX\n
```

Disables the high-voltage supply for the Nixie tubes.

#### Bootloader Control

```
-> $TCCHBOOT0*XX\n      Disable: clear bootloader-on-next-boot flag
-> $TCCHBOOT1*XX\n      Enable:  set flag; bootloader runs on next reset
-> $TCCHBOOT2*XX\n      Enter:   set flag then reset immediately

<- $TCSHBOOT0*XX\n
<- $TCSHBOOT1*XX\n
<- $TCSHBOOT2*XX\n      (ACK sent before reset fires)
```

Controls entry into the STM32F072 factory bootloader. The clock may be
re-flashed via USART1 (PA9 TX / PA10 RX) or USART2 (PA14 TX / PA15 RX)
using the standard STM32 bootloader protocol.

- **BOOT0** clears the bootloader flag, cancelling a previously armed BOOT1.
- **BOOT1** arms the flag. The clock operates normally until the next reset (for example,
  via the external MCU asserting NRST), at which point it jumps to the bootloader
  instead of running the application.
- **BOOT2** arms the flag and triggers a system reset immediately after the ACK
  is transmitted. Equivalent to BOOT1 followed by a NRST pulse.

Normal operation resumes on the next power cycle or reset with BOOT0 low.

---

### T -- Time

Get or set the current date and time.

#### Get Time

```
-> $TCCT*XX\n
<- $TCST<HHMMSSYYYYMMDD>*XX\n
```

Returns 14 digits: hours (2), minutes (2), seconds (2), year (4), month (2), day (2).

Example: `$TCST234520260211*XX\n` = 23:45:20 on 2026-02-11.

#### Set Time

```
-> $TCCT<HHMMSSYYYYMMDD>*XX\n
<- $TCST<HHMMSSYYYYMMDD>*XX\n
```

Sets the clock to the specified time and date. The response confirms the new time.

Example: `$TCCT120000202602120A*XX\n` sets the time to 12:00:00 on 2026-02-12.

The payload must contain exactly 14 digits starting at the data position:
- `HH` -- Hours (00-23)
- `MM` -- Minutes (00-59)
- `SS` -- Seconds (00-59)
- `YYYY` -- Year (for example, 2026)
- `MM` -- Month (01-12)
- `DD` -- Day (01-31)

---

### M -- Temperature

Get or set the current temperature. Responses include a value for every sensor type.

#### Get Temperature

```
-> $TCCM*XX\n
<- $TCSM<adc>,<ds3234>,<ds1722>,<lm74>,<external>*XX\n
```

Returns temperatures from all sensor types as comma-separated signed decimal integers in **tenths of degrees Celsius** (Celsius x10). The fields correspond to the `TempSensorType` enum values in order:

| Index | Sensor            | Notes                                           |
|-------|-------------------|-------------------------------------------------|
| 0     | STM32ADC      | STM32 internal ADC temperature (always present) |
| 1     | DS3234            | DS3234 RTC integrated temperature sensor        |
| 2     | DS1722            | External DS1722 SPI temperature sensor          |
| 3     | LM74              | External LM74 SPI temperature sensor            |
| 4     | ExternalSerial    | Externally set via Set Temperature command       |

Undetected or unavailable sensors report `32767` (sentinel value).

Example: `$TCSM235,237,32767,32767,32767*XX\n` (internal ADC reads 23.5°C, DS3234 reads 23.7°C, no other sensors detected).

This response is also sent as an **unsolicited status notification** whenever a new temperature reading is obtained from hardware.

#### Set Temperature

```
-> $TCCM<value>*XX\n
<- $TCSM<adc>,<ds3234>,<ds1722>,<lm74>,<external>*XX\n
```

Sets the temperature from an external source. The value is in **tenths of degrees Celsius** (Celsius x10, signed). Once set, this external value takes precedence over all auto-detected hardware temperature sensors (DS1722, LM74, DS3234, STM32 internal ADC) and persists until changed or until reboot.

The response returns all sensor values in the same format as Get Temperature.

Examples:
- `$TCCM235*XX\n` sets temperature to 23.5°C
- `$TCCM-50*XX\n` sets temperature to -5.0°C

Setting a temperature value automatically sets the temperature source to `ExternalSerial` (4).

#### Get Temperature Source

```
-> $TCCMS*XX\n
<- $TCSMS<n>*XX\n
```

Returns the active temperature sensor type as a decimal integer (see table below).

Example: `$TCSMS4*XX\n` (ExternalSerial is the active source).

#### Set Temperature Source

```
-> $TCCMS<n>*XX\n
<- $TCSMS<n>*XX\n       (success)
<- $TCSMSE*XX\n          (error: sensor not available)
```

Selects the temperature source. If the requested sensor was not detected at startup, an error response is returned and the source is not changed.

Example: `$TCCMS0*XX\n` switches to the internal STM32 ADC.

#### Temperature Source Values

| Value | Source | Notes |
|-------|--------|-------|
| 0 | Internal STM32 ADC | Always available |
| 1 | DS3234 | Available if DS3234 RTC is connected |
| 2 | DS1722 | Available if detected at startup |
| 3 | LM74 | Available if detected at startup |
| 4 | ExternalSerial | Always available; temperature set via M command |

---

### L -- Status LED

Get or set the RGB status LED color, intensity, gamma mode, and auto-brightness state. Color values are 0–255 per channel; internally the LED uses 12-bit resolution (0–4095), so the serial interface scales by a factor of 16 in each direction. Intensity is 0–255. The LED has its own auto-brightness flag, independent of the nixie tube auto-brightness (see [I -- Intensity](#i----intensity)).

#### Get LED State

```
-> $TCCL*XX\n
<- $TCSL<i>,<r>,<g>,<b>,<gamma>,<auto>*XX\n
```

Returns six comma-separated values:
- **i**: LED intensity (0–255)
- **r**, **g**, **b**: Red, green, blue channel values (0–255 each)
- **gamma**: Gamma correction mode — `1` = firmware applies gamma correction; `0` = values are already gamma-corrected (correction skipped)
- **auto**: LED auto-adjust enabled (`1`) or disabled (`0`)

Example: `$TCSL128,255,0,0,1,0*XX\n` (red LED at intensity 128, gamma correction on, auto-adjust off).

#### Set LED State

```
-> $TCCL<i>,<r>,<g>,<b>[,<gamma>[,<auto>]]*XX\n
<- $TCSL<i>,<r>,<g>,<b>,<gamma>,<auto>*XX\n
```

Sets the LED intensity and color. The optional `<gamma>` field (`0` or `1`) controls whether the firmware applies gamma correction to the received values. The optional trailing `<auto>` field (`0` or `1`) sets the LED auto-brightness flag. The response confirms the new state.

- `gamma=1` (default when omitted): color values are linear; the firmware applies `gammaCorrect12bit()` during rendering for perceptually linear brightness.
- `gamma=0`: color values are already gamma-corrected; the firmware skips its internal gamma correction step. Use this when your controller has pre-applied gamma (for example, from a lighting control system).

**Note:** The gamma flag applies only to the values in this command. Any subsequent write to the LED — from an internal view or a new TCCL command — resets to `gamma=1` (apply correction) unless explicitly specified otherwise.

Example: `$TCCL255,255,128,0*XX\n` sets the LED to orange at full intensity (gamma applied by firmware).
Example: `$TCCL200,186,0,0,0*XX\n` sets the LED to red at intensity 200 with pre-corrected values (firmware skips gamma).
Example: `$TCCL50,0,255,0,1,1*XX\n` sets the LED to green at intensity 50, gamma on, auto-adjust enabled.

**Note:** Setting the LED intensity with this command disables LED auto-adjust (unless the optional `<auto>` field is included and set to `1`). Tube auto-adjust is not affected.

**Breaking change:** The `<auto>` field has shifted from position 5 to position 6. Senders using the old `$TCCL<i>,<r>,<g>,<b>,<auto>` form must insert the `<gamma>` field before `<auto>`.

#### Set LED Auto-Adjust

```
-> $TCCLA<0|1>*XX\n
<- $TCSL<i>,<r>,<g>,<b>,<gamma>,<auto>*XX\n
```

Enables (`1`) or disables (`0`) automatic LED intensity adjustment based on ambient light. This flag is independent of tube auto-adjust (`$TCCIA`). The response reports the current LED state.

Example: `$TCCLA1*XX\n` enables LED auto-adjust.

**Note:** When LED auto-adjust is enabled, the LED intensity tracks the nixie tube intensity as the ambient light changes. When the AM/PM indicator feature is enabled (`StatusLedAsAmPm` system option), the clock manages LED auto-adjust automatically: it is enabled during PM time and disabled when the indicator is off (AM or non-time display), preventing auto-brightness from turning the LED on when it should be off.

**Note:** The LED color and intensity set via this command persist until a view that manages the LED internally changes the color (for example, a PM indicator transition in `TimeDateTempView`, or entering `MainMenuView`).

This response is also sent as an **unsolicited status notification** whenever the LED color, intensity, gamma mode, or auto-adjust state changes from any source (internal views, external commands, or auto-brightness).

---

### B -- Buzzer (RTTTL Playback)

Play, stop, or query RTTTL (Ring Tone Text Transfer Language) melodies through the clock's buzzer.

[RTTTL format](https://en.wikipedia.org/wiki/Ring_Tone_Transfer_Language): `Name:d=<dur>,o=<oct>,b=<bpm>:<notes>`

| Field | Description |
|-------|-------------|
| `Name` | Song name (up to ~15 characters, ignored during playback) |
| `d=` | Default note duration: 1=whole, 2=half, 4=quarter, 8=eighth, 16=sixteenth, 32=thirty-second |
| `o=` | Default octave (4–7) |
| `b=` | BPM (beats per minute, in quarter notes) |
| notes | Comma-separated sequence of `[dur][note][#][oct][.]` |

Note characters: `c`, `d`, `e`, `f`, `g`, `a`, `b` (or `h`), `p` (pause). Append `#` for sharp, a digit for octave override, `.` for dotted (50% longer).

#### Play RTTTL

```
-> $TCCBPName:d=4,o=5,b=120:4e,4d,4c*XX\n
<- $TCSBP*XX\n
```

The RTTTL string begins immediately after the `P` action character. The response reports current playback status: `P` (playing) or `S` (stopped). If playback is already in progress, it is interrupted and the new song starts immediately.

Maximum RTTTL string length: 251 bytes (within the 255-byte payload limit, minus the 3-byte `CBP` prefix).

Example: play two notes (E and C in octave 5 at 120 BPM):
```
-> $TCCBPTwo:d=4,o=5,b=120:e,c*XX\n
<- $TCSBP*XX\n
```

#### Stop Playback

```
-> $TCCBS*XX\n
<- $TCSBS*XX\n
```

Stops RTTTL playback immediately. The current note (already in the hardware tone queue) may finish playing. The response reports `S` (stopped).

#### Play Hourly Chime

```
-> $TCCBC*XX\n           (play chime for current displayed hour)
-> $TCCBC<hh>*XX\n       (play chime for hour hh, 0–23)
<- $TCSBP*XX\n
```

Plays the hourly chime melody. With no argument, uses the current displayed hour — identical to the chime that fires automatically at the top of each hour. With an explicit `hh` argument (0–23), encodes that hour as the chime; the 12-hour display setting is applied when active (for example, `0` → 12, `13` → 1).

The chime encodes the hour as a binary sequence of beeps (LSb first): a low note for a 0-bit and a high note for a 1-bit, with 16th-note rests between notes. Hour 0 produces a single low beep.

If playback is already in progress, it is interrupted and the chime starts immediately. The response reports current buzzer playback status (same as [Query Status](#query-status)).

#### Query Status

```
-> $TCCBQ*XX\n
<- $TCSB<status>*XX\n
```

Returns the current playback status: `P` (playing) or `S` (stopped).

#### RTTTL Playback Complete Notification

```
<- $TCSBOK*XX\n
```

Sent as an unsolicited notification when RTTTL playback completes. This fires when the last note has been accepted into the hardware tone queue; the final note(s) may still be audible for a short time after the notification is received.

---

### I -- Intensity

Get or set the nixie tube display brightness. The status LED has its own independent intensity and auto-brightness flag, both controlled via the [L -- Status LED](#l----status-led) command.

#### Get Intensity

```
-> $TCCI*XX\n
<- $TCSI<intensity>,<auto>*XX\n
```

Returns two comma-separated values:
- **intensity**: Current brightness level (0-255)
- **auto**: Auto-adjust enabled (`1`) or disabled (`0`)

Example: `$TCSI128,1*XX\n` = intensity 128, auto-adjust on.

#### Set Intensity

```
-> $TCCI<nnn>*XX\n
<- $TCSI<intensity>,<auto>*XX\n
```

Sets the display intensity to `nnn` (0-255). The response confirms the new state.

Example: `$TCCI200*XX\n` sets intensity to 200.

**Note:** Setting intensity with this command disables automatic brightness adjustment (`auto` will be `0` in the response). Use `$TCCIA1*XX\n` to re-enable it.

#### Set Auto-Adjust

```
-> $TCCIA<0|1>*XX\n
<- $TCSI<intensity>,<auto>*XX\n
```

Enables (`1`) or disables (`0`) automatic intensity adjustment based on ambient light.

Example: `$TCCIA1*XX\n` enables auto-adjust.

**Note:** `$TCCIA` controls tube auto-adjust only. For LED auto-adjust, use `$TCCLA`. When tube auto-adjust is active and the intensity changes automatically, an unsolicited `$TCSI` notification is sent. See [Intensity Change](#intensity-change) in the Unsolicited Notifications section.

---

### S -- Settings

Get or set individual clock settings by index number.

#### Get Setting

```
-> $TCCS<nn>*XX\n
<- $TCSS<nn>,<value>*XX\n
```

Returns the current value of setting `nn`.

Example: `$TCCS10*XX\n` queries setting 10 (FadeDuration); response: `$TCSS10,500*XX\n`.

#### Set Setting

```
-> $TCCS<nn>,<value>*XX\n
<- $TCSS<nn>,<value>*XX\n
```

Sets setting `nn` to `value` (unsigned 16-bit integer). The response confirms the new value. After setting a value, the clock's active settings are refreshed immediately.

Example: `$TCCS20,5*XX\n` sets BeeperVolume (20) to 5.

**Note:** Set Setting updates the in-memory settings only. Use Save Settings to persist changes across power cycles.

#### Save Settings

```
-> $TCCSW*XX\n
<- $TCSSW<result>*XX\n
```

Saves the current in-memory settings to flash. To prevent unnecessary flash wear, the write is skipped if the in-memory settings are identical to the copy already stored in flash (compared by CRC).

- **result**: `1` = saved successfully (or already up to date), `0` = flash write error

Example: `$TCSSW1*XX\n` — settings saved (or unchanged).

#### Erase Settings (Factory Reset)

```
-> $TCCSERASE*XX\n
<- $TCSSERASE<result>*XX\n
```

Erases the settings flash area, removing all user preferences. On the next reboot the clock will load factory defaults.

- **result**: `1` = erased successfully, `0` = erase error

**Note:** The in-memory settings are not modified by this command. The clock continues running with its current in-memory settings until restarted. To apply factory defaults immediately, reset the clock.

Example: `$TCSSERASE1*XX\n` — settings flash erased.

#### Setting Index Reference

| Index | Setting | Description |
|-------|---------|-------------|
| 0 | SystemOptions | Bitfield of system option flags (see below) |
| 1 | BeepStates | Per-slot beep enable bitmask |
| 2 | BlinkStates | Per-slot blink enable bitmask |
| 3 | OnOffStates | Per-slot on/off enable bitmask |
| 4 | PMIndicatorRedValue | PM indicator LED red channel (0–4095) |
| 5 | PMIndicatorGreenValue | PM indicator LED green channel (0–4095) |
| 6 | PMIndicatorBlueValue | PM indicator LED blue channel (0–4095) |
| 7 | TimeDisplayDuration | Duration of time display (toggle mode) |
| 8 | DateDisplayDuration | Duration of date display (toggle mode) |
| 9 | TemperatureDisplayDuration | Duration of temperature display (toggle mode) |
| 10 | FadeDuration | Digit crossfade duration |
| 11 | DstBeginMonth | DST start month |
| 12 | DstBeginDowOrdinal | DST start day-of-week ordinal |
| 13 | DstEndMonth | DST end month |
| 14 | DstEndDowOrdinal | DST end day-of-week ordinal |
| 15 | DstSwitchDayOfWeek | DST switch day of week |
| 16 | DstSwitchHour | DST switch hour |
| 17 | EffectDuration | Display effect duration |
| 18 | EffectFrequency | Display effect frequency |
| 19 | MinimumIntensity | Minimum display intensity |
| 20 | BeeperVolume | Beeper volume |
| 21 | TemperatureCalibrationSTM32 | Temperature calibration offset (STM32 internal ADC) |
| 22 | TemperatureCalibrationDS3234 | Temperature calibration offset (DS3234) |
| 23 | TemperatureCalibrationDS1722 | Temperature calibration offset (DS1722) |
| 24 | TemperatureCalibrationLM74 | Temperature calibration offset (LM74) |
| 25 | IdleTimeout | Idle timeout (seconds, 10–600) |
| 26 | DateFormat | Date format |
| 27 | TimeZone | Time zone offset |
| 28 | ColonBehavior | Colon/separator behavior during time display |
| 29 | TimerResetValue | Timer/counter reset value |
| 30 | DmxAddress | DMX-512 base address |

#### SystemOptions Bit Flags (Setting 0)

| Bit | Flag | Description |
|-----|------|-------------|
| 0 | Display12Hour | 12-hour time format |
| 1 | StatusLedAsAmPm | Use status LED for AM/PM indication |
| 2 | HourlyChime | Enable hourly chime |
| 3 | DstEnable | Enable automatic DST adjustment |
| 4 | DisplayFahrenheit | Display temperature in Fahrenheit |
| 5 | AutoAdjustIntensity | Enable auto brightness adjustment |
| 6 | StartupToToggle | Boot into toggle display mode |
| 7 | DmxExtended | DMX-512 extended mode |
| 8 | MSDsOff | Turn off most-significant digits when zero |
| 9 | TriggerEffectOnRotate | Trigger display effect on time/date/temperature toggling |

---

### A -- Alarm Slot Time

Get or set the time for alarm slots 1–8. These are the time-of-day triggers used by `AlarmHandler`. Only the time portion (HH, MM, SS) is stored; the date is ignored during alarm comparison.

#### Get Alarm Slot Time

```
-> $TCCA<n>*XX\n
<- $TCSA<n>,<HHMMSS>*XX\n
```

Returns the current alarm time for slot `n` (1–8) as 6 digits.

Example: `$TCCA1*XX\n` → `$TCSA1,013000*XX\n` (slot 1 set to 01:30:00)

#### Set Alarm Slot Time

```
-> $TCCA<n>,<HHMMSS>*XX\n
<- $TCSA<n>,<HHMMSS>*XX\n
```

Sets the alarm time for slot `n` (1–8). The response confirms the new value.

- `n`: Slot number, 1–8
- `HH`: Hours (00–23)
- `MM`: Minutes (00–59)
- `SS`: Seconds (00–59)

Invalid slot numbers or out-of-range time values produce no response.

Example: `$TCCA3,083000*XX\n` sets slot 3 to 08:30:00.

**Note:** This command updates in-memory settings only. Use `$TCCSSW*XX\n` to persist changes across power cycles. Whether each slot triggers a beep and/or display blink is controlled by the `BeepStates` (setting 1) and `BlinkStates` (setting 2) bitmasks.

---

### R -- Timer/Counter

Control the timer/counter (operating mode 3: `TimerCounter`). All commands except `RA` (clear alarm) automatically switch to `OperatingModeTimerCounter`.

#### Run Up

```
-> $TCCRU*XX\n
<- $TCSRU,<value>*XX\n
```

Sets count direction to up and starts the timer counting from its current value. The response includes the state character and current timer value (see Response Format below).

#### Run Down

```
-> $TCCRD*XX\n
<- $TCSRD,<value>*XX\n
```

Sets count direction to down and starts the timer counting from its current value.

#### Stop

```
-> $TCCRS*XX\n
<- $TCSRS,<value>*XX\n
```

Pauses the timer at its current value.

#### Reset

```
-> $TCCRR*XX\n
<- $TCSRR,<value>*XX\n
```

Reloads the timer from the `TimerResetValue` setting (index 29): if counting up, resets to 0; if counting down, resets to the setting value. Same behavior as pressing key C. The response state will be `R` briefly (timer is in reset sub-mode), then the view transitions to stopped.

#### Reset to N Seconds

```
-> $TCCRR<seconds>*XX\n
<- $TCSRS,<seconds>*XX\n
```

Loads an arbitrary value `<seconds>` into the timer (range 0–999999) and stops without changing the current count direction. The response state is `S` (stopped).

Example: `$TCCRR30*XX\n` loads 30 into the timer and stops.

#### Clear Alarm

```
-> $TCCRA*XX\n
<- $TCSRA<0|1>*XX\n
```

Clears any active timer/counter alarm. Does **not** change the operating mode. The response value is:
- `1` — alarm was active and has been cleared
- `0` — no alarm was active

#### Response Format

Timer command responses (U/D/S/R actions):

```
$TCSR<state>,<value>*XX\n
```

| Field | Description |
|-------|-------------|
| `<state>` | `U` = running up, `D` = running down, `S` = stopped, `R` = reset (transient) |
| `<value>` | Current timer value as a decimal integer (live at dequeue time) |

Examples:
```
$TCSRU,45*XX\n    (running up, timer at 45)
$TCSRD,30*XX\n    (running down, timer at 30)
$TCSRS,0*XX\n     (stopped at 0)
```

---

### D -- Diagnostics

Read-only queries for system health and identity. These commands return live state at the time the response is sent; none modify any clock state except `DOTR` (on-time reset).

#### Get Firmware Version

```
-> $TCCDF*XX\n
<- $TCSDF<version>,<build>*XX\n
```

Returns the firmware version string and build number. The response fields are identical to those in the `$TCSBOOT` notification sent at startup.

| Field | Type | Description |
|-------|------|-------------|
| `version` | string | Calendar-based version `"YY.MM.PP"` (always 8 characters, zero-padded) |
| `build` | decimal | Monotonically incrementing build counter (uint16) |

Example:
```
-> $TCCDF*5C\n
<- $TCSDF26.03.01,42*XX\n
```

#### Get HV On-Time (Tube Lifetime)

```
-> $TCCDOT*XX\n
<- $TCSDOT<seconds>*XX\n
```

Returns the total number of seconds the HV (high-voltage nixie tube supply) has been active. The counter persists across power cycles via the RTC backup register and DS3234 SRAM (if available). Can be reset with `$TCCDOTR`.

| Field | Type | Description |
|-------|------|-------------|
| `seconds` | decimal | Accumulated HV on-time in seconds (uint32) |

Example:
```
-> $TCCDOT*XX\n
<- $TCSDOT432000*XX\n   (120 hours)
```

#### Reset HV On-Time Counter

```
-> $TCCDOTR*XX\n
<- $TCSDOTR*XX\n
```

Resets the HV on-time counter to zero and acknowledges the reset. This is the same reset triggered by pressing C in the System Status view.

#### Get RTC Info

```
-> $TCCDRTC*XX\n
<- $TCSDRTC<type>,<startResult>*XX\n
```

Returns the active RTC source and the result of RTC initialization at startup.

| Field | Type | Description |
|-------|------|-------------|
| `type` | decimal | Active RTC: `0` = STM32 internal, `1` = DS323x external |
| `startResult` | decimal | Startup result: `0` = UnknownError, `1` = Ok, `2` = OscStopped (reinitialized), `3` = OscTimeout |

Example (DS3234 found, started normally):
```
-> $TCCDRTC*XX\n
<- $TCSDRTC1,1*XX\n
```

#### Get Settings Load Source

```
-> $TCCDS*XX\n
<- $TCSDS<source>*XX\n
```

Returns which source settings were loaded from at the last boot.

| Field | Type | Description |
|-------|------|-------------|
| `source` | decimal | `0` = Compiled-in defaults (no stored settings found), `1` = Flash, `2` = DS3234 SRAM |

Example:
```
-> $TCCDS*XX\n
<- $TCSDS1*XX\n   (loaded from flash)
```

#### Get GPS Status

```
-> $TCCDGPS*XX\n
<- $TCSDGPS<connected>,<valid>,<sats>*XX\n
```

Returns detailed GPS receiver status. `connected` indicates the module is responding (GPRMC sentences received); `valid` indicates an active position fix with a valid date/time. Note: this is a more granular view of the GPS data already partially exposed via `$TCCHCON`.

This response is also sent as an **unsolicited status notification** whenever any GPS status field changes. See [GPS Status Change](#gps-status-change) in the Unsolicited Notifications section.

| Field | Type | Description |
|-------|------|-------------|
| `connected` | `0`/`1` | GPS module is responding (valid or invalid fix character received) |
| `valid` | `0`/`1` | GPS has a valid fix and date/time (`GPRMC` status `A`) |
| `sats` | decimal | Number of satellites currently in view (uint8) |

Example (connected, fix acquired, 8 satellites):
```
-> $TCCDGPS*XX\n
<- $TCSDGPS1,1,8*XX\n
```

---

## Unsolicited Notifications

The clock sends these notifications automatically when state changes occur, without a preceding command. Notifications are buffered in a transmit queue and sent when the TX path becomes idle (after both USART output channels finish transmitting the previous message). Unsolicited notification types are coalesced — at most one pending entry per type is kept in the queue, so only the latest state is sent when the queue drains. Key events are an exception and are never coalesced; each press and release is individually queued. If the queue is full (capacity 8), new entries are silently dropped. The external MCU can always poll for current state if a notification is missed.

### Boot

```
<- $TCSBOOT<version>,<build>*XX\n
```

Sent once immediately after the clock finishes initialization, before any other unsolicited notifications. `<version>` is the firmware version string. Calendar-based versioning is used: `YY.MM.patch` with each field having two-digits, zero-padded when necessary. `<build>` is the monotonically incrementing build counter.

Example: `$TCSBOOT26.03.01,12*XX\n`

---

### Mode Change

```
<- $TCSP<nn>*XX\n
<- $TCSP<nn>P<m>*XX\n     (with non-zero view mode)
```

Sent when the operating mode or view mode changes (for example, user navigates menus, alarm triggers a mode switch).

### Key Event

```
<- $TCSK<bitmask>*XX\n
```

Sent whenever the set of pressed keys changes. **bitmask** is a decimal bitmask of **all currently pressed keys** (see Keys table above). A value of `0` means all keys have been released. The external MCU can compare successive bitmasks to determine which keys were pressed or released.

Example sequence for pressing U and E simultaneously, then releasing both:
```
<- $TCSK5*XX\n      (U=1 and E=4 both pressed, bitmask=5)
<- $TCSK0*XX\n      (all keys released)
```

Example sequence for pressing U, then also pressing E, then releasing U:
```
<- $TCSK1*XX\n      (U pressed)
<- $TCSK5*XX\n      (E also pressed; bitmask now 5 = U+E)
<- $TCSK4*XX\n      (U released; E still held, bitmask=4)
<- $TCSK0*XX\n      (E released)
```

### LED State Change

```
<- $TCSL<i>,<r>,<g>,<b>,<auto>*XX\n
```

Sent whenever the LED color, intensity, or auto-adjust state changes, whether from an external `$TCCL`/`$TCCLA` command, an internal view (for example, AM/PM indicator), or automatic brightness adjustment. Format is identical to the `L` command response. See the L command for field descriptions.

### Temperature Change

```
<- $TCSM<adc>,<ds3234>,<ds1722>,<lm74>,<external>*XX\n
```

Sent whenever a new temperature reading is obtained from hardware and at least one sensor value has changed. Format is identical to the `M` command response. See the M command for field descriptions.

### Intensity Change

```
<- $TCSI<intensity>,<auto>*XX\n
```

Sent when the display intensity changes due to automatic brightness adjustment. Not sent in response to an explicit `$TCCI` or `$TCCIA` command (the command response provides that confirmation). Format is identical to the `I` command response. See the I command for field descriptions.

### ADC Update

```
<- $TCSHADC<light>,<vdda>,<vbatt>*XX\n
```

Sent whenever new ADC readings are obtained and at least one value (light level, VddA, or VBatt) has changed. Format is identical to the `H/ADC` command response. See the H command for field descriptions.

### RTTTL Playback Complete

```
<- $TCSBOK*XX\n
```

Sent when RTTTL playback finishes. Fires when the last note has been accepted into the hardware tone queue; the final note(s) may still be audible briefly. This notification is suppressed if playback is stopped via the `$TCCBS` command.

### HV State Change

```
<- $TCSHV<0|1>*XX\n
```

Sent when the HV supply state is toggled via the IR remote (Play/Pause key). `1` = enabled, `0` = disabled. Format is identical to the `H/HV` command response. This notification is coalesced.

**Note:** This notification is not sent for HV state changes triggered by serial `$TCCHVON`/`$TCCHVOF` commands — those are confirmed by the command response.

---

### Setting Changed

```
<- $TCSS<nn>,<value>*XX\n
```

Sent whenever a setting is changed via the clock's local UI (capacitive touch keys/menu navigation). Format is identical to the `S` command response — see the [Setting Index Reference](#setting-index-reference) for field descriptions.

Unlike other unsolicited notifications, setting change events are **not coalesced**: each setting that changes produces its own notification. The value reflects the live setting state at the time the notification is transmitted. If the notification queue is full, entries are silently dropped (see [TX notification queue](#implementation-notes)).

**Note:** This notification is not sent for settings changed via the serial `$TCCS` command — those are confirmed by the command response.

### GPS Status Change

```
<- $TCSDGPS<connected>,<valid>,<sats>*XX\n
```

Sent whenever any GPS status field changes: `connected` (module responding), `valid` (fix active), or `sats` (satellite count). Format is identical to the `D/GPS` command response. See the D command for field descriptions. This notification is coalesced.

---

### Alarm Active

```
<- $TCSRALM*XX\n
```

Sent (unsolicited) whenever any alarm transitions from inactive to active (rising edge). This fires for the timer/counter alarm when the countdown reaches zero (count-down mode) or the target value (count-up mode). Only the rising edge is reported; no further notification is sent while the alarm remains active.

After receiving this notification, send `$TCCRA*XX\n` to clear the alarm.

---

## Implementation Notes

- **Interrupt-driven I/O**: USART3 and USART4 use interrupt-driven I/O (not DMA). All DMA channels are allocated to other peripherals.
- **Double-buffered RX**: The ISR fills one receive buffer while the main loop processes the other. This prevents data loss if a new sentence arrives while processing.
- **Shared TX buffer**: Both USART1 and USART4 transmit from the same buffer with independent per-USART progress tracking. USART1 and USART4 may complete transmission at different times.
- **GPS coexistence**: On USART1, both the GPS parser and the serial remote parser see every received byte independently. The GPS parser ignores `$TC` sentences and the serial remote parser ignores `$GP` sentences. No special multiplexing is needed.
- **Command processing**: Commands are processed in the main application loop (not in ISR context). One command is processed per loop iteration.
- **TX notification queue**: Command responses and unsolicited notifications share a transmit queue (capacity 8). Messages are held in the queue and sent when both USART output channels are idle. Unsolicited notification types are coalesced in the queue (at most one pending entry per type, so only the latest state is sent). Key events are not coalesced. New entries are silently dropped when the queue is full — an uncommon condition in normal use.
- **Payload size limit**: Maximum payload length between `$TC` header and `*` checksum marker is 255 bytes.
- **Response size limit**: Maximum total response size (including header, checksum, and newline) is 48 bytes.
