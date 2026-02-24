# Serial Remote Control API

This document describes the serial protocol used to monitor and control the TubeClock from an external microcontroller (e.g. ESP32, RP2040). The interface enables remote access to all major clock functions: display mode selection, time/date, temperature, intensity, settings, hardware status, and key events.

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

### Numeric Values

All numeric values in commands and responses are decimal ASCII. Leading zeros may be present in fixed-width fields (e.g. time: `093015` for 09:30:15). Negative values are prefixed with `-`.

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
| 13 | SetDurationClock | Clock display duration |
| 14 | SetDurationDate | Date display duration |
| 15 | SetDurationTemp | Temperature display duration |
| 16 | SetDurationFade | Digit fade duration |
| 17 | DstBeginMonth | DST start month |
| 18 | DstBeginDowOrdinal | DST start day ordinal |
| 19 | DstEndMonth | DST end month |
| 20 | DstEndDowOrdinal | DST end day ordinal |
| 21 | DstSwitchDayOfWeek | DST switch day of week |
| 22 | DstSwitchHour | DST switch hour |
| 23 | SetEffectDuration | Display effect duration |
| 24 | SetEffectFrequency | Display effect frequency |
| 25 | SetMinimumIntensity | Minimum display intensity |
| 26 | SetBeeperVolume | Beeper volume |
| 27 | SetTempCalibration | Temperature calibration |
| 28 | SetDisplayRefreshInterval | Display refresh interval |
| 29 | SetDateFormat | Date format |
| 30 | SetTimeZone | Time zone |
| 31 | SetColonBehavior | Colon/separator behavior |
| 32 | SetDMX512Address | DMX-512 address |
| 33-40 | Slot1Time-Slot8Time | Set slot 1-8 time |

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
- `YYYY` -- Year (e.g. 2026)
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

Example: `$TCSMS4*XX\n` (LM74 is the active source).

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

Get or set the RGB status LED color. Values are 0–255 per channel; internally the LED uses 12-bit resolution (0–4095), so the serial interface scales by a factor of 16 in each direction.

#### Get LED Color

```
-> $TCCL*XX\n
<- $TCSL<r>,<g>,<b>*XX\n
```

Returns the current red, green, and blue channel values (0–255 each).

Example: `$TCSL0,0,0*XX\n` (LED off).

#### Set LED Color

```
-> $TCCL<r>,<g>,<b>*XX\n
<- $TCSL<r>,<g>,<b>*XX\n
```

Sets the LED to the specified color. The response confirms the new state.

Example: `$TCCL255,128,0*XX\n` sets the LED to orange.

**Note:** The LED color set via this command persists until a view that manages the LED internally changes it (e.g. a PM indicator transition in `TimeDateTempView`, or entering `MainMenuView`). The clock minimizes unnecessary LED writes, so the externally set color is preserved as long as the active view's own desired LED color does not change.

This response is also sent as an **unsolicited status notification** whenever the LED color changes from any source (internal views or external commands).

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

Get or set the display brightness.

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

#### Set Auto-Adjust

```
-> $TCCIA<0|1>*XX\n
<- $TCSI<intensity>,<auto>*XX\n
```

Enables (`1`) or disables (`0`) automatic intensity adjustment based on ambient light.

Example: `$TCCIA1*XX\n` enables auto-adjust.

**Note:** When auto-adjust is active and the intensity changes automatically, an unsolicited `$TCSI` notification is sent. See [Intensity Change](#intensity-change) in the Unsolicited Notifications section.

---

### S -- Settings

Get or set individual clock settings by index number.

#### Get Setting

```
-> $TCCS<nn>*XX\n
<- $TCSS<nn>,<value>*XX\n
```

Returns the current value of setting `nn`.

Example: `$TCCS7*XX\n` queries setting 7 (FadeDuration); response: `$TCSS7,500*XX\n`.

#### Set Setting

```
-> $TCCS<nn>,<value>*XX\n
<- $TCSS<nn>,<value>*XX\n
```

Sets setting `nn` to `value` (unsigned 16-bit integer). The response confirms the new value. After setting a value, the clock's active settings are refreshed immediately.

Example: `$TCCS17,128*XX\n` sets BeeperVolume (17) to 128.

**Note:** Set Setting updates the in-memory settings only. Use Save Settings to persist changes across power cycles.

#### Save Settings

```
-> $TCCSW*XX\n
<- $TCSSW<result>*XX\n
```

Saves the current in-memory settings to flash. To prevent unnecessary flash wear, the write is skipped if the in-memory settings are identical to the copy already stored in flash (compared by CRC).

- **result**: `1` = saved successfully (or already up to date), `0` = flash write error

Example: `$TCSSW1*XX\n` — settings saved (or unchanged).

#### Setting Index Reference

| Index | Setting | Description |
|-------|---------|-------------|
| 0 | SystemOptions | Bitfield of system option flags (see below) |
| 1 | BeepStates | Per-slot beep enable bitmask |
| 2 | BlinkStates | Per-slot blink enable bitmask |
| 3 | OnOffStates | Per-slot on/off enable bitmask |
| 4 | TimeDisplayDuration | Duration of time display (toggle mode) |
| 5 | DateDisplayDuration | Duration of date display (toggle mode) |
| 6 | TemperatureDisplayDuration | Duration of temperature display (toggle mode) |
| 7 | FadeDuration | Digit crossfade duration |
| 8 | DstBeginMonth | DST start month |
| 9 | DstBeginDowOrdinal | DST start day-of-week ordinal |
| 10 | DstEndMonth | DST end month |
| 11 | DstEndDowOrdinal | DST end day-of-week ordinal |
| 12 | DstSwitchDayOfWeek | DST switch day of week |
| 13 | DstSwitchHour | DST switch hour |
| 14 | EffectDuration | Display effect duration |
| 15 | EffectFrequency | Display effect frequency |
| 16 | MinimumIntensity | Minimum display intensity |
| 17 | BeeperVolume | Beeper volume |
| 18 | TemperatureCalibrationSTM32 | Temperature calibration offset (STM32 internal ADC) |
| 19 | TemperatureCalibrationDS3234 | Temperature calibration offset (DS3234) |
| 20 | TemperatureCalibrationDS1722 | Temperature calibration offset (DS1722) |
| 21 | TemperatureCalibrationLM74 | Temperature calibration offset (LM74) |
| 22 | DisplayRefreshInterval | Display refresh interval |
| 23 | DateFormat | Date format |
| 24 | TimeZone | Time zone offset |
| 25 | ColonBehavior | Colon/separator behavior during time display |
| 26 | TimerResetValue | Timer/counter reset value |
| 27 | DmxAddress | DMX-512 base address |

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

## Unsolicited Notifications

The clock sends these notifications automatically when state changes occur, without a preceding command. Notifications are buffered in a transmit queue and sent when the TX path becomes idle (after both USART output channels finish transmitting the previous message). Unsolicited notification types are coalesced — at most one pending entry per type is kept in the queue, so only the latest state is sent when the queue drains. Key events are an exception and are never coalesced; each press and release is individually queued. If the queue is full (capacity 8), new entries are silently dropped. The external MCU can always poll for current state if a notification is missed.

### Boot

```
<- $TCSBOOT<version>*XX\n
```

Sent once immediately after the clock finishes initialization, before any other unsolicited notifications. The `<version>` field is the firmware version string in `M.m.BB` format (major, minor, two-digit zero-padded build number).

Example: `$TCSBOOT1.0.05*XX\n`

---

### Mode Change

```
<- $TCSP<nn>*XX\n
<- $TCSP<nn>P<m>*XX\n     (with non-zero view mode)
```

Sent when the operating mode or view mode changes (e.g. user navigates menus, alarm triggers a mode switch).

### Key Event

```
<- $TCSK<bitmask>,<state>*XX\n
```

Sent on key press and release:
- **bitmask**: Decimal bitmask of the key (see Keys table above)
- **state**: `1` for pressed, `0` for released

Example sequence for pressing and releasing the Enter key:
```
<- $TCSK4,1*XX\n      (Enter pressed)
<- $TCSK4,0*XX\n      (Enter released)
```

### LED Color Change

```
<- $TCSL<r>,<g>,<b>*XX\n
```

Sent whenever the LED color changes, whether from an external `$TCCL` command or from an internal view (e.g. AM/PM indicator). Values are 0–255 per channel.

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
