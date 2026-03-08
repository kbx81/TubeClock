//
// kbx81's tube clock settings class
// ---------------------------------------------------------------------------
// (c)2019 by kbx81. See LICENSE for details.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>
//
// ---------------------------------------------------------------------------
// Settings Profiles:
// Default settings are organized into profiles that can be selected at build
// time using the SETTINGS_PROFILE macro (set via Makefile PROFILE variable).
//
// To build with a specific profile:
//   make PROFILE=0  (Standard - US/North America defaults)
//   make PROFILE=1  (European - 24hr, Celsius)
//   make PROFILE=2  (Minimal - Silent, no effects)
//   make PROFILE=3  (Kbx - kbx's preferred settings)
//
// To add a new profile, create a new SettingsProfile struct below and add it
// to the cSettingsProfiles array. The compiler will validate the PROFILE value
// at build time and show an error if it's out of range.
// ---------------------------------------------------------------------------
//
#include <cstring>

#include "Application.h"
#include "DateTime.h"
#include "DS3234.h"
#include "Hardware.h"
#include "Settings.h"

// Linker-provided symbol for the settings flash address (must be outside namespace)
extern "C" uint32_t __settings_flash_start;

static_assert(sizeof(kbxTubeClock::Settings) <= kbxTubeClock::Hardware::cOnTimeCounterSramOffset,
              "Settings struct overlaps DS3234 SRAM on-time counter slot; update cOnTimeCounterSramOffset");

namespace kbxTubeClock {

// Default settings profiles
// Each profile contains default values for all 28 settings
// Profile selection is controlled by SETTINGS_PROFILE macro (set in Makefile)
// To add a new profile, add a new #elif block below and update the error message.

struct SettingsProfile {
  const uint16_t settings[Settings::Setting::DmxAddress + 1];
};

#ifndef SETTINGS_PROFILE
#define SETTINGS_PROFILE 0
#endif

#if SETTINGS_PROFILE == 0
// Profile 0: Standard US/North America defaults (DST enabled, 12-hour, Fahrenheit)
static const SettingsProfile cActiveProfile = {{
    0b1011111111,   // SystemOptions: 12hr, status LED as AM/PM, hourly chime, DST, Fahrenheit, auto intensity
    0b11100000000,  // BeepStates
    0b11100000000,  // BlinkStates
    0b11111111,     // OnOffStates
    1280,           // PMIndicatorRedValue
    496,            // PMIndicatorGreenValue
    112,            // PMIndicatorBlueValue
    26,             // TimeDisplayDuration
    2,              // DateDisplayDuration
    2,              // TemperatureDisplayDuration
    100,            // FadeDuration
    3,              // DstBeginMonth (March)
    2,              // DstBeginDowOrdinal (2nd occurrence)
    11,             // DstEndMonth (November)
    1,              // DstEndDowOrdinal (1st occurrence)
    0,              // DstSwitchDayOfWeek (Sunday)
    2,              // DstSwitchHour (2 AM)
    500,            // EffectDuration
    300,            // EffectFrequency
    8,              // MinimumIntensity
    3,              // BeeperVolume
    4,              // TemperatureCalibrationSTM32 (midpoint 99; 9 = -9.5C)
    79,             // TemperatureCalibrationDS3234 (79 = -2.0C)
    79,             // TemperatureCalibrationDS1722 (79 = -2.0C)
    79,             // TemperatureCalibrationLM74 (79 = -2.0C)
    120,            // IdleTimeout (120 seconds)
    2,              // DateFormat (MM.DD.YY)
    32,             // TimeZone
    0,              // ColonBehavior
    60,             // TimerResetValue
    0               // DmxAddress
}};
#elif SETTINGS_PROFILE == 1
// Profile 1: European defaults (24-hour, Celsius, no DST handling)
static const SettingsProfile cActiveProfile = {{
    0b1011101100,   // SystemOptions: 24hr, Celsius, auto intensity
    0b11100000000,  // BeepStates
    0b11100000000,  // BlinkStates
    0b11111111,     // OnOffStates
    1280,           // PMIndicatorRedValue
    496,            // PMIndicatorGreenValue
    112,            // PMIndicatorBlueValue
    26,             // TimeDisplayDuration
    2,              // DateDisplayDuration
    2,              // TemperatureDisplayDuration
    100,            // FadeDuration
    3,              // DstBeginMonth (not used)
    2,              // DstBeginDowOrdinal
    11,             // DstEndMonth (not used)
    1,              // DstEndDowOrdinal
    0,              // DstSwitchDayOfWeek
    2,              // DstSwitchHour
    500,            // EffectDuration
    300,            // EffectFrequency
    8,              // MinimumIntensity
    3,              // BeeperVolume
    4,              // TemperatureCalibrationSTM32 (midpoint 99; 9 = -9.5C)
    79,             // TemperatureCalibrationDS3234 (79 = -2.0C)
    79,             // TemperatureCalibrationDS1722 (79 = -2.0C)
    79,             // TemperatureCalibrationLM74 (79 = -2.0C)
    120,            // IdleTimeout (120 seconds)
    1,              // DateFormat (DD.MM.YY)
    56,             // TimeZone
    0,              // ColonBehavior
    60,             // TimerResetValue
    0               // DmxAddress
}};
#elif SETTINGS_PROFILE == 2
// Profile 2: Minimal/Silent (no beeps, no effects, basic display)
static const SettingsProfile cActiveProfile = {{
    0b0000100000,   // SystemOptions: 24hr, auto intensity only
    0b00000000000,  // BeepStates: all disabled
    0b00000000000,  // BlinkStates: all disabled
    0b11111111,     // OnOffStates
    1280,           // PMIndicatorRedValue
    496,            // PMIndicatorGreenValue
    112,            // PMIndicatorBlueValue
    26,             // TimeDisplayDuration
    2,              // DateDisplayDuration
    2,              // TemperatureDisplayDuration
    50,             // FadeDuration (faster)
    3,              // DstBeginMonth
    2,              // DstBeginDowOrdinal
    11,             // DstEndMonth
    1,              // DstEndDowOrdinal
    0,              // DstSwitchDayOfWeek
    2,              // DstSwitchHour
    0,              // EffectDuration (effects disabled)
    0,              // EffectFrequency (effects disabled)
    8,              // MinimumIntensity
    0,              // BeeperVolume (silent)
    4,              // TemperatureCalibrationSTM32 (midpoint 99; 9 = -9.5C)
    79,             // TemperatureCalibrationDS3234 (79 = -2.0C)
    79,             // TemperatureCalibrationDS1722 (79 = -2.0C)
    79,             // TemperatureCalibrationLM74 (79 = -2.0C)
    120,            // IdleTimeout (120 seconds)
    0,              // DateFormat (YY.MM.DD)
    56,             // TimeZone
    0,              // ColonBehavior
    60,             // TimerResetValue
    0               // DmxAddress
}};
#elif SETTINGS_PROFILE == 3
// Profile 3: kbx's preferred defaults (DST enabled, 12-hour, Fahrenheit)
static const SettingsProfile cActiveProfile = {{
    0b1011111111,   // SystemOptions: 12hr, status LED as AM/PM, hourly chime, DST, Fahrenheit, auto intensity
    0b11100000000,  // BeepStates
    0b11100000000,  // BlinkStates
    0b11111111,     // OnOffStates
    1280,           // PMIndicatorRedValue
    496,            // PMIndicatorGreenValue
    112,            // PMIndicatorBlueValue
    26,             // TimeDisplayDuration
    2,              // DateDisplayDuration
    2,              // TemperatureDisplayDuration
    250,            // FadeDuration
    3,              // DstBeginMonth (March)
    2,              // DstBeginDowOrdinal (2nd occurrence)
    11,             // DstEndMonth (November)
    1,              // DstEndDowOrdinal (1st occurrence)
    0,              // DstSwitchDayOfWeek (Sunday)
    2,              // DstSwitchHour (2 AM)
    500,            // EffectDuration
    300,            // EffectFrequency
    8,              // MinimumIntensity
    2,              // BeeperVolume
    4,              // TemperatureCalibrationSTM32 (midpoint 99; 9 = -9.5C)
    79,             // TemperatureCalibrationDS3234 (79 = -2.0C)
    79,             // TemperatureCalibrationDS1722 (79 = -2.0C)
    79,             // TemperatureCalibrationLM74 (79 = -2.0C)
    120,            // IdleTimeout (120 seconds)
    0,              // DateFormat (YY.MM.DD)
    32,             // TimeZone
    0,              // ColonBehavior
    60,             // TimerResetValue
    0               // DmxAddress
}};
#else
#error "SETTINGS_PROFILE is out of range; update Settings.cpp to add a new profile"
#endif

// where settings will be read/written in FLASH
const uint32_t Settings::cSettingsFlashAddress = reinterpret_cast<uint32_t>(&__settings_flash_start);

// Descriptor for each setting: max/mask, min value, display transform
const Settings::SettingDescriptor Settings::cSettingDescriptors[] = {
    {0x03ff, 0, Settings::SettingTransform::None},   // SystemOptions
    {0x07ff, 0, Settings::SettingTransform::None},   // BeepStates
    {0x07ff, 0, Settings::SettingTransform::None},   // BlinkStates
    {0x00ff, 0, Settings::SettingTransform::None},   // OnOffStates
    {4095,   0, Settings::SettingTransform::None},   // PMIndicatorRedValue
    {4095,   0, Settings::SettingTransform::None},   // PMIndicatorGreenValue
    {4095,   0, Settings::SettingTransform::None},   // PMIndicatorBlueValue
    {300,    0, Settings::SettingTransform::None},   // TimeDisplayDuration
    {300,    0, Settings::SettingTransform::None},   // DateDisplayDuration
    {300,    0, Settings::SettingTransform::None},   // TemperatureDisplayDuration
    {1000,   0, Settings::SettingTransform::None},   // FadeDuration
    {12,     1, Settings::SettingTransform::None},   // DstBeginMonth (1-12)
    {4,      1, Settings::SettingTransform::None},   // DstBeginDowOrdinal (1-4)
    {12,     1, Settings::SettingTransform::None},   // DstEndMonth (1-12)
    {4,      1, Settings::SettingTransform::None},   // DstEndDowOrdinal (1-4)
    {6,      0, Settings::SettingTransform::None},   // DstSwitchDayOfWeek
    {23,     0, Settings::SettingTransform::None},   // DstSwitchHour
    {1000,   0, Settings::SettingTransform::None},   // EffectDuration
    {43200,  0, Settings::SettingTransform::None},   // EffectFrequency
    {1000,   0, Settings::SettingTransform::None},   // MinimumIntensity
    {7,      0, Settings::SettingTransform::None},   // BeeperVolume
    {198,    0, Settings::SettingTransform::Calibration},  // TemperatureCalibrationSTM32 (midpoint 99)
    {198,    0, Settings::SettingTransform::Calibration},  // TemperatureCalibrationDS3234
    {198,    0, Settings::SettingTransform::Calibration},  // TemperatureCalibrationDS1722
    {198,    0, Settings::SettingTransform::Calibration},  // TemperatureCalibrationLM74
    {600,   10, Settings::SettingTransform::None},   // IdleTimeout (10-600 seconds)
    {2,      0, Settings::SettingTransform::None},   // DateFormat
    {112,    0, Settings::SettingTransform::TimeZone},     // TimeZone (0-112, x15 min, displayed ±HH:MM)
    {5,      0, Settings::SettingTransform::None},   // ColonBehavior
    {65535,  0, Settings::SettingTransform::None},   // TimerResetValue
    {512-8,  0, Settings::SettingTransform::PlusOne},      // DmxAddress (stored 0-504, shown 1-512)
};

Settings::Settings() : _crc(0) { initialize(); }

void Settings::initialize() {
  // set default time slots every three hours starting at 0100 hours
  for (uint8_t i = static_cast<uint8_t>(Slot::Slot1); i <= static_cast<uint8_t>(Slot::Slot8); i++) {
    _time[i].setTime((3 * i) + 1, 30, 0);
  }

  // Load settings from the active profile selected at build time
  memcpy(_setting, cActiveProfile.settings, sizeof(_setting));
}

bool Settings::loadFromFlash() {
  Hardware::readFlash(cSettingsFlashAddress, sizeof(Settings), (uint8_t *) this);

  uint32_t loadedCrc = _crc;  // save before zeroing for the CRC computation

  _crc = 0;  // always compute the CRC with this as zero

  if (loadedCrc != Hardware::getCRC((uint32_t *) this, sizeof(Settings) / 4)) {
    initialize();

    return false;
  }
  _crc = loadedCrc;  // restore so struct is identical to what was stored
  return true;
}

uint32_t Settings::saveToFlash() {
  _crc = 0;  // always compute the CRC with this as zero

  // compute the new CRC value and save it before we write it all to FLASH
  _crc = Hardware::getCRC((uint32_t *) this, sizeof(Settings) / 4);

  return Hardware::writeFlash(cSettingsFlashAddress, (uint8_t *) this, sizeof(Settings));
}

bool Settings::saveToFlashIfChanged() {
  // Compute what the CRC would be (same way saveToFlash does: _crc=0, then compute)
  uint32_t savedCrc = _crc;
  _crc = 0;
  uint32_t newCrc = Hardware::getCRC((uint32_t *) this, sizeof(Settings) / 4);
  _crc = savedCrc;

  // Read the stored CRC from flash (_crc is the last field in the Settings struct)
  uint32_t flashCrc = 0;
  Hardware::readFlash(cSettingsFlashAddress + (sizeof(Settings) - sizeof(uint32_t)), sizeof(uint32_t),
                      (uint8_t *) &flashCrc);

  if (newCrc == flashCrc) {
    return true;  // settings unchanged, skip the erase/write cycle
  }

  return saveToFlash() == 0;
}

bool Settings::loadFromSram() {
  if (!DS3234::isConnected()) {
    return false;
  }

  DS3234::readSram(0, reinterpret_cast<uint8_t *>(this), static_cast<uint8_t>(sizeof(Settings)));

  uint32_t loadedCrc = _crc;
  _crc = 0;

  if (loadedCrc != Hardware::getCRC((uint32_t *)this, sizeof(Settings) / 4)) {
    initialize();
    return false;
  }
  _crc = loadedCrc;  // restore so struct is identical to what was stored
  return true;
}

bool Settings::sramIsValid() const {
  if (!DS3234::isConnected()) {
    return false;
  }

  Settings temp;
  DS3234::readSram(0, reinterpret_cast<uint8_t *>(&temp), static_cast<uint8_t>(sizeof(Settings)));

  uint32_t loadedCrc = temp._crc;
  temp._crc = 0;
  return loadedCrc == Hardware::getCRC((uint32_t *)&temp, sizeof(Settings) / 4);
}

void Settings::saveToSram() {
  if (!DS3234::isConnected()) {
    return;
  }

  DS3234::writeSram(0, reinterpret_cast<uint8_t *>(this), static_cast<uint8_t>(sizeof(Settings)), true);
}

bool Settings::getSetting(const uint8_t setting, const uint8_t item) {
  if (setting <= static_cast<uint8_t>(Setting::DmxAddress)) {
    return (_setting[setting] & (1 << item));
  }
  return false;
}

void Settings::setSetting(const uint8_t setting, const uint8_t item, const bool state) {
  if (setting <= static_cast<uint8_t>(Setting::DmxAddress)) {
    if (state) {
      _setting[setting] |= (1 << item);
    } else {
      _setting[setting] &= ~(1 << item);
    }
  }
}

uint16_t Settings::getRawSetting(const uint8_t setting) {
  if (setting <= static_cast<uint8_t>(Setting::DmxAddress)) {
    return _setting[setting];
  }
  return 0;
}

void Settings::setRawSetting(const uint8_t setting, const uint16_t value) {
  if (setting <= static_cast<uint8_t>(Setting::DmxAddress)) {
    _setting[setting] = value;
  }
}

DateTime Settings::getTime(const Slot setting) { return getTime(static_cast<uint8_t>(setting)); }

DateTime Settings::getTime(const uint8_t setting) {
  if (setting <= static_cast<uint8_t>(Slot::Slot8)) {
    return _time[static_cast<uint8_t>(setting)];
  }
  return _time[static_cast<uint8_t>(Slot::Slot1)];
}

void Settings::setTime(const Slot setting, const DateTime &time) {
  return setTime(static_cast<uint8_t>(setting), time);
}

void Settings::setTime(const uint8_t setting, const DateTime &time) {
  if (setting <= static_cast<uint8_t>(Slot::Slot8)) {
    _time[static_cast<uint8_t>(setting)] = time;
  }
}

bool Settings::hvState() {
  DateTime currentTime = Application::dateTime();
  int32_t delta, currentSecondsSinceMidnight = (int32_t) currentTime.secondsSinceMidnight(false),
                 // minSecs = 0,  // first/earliest slot
      prevSecs = -Application::cSecondsInADay,  // slot most recently passed
      // nextSecs = Application::cSecondsInADay,  // next slot that will pass
      maxSecs = 0;   // latest/last slot
  uint8_t            // minSlot = 0,  // index of first/earliest slot
      prevSlot = 0,  // index of slot most recently passed
      // nextSlot = 0, // index of next slot that will pass
      maxSlot = 0;  // index of latest/last slot

  if (Hardware::rtcIsSet() == true) {
    // find minimum, maximum, just passed, and next to pass values
    for (uint8_t i = static_cast<uint8_t>(Slot::Slot1); i <= static_cast<uint8_t>(Slot::Slot8); i++) {
      delta = (int32_t) _time[i].secondsSinceMidnight(false) - currentSecondsSinceMidnight;

      // now, if delta is less than zero, slot i is before the current time;
      //   if delta is greater than zero, slot i is after the current time.
      // next we'll sort out which is just before and which is just after

      // if (((int32_t)_time[i].secondsSinceMidnight(false) < minSecs) || !i)
      // {
      //   minSecs = _time[i].secondsSinceMidnight(false);
      //   minSlot = i;
      // }

      if ((delta <= 0) && (delta > prevSecs)) {
        prevSecs = delta;
        prevSlot = i;
      }

      // if ((delta > 0) && (delta < nextSecs))
      // {
      //   nextSecs = delta;
      //   nextSlot = i;
      // }

      if ((int32_t) _time[i].secondsSinceMidnight(false) > maxSecs) {
        maxSecs = _time[i].secondsSinceMidnight(false);
        maxSlot = i;
      }
    }
    // if no slots have been passed yet, then the last one passed is the max/latest one
    if (prevSecs == (int32_t) -Application::cSecondsInADay) {
      prevSlot = maxSlot;
    }

    return getSetting(Setting::OnOffStates, prevSlot);
  } else {
    return true;
  }
}

}  // namespace kbxTubeClock
