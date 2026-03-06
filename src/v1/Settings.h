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
#pragma once

#include <cstdint>

#include "DateTime.h"

namespace kbxTubeClock {

class Settings {
 public:
  /// @brief Types of settings we keep
  ///
  enum Setting : uint8_t {
    SystemOptions = 0,
    BeepStates = 1,
    BlinkStates = 2,
    OnOffStates = 3,
    TimeDisplayDuration = 4,
    DateDisplayDuration = 5,
    TemperatureDisplayDuration = 6,
    FadeDuration = 7,
    DstBeginMonth = 8,
    DstBeginDowOrdinal = 9,
    DstEndMonth = 10,
    DstEndDowOrdinal = 11,
    DstSwitchDayOfWeek = 12,
    DstSwitchHour = 13,
    EffectDuration = 14,
    EffectFrequency = 15,
    MinimumIntensity = 16,
    BeeperVolume = 17,
    TemperatureCalibrationSTM32 = 18,
    TemperatureCalibrationDS3234 = 19,
    TemperatureCalibrationDS1722 = 20,
    TemperatureCalibrationLM74 = 21,
    DisplayRefreshInterval = 22,
    DateFormat = 23,
    TimeZone = 24,
    ColonBehavior = 25,
    TimerResetValue = 26,
    DmxAddress = 27
  };

  /// @brief Settings we keep
  ///
  enum SystemOptionsBits : uint8_t {
    Display12Hour = 0,
    StatusLedAsAmPm = 1,
    HourlyChime = 2,
    DstEnable = 3,
    DisplayFahrenheit = 4,
    AutoAdjustIntensity = 5,
    StartupToToggle = 6,
    DmxExtended = 7,
    MSDsOff = 8,
    TriggerEffectOnRotate = 9
  };

  /// @brief Enums for time slots
  ///
  enum Slot : uint8_t {
    Slot1 = 0,
    Slot2 = 1,
    Slot3 = 2,
    Slot4 = 3,
    Slot5 = 4,
    Slot6 = 5,
    Slot7 = 6,
    Slot8 = 7,
    SlotDate = 8,
    SlotTemperature = 9,
    SlotTimer = 10,
    SlotMenu = 11,
    SlotSet = 12,
    SlotDmx = 13,
    SlotCalculated = 14
  };

 public:
  /// @brief Where settings will be written in FLASH
  /// @note This address is provided by the linker script (__settings_flash_start)
  ///       and is located at the end of flash memory to prevent conflicts with code
  ///
  static const uint32_t cSettingsFlashAddress;

  /// @brief Bits used in each settings class
  ///
  static const uint16_t cSettingData[];

  /// @brief Midpoint value for signed temperature calibration encoding
  ///  stored = cCalibrationMidpoint + offsetCx10; offsetCx10 = stored - cCalibrationMidpoint
  static const int16_t cCalibrationMidpoint = 99;

  /// @brief Default constructor
  ///
  Settings();

  /// @brief Initialize settings
  ///
  void initialize();

  /// @brief Loads settings from FLASH
  /// @return true if success, false if failure & defaults were set
  ///
  bool loadFromFlash();

  /// @brief Saves settings to FLASH
  /// @return result of writeFlash()
  ///
  uint32_t saveToFlash();

  /// @brief Saves settings to FLASH only if they differ from the stored copy
  /// @return true on success (written or already up to date), false on write error
  ///
  bool saveToFlashIfChanged();

  /// @brief Returns a setting
  /// @return true if setting is enabled, false otherwise
  ///
  bool getSetting(const uint8_t setting, const uint8_t item);

  /// @brief Sets a setting
  ///
  void setSetting(const uint8_t setting, const uint8_t item, const bool state);

  /// @brief Returns all settings
  /// @return value of requested setting
  ///
  uint16_t getRawSetting(const uint8_t setting);

  /// @brief Sets all settings
  ///
  void setRawSetting(const uint8_t setting, const uint16_t value);

  /// @brief Get a time value stored in the settings
  ///
  DateTime getTime(const Slot setting);
  DateTime getTime(const uint8_t setting);

  /// @brief Set a time value in the settings
  ///
  void setTime(const Slot setting, const DateTime &time);
  void setTime(const uint8_t setting, const DateTime &time);

  /// @brief Returns the desired HV state based on the time
  /// @return true if HV should be on, false otherwise
  ///
  bool hvState();

 private:
  /// @brief array of values for various settings data
  ///
  uint16_t _setting[Setting::DmxAddress + 1];

  /// @brief array of stored times for the alarms and/or color changes
  ///
  DateTime _time[Slot::Slot8 + 1];

  /// @brief determines validity of Settings data after loading from FLASH
  ///
  uint32_t _crc;
};

}  // namespace kbxTubeClock
