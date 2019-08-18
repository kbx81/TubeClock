//
// kbx81's tube clock main application
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
#include "Display.h"
#include "Settings.h"
#include "View.h"


namespace kbxTubeClock {

namespace Application {

  /// @brief Application operating modes
  ///
  enum OperatingMode : uint8_t
  {
    OperatingModeMainMenu,            ///< Main Menu mode
    OperatingModeFixedDisplay,        ///< Fixed display mode
    OperatingModeToggleDisplay,       ///< Toggling display mode
    OperatingModeTimerCounter,        ///< Timer/counter mode
    OperatingModeDmx512Display,       ///< DMX-512 display mode
    OperatingModeSetClock,            ///< Set timeclock mode
    OperatingModeSetDate,             ///< Set date mode
    OperatingModeSetTimerResetValue,  ///< Set timer/counter reset value
    OperatingModeSystemStatusView,    ///< System status view mode
    OperatingModeSetSystemOptions,    ///< Set Binary/BCD and 24 hour bits mode
    OperatingModeSlotBeepConfig,      ///< Enable/Disable beeping (alarm) per-slot
    OperatingModeSlotBlinkConfig,     ///< Enable/Disable display blinking per-slot
    OperatingModeSlotOnOffConfig,     ///< Enable/Disable color change per-slot
    OperatingModeSetDurationClock,    ///< Set duration of clock display mode
    OperatingModeSetDurationDate,     ///< Set duration of date display mode
    OperatingModeSetDurationTemp,     ///< Set duration of temperature display mode
    OperatingModeSetDurationFade,     ///< Set duration of fading for digits
    OperatingModeDstBeginMonth,       ///< Set first month of DST
    OperatingModeDstBeginDowOrdinal,  ///< Set which given day DST begins
    OperatingModeDstEndMonth,         ///< Set last month of DST
    OperatingModeDstEndDowOrdinal,    ///< Set which given day DST ends
    OperatingModeDstSwitchDayOfWeek,  ///< Set which day of week DST begins/ends
    OperatingModeDstSwitchHour,       ///< Set hour at which DST begins/ends
    OperatingModeSetEffectDuration,   ///< Set duration of display effects
    OperatingModeSetEffectFrequency,  ///< Set frequency of display effects
    OperatingModeSetMinimumIntensity, ///< Set minimum display intensity
    OperatingModeSetBeeperVolume,     ///< Set Beeper Volume
    OperatingModeSetTempCalibration,  ///< Set temperature calibration
    OperatingModeSetDisplayRefreshInterval, ///< Set display refresh interval
    OperatingModeSetDateFormat,       ///< Set Date Format
    OperatingModeSetTimeZone,         ///< Set Time Zone
    OperatingModeSetColonBehavior,    ///< Set Colon/Seperator behavior (time display)
    OperatingModeSetDMX512Address,    ///< Set DMX-512 address mode
    OperatingModeSlot1Time,           ///< Set slot 1 time mode
    OperatingModeSlot2Time,           ///< Set slot 2 time mode
    OperatingModeSlot3Time,           ///< Set slot 3 time mode
    OperatingModeSlot4Time,           ///< Set slot 4 time mode
    OperatingModeSlot5Time,           ///< Set slot 5 time mode
    OperatingModeSlot6Time,           ///< Set slot 6 time mode
    OperatingModeSlot7Time,           ///< Set slot 7 time mode
    OperatingModeSlot8Time,           ///< Set slot 8 time mode
    OperatingModeTestDisplay          ///< Test display mode
  };

  /// @brief Enums for views
  ///
  enum ViewEnum : uint8_t
  {
      MainMenuViewEnum = 0,
      TimeDateTempViewEnum = 1,
      TimerCounterViewEnum = 2,
      Dmx512ViewEnum = 3,
      SetTimeDateViewEnum = 4,
      SetBitsViewEnum = 5,
      SetValueViewEnum = 6,
      SystemStatusViewEnum = 7,
      TestDisplayEnum = 8
  };

  /// @brief Enums for views
  ///
  enum ExternalControl : uint8_t
  {
      NoActiveExtControlEnum,
      SerialExtControlEnum,
      Dmx512ExtControlEnum
  };

  /// @brief Enums for DST computation
  ///
  enum DstState : uint8_t
  {
    Reset,
    Spring,
    Fall
  };


  /// @brief Number of seconds in a day
  ///
  const uint16_t cSecondsInAnHour = 60 * 60;
  const uint32_t cSecondsInADay = cSecondsInAnHour * 24;

  /// @brief Common colors used throughout
  ///
  static const RgbLed
    red(RgbLed::cLedMaxIntensity, 0, 0),
    orange(RgbLed::cLedMaxIntensity, RgbLed::cLedMaxIntensity / 4, 0),
    yellow(RgbLed::cLedMaxIntensity, RgbLed::cLedMaxIntensity, 0),
    green(0, RgbLed::cLedMaxIntensity, 0),
    cyan(0, RgbLed::cLedMaxIntensity, RgbLed::cLedMaxIntensity),
    blue(0, 0, RgbLed::cLedMaxIntensity),
    violet(RgbLed::cLedMaxIntensity / 8, 0, RgbLed::cLedMaxIntensity),
    magenta(RgbLed::cLedMaxIntensity, 0, RgbLed::cLedMaxIntensity),
    white(RgbLed::cLedMaxIntensity, RgbLed::cLedMaxIntensity, RgbLed::cLedMaxIntensity),
    gray(RgbLed::cLedMaxIntensity / 8, RgbLed::cLedMaxIntensity / 8, RgbLed::cLedMaxIntensity / 8),
    darkGray(RgbLed::cLedMaxIntensity / 24, RgbLed::cLedMaxIntensity / 24, RgbLed::cLedMaxIntensity / 24),
    nixieOrange(256, 32, 2);

  /// @brief Initialize the application
  ///
  void initialize();

  /// @brief Gets the application's date & time, adjusted for time zone and DST
  ///
  DateTime dateTime();

  /// @brief Set the date/time
  ///
  void setDateTime(const DateTime &now);

  /// @brief Returns the temperature based on hardware
  ///  (in fahrenheit if fahrenheit == true; in BCD if bcd == true)
  int32_t  temperature(const bool fahrenheit, const bool bcd = false);

  /// @brief Get a mode's display number
  /// @param mode Desired mode to get the display number for
  ///
  uint8_t getModeDisplayNumber(OperatingMode mode);
  uint8_t getModeDisplayNumber(uint8_t mode);

  /// @brief Get application mode
  ///
  OperatingMode getOperatingMode();

  /// @brief Set new application mode
  /// @param mode New application mode
  ///
  void setOperatingMode(OperatingMode mode);

  /// @brief Get view mode
  ///
  ViewMode getViewMode();

  /// @brief Set new view mode
  /// @param mode New view mode
  ///
  void setViewMode(ViewMode mode);

  /// @brief Get view's related setting
  /// @param mode Desired mode to get the related setting for
  ///
  uint8_t getOperatingModeRelatedSetting(OperatingMode mode);

  /// @brief Get external control status
  ///
  ExternalControl getExternalControlState();

  /// @brief Get application settings
  ///
  Settings getSettings();
  Settings* getSettingsPtr();

  /// @brief Refreshes hardware from current application settings
  ///
  void refreshSettings();

  /// @brief Set new application settings, also calls refreshSettings()
  /// @param settings New settings to apply to the applications and hardware
  ///
  void setSettings(Settings settings);

  /// @brief Handles DST date/time computation; maintains clock's DST state machine.
  ///  Intended for tracking DST clock adjustments, not for arbitrary use!
  ///  To reset state machine, call with year != year provided when last called.
  /// @param currentTime DateTime object to compute DST state for
  /// @return true if DST is active based on passed DateTime object
  bool isDst(const DateTime &currentTime);

  /// @brief Returns state of automatic display intensity adjustments
  /// @return True if automatic intensity adjustment is enabled
  bool getIntensityAutoAdjust();

  /// @brief Enables/disables automagic adjusting of the display intensity based
  ///  on ambient light seen by the phototransistor
  /// @param enable Enables automatic adjustments if true
  /// @param quickAdjust Perform an immediate update of the intensity
  void setIntensityAutoAdjust(const bool enable, const bool quickAdjust = false);

  /// @brief Returns the application's current display intensity
  /// @return current display intensity percentage application is using (see RgbLed)
  uint16_t getIntensity();

  /// @brief Set the application's display intensity and disables automagic adjusting of the display intensity
  /// @param intensity New display intensity percentage application should use (see RgbLed)
  ///
  void setIntensity(const uint16_t intensity);

  /// @brief Refreshs master display intensity. Call at fixed intervals.
  ///
  void tick();

  /// @brief The main loop of the application.
  ///
  __attribute__((noreturn))
  void loop();
}

}
