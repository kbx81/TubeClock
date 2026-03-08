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
#include <cstdint>

#include "AlarmHandler.h"
#include "Animator.h"
#include "Application.h"
#include "DateTime.h"
#include "Display.h"
#include "DisplayManager.h"
#include "Dmx-512-Controller.h"
#include "Dmx-512-Rx.h"
#include "Dmx-512-View.h"
#include "GpsReceiver.h"
#include "Keys.h"
#include "MainMenuView.h"
#include "RtttlPlayer.h"
#include "SerialRemote.h"
#include "SetBitsView.h"
#include "SetTimeDateView.h"
#include "Settings.h"
#include "SetValueView.h"
#include "SystemStatusView.h"
#include "TestDisplayView.h"
#include "TimeDateTempView.h"
#include "TimerCounterView.h"
#include "UsbSerial.h"

// Workaround for  "hidden symbol `__dso_handle' isn't defined" linker error
void *__dso_handle;

namespace kbxTubeClock::Application {

// Color constant definitions (declared extern in Application.h)
const RgbLed red(RgbLed::cLedMaxIntensity, 0, 0), orange(RgbLed::cLedMaxIntensity, RgbLed::cLedMaxIntensity / 4, 0),
    yellow(RgbLed::cLedMaxIntensity, RgbLed::cLedMaxIntensity, 0), green(0, RgbLed::cLedMaxIntensity, 0),
    cyan(0, RgbLed::cLedMaxIntensity, RgbLed::cLedMaxIntensity), blue(0, 0, RgbLed::cLedMaxIntensity),
    violet(RgbLed::cLedMaxIntensity / 8, 0, RgbLed::cLedMaxIntensity),
    magenta(RgbLed::cLedMaxIntensity, 0, RgbLed::cLedMaxIntensity),
    white(RgbLed::cLedMaxIntensity, RgbLed::cLedMaxIntensity, RgbLed::cLedMaxIntensity),
    gray(RgbLed::cLedMaxIntensity / 8, RgbLed::cLedMaxIntensity / 8, RgbLed::cLedMaxIntensity / 8),
    darkGray(RgbLed::cLedMaxIntensity / 24, RgbLed::cLedMaxIntensity / 24, RgbLed::cLedMaxIntensity / 24),
    nixieOrange(1280, 496, 112);  // approximate orange color of neon-filled nixie tubes

// View instances — statically allocated, avoiding heap dependency
//
static MainMenuView _viewMainMenu;
static TimeDateTempView _viewTimeDateTemp;
static TimerCounterView _viewTimerCounter;
static Dmx512View _viewDmx512;
static SetTimeDateView _viewSetTimeDate;
static SetBitsView _viewSetBits;
static SetValueView _viewSetValue;
static SystemStatusView _viewSystemStatus;
static TestDisplayView _viewTestDisplay;

// An array with all views for the application; must correspond with ViewEnum!
//
static View *const cModeViews[] = {
    &_viewMainMenu, &_viewTimeDateTemp, &_viewTimerCounter, &_viewDmx512,      &_viewSetTimeDate,
    &_viewSetBits,  &_viewSetValue,     &_viewSystemStatus, &_viewTestDisplay,
};

// Structure defining menu views
//
struct viewDescriptor {
  uint8_t menuItemDisplayNumber;
  ViewEnum view;
  uint8_t relatedSettingNumber;
  uint8_t numSettings;
};

// An array of mode display numbers and their corresponding views
//
static viewDescriptor const cViewDescriptor[] = {
    {0, ViewEnum::MainMenuViewEnum, Settings::Setting::SystemOptions, 1},      // OperatingModeMainMenu
    {1, ViewEnum::TimeDateTempViewEnum, Settings::Setting::SystemOptions, 1},  // OperatingModeFixedDisplay
    {2, ViewEnum::TimeDateTempViewEnum, Settings::Setting::SystemOptions, 1},  // OperatingModeToggleDisplay
    {3, ViewEnum::TimerCounterViewEnum, Settings::Setting::SystemOptions, 1},  // OperatingModeTimerCounter
    {4, ViewEnum::Dmx512ViewEnum, Settings::Setting::SystemOptions, 1},        // OperatingModeDmx512Display
    {5, ViewEnum::SetTimeDateViewEnum, Settings::Setting::SystemOptions, 1},   // OperatingModeSetClock
    {6, ViewEnum::SetTimeDateViewEnum, Settings::Setting::SystemOptions, 1},   // OperatingModeSetDate
    {7, ViewEnum::SetValueViewEnum, Settings::Setting::TimerResetValue, 1},
    {8, ViewEnum::SystemStatusViewEnum, Settings::Setting::SystemOptions, 1},
    {10, ViewEnum::SetBitsViewEnum, Settings::Setting::SystemOptions, 1},
    {11, ViewEnum::SetBitsViewEnum, Settings::Setting::BeepStates, 1},
    {12, ViewEnum::SetBitsViewEnum, Settings::Setting::BlinkStates, 1},
    {13, ViewEnum::SetBitsViewEnum, Settings::Setting::OnOffStates, 1},
    {14, ViewEnum::SetValueViewEnum, Settings::Setting::PMIndicatorRedValue, 3},
    {20, ViewEnum::SetValueViewEnum, Settings::Setting::TimeDisplayDuration, 1},
    {21, ViewEnum::SetValueViewEnum, Settings::Setting::DateDisplayDuration, 1},
    {22, ViewEnum::SetValueViewEnum, Settings::Setting::TemperatureDisplayDuration, 1},
    {23, ViewEnum::SetValueViewEnum, Settings::Setting::FadeDuration, 1},
    {24, ViewEnum::SetValueViewEnum, Settings::Setting::DstBeginMonth, 1},
    {25, ViewEnum::SetValueViewEnum, Settings::Setting::DstBeginDowOrdinal, 1},
    {26, ViewEnum::SetValueViewEnum, Settings::Setting::DstEndMonth, 1},
    {27, ViewEnum::SetValueViewEnum, Settings::Setting::DstEndDowOrdinal, 1},
    {28, ViewEnum::SetValueViewEnum, Settings::Setting::DstSwitchDayOfWeek, 1},
    {29, ViewEnum::SetValueViewEnum, Settings::Setting::DstSwitchHour, 1},
    {30, ViewEnum::SetValueViewEnum, Settings::Setting::EffectDuration, 1},
    {31, ViewEnum::SetValueViewEnum, Settings::Setting::EffectFrequency, 1},
    {32, ViewEnum::SetValueViewEnum, Settings::Setting::MinimumIntensity, 1},
    {33, ViewEnum::SetValueViewEnum, Settings::Setting::BeeperVolume, 1},
    {34, ViewEnum::SetValueViewEnum, Settings::Setting::TemperatureCalibrationSTM32, 4},
    {35, ViewEnum::SetValueViewEnum, Settings::Setting::IdleTimeout, 1},
    {36, ViewEnum::SetValueViewEnum, Settings::Setting::DateFormat, 1},
    {37, ViewEnum::SetValueViewEnum, Settings::Setting::TimeZone, 1},
    {38, ViewEnum::SetValueViewEnum, Settings::Setting::ColonBehavior, 1},
    {39, ViewEnum::SetValueViewEnum, Settings::Setting::DmxAddress, 1},
    {40, ViewEnum::SetTimeDateViewEnum, Settings::Slot::Slot1, 1},
    {41, ViewEnum::SetTimeDateViewEnum, Settings::Slot::Slot2, 1},
    {42, ViewEnum::SetTimeDateViewEnum, Settings::Slot::Slot3, 1},
    {43, ViewEnum::SetTimeDateViewEnum, Settings::Slot::Slot4, 1},
    {44, ViewEnum::SetTimeDateViewEnum, Settings::Slot::Slot5, 1},
    {45, ViewEnum::SetTimeDateViewEnum, Settings::Slot::Slot6, 1},
    {46, ViewEnum::SetTimeDateViewEnum, Settings::Slot::Slot7, 1},
    {47, ViewEnum::SetTimeDateViewEnum, Settings::Slot::Slot8, 1},
    {98, ViewEnum::TestDisplayEnum, Settings::Setting::SystemOptions, 1},
};

// The maximum idle time (in ms) before the application switches back to the default view
//   Set from Settings::Setting::IdleTimeout (in seconds) via refreshSettings()
//   Counter is incremented by tick()
static uint32_t _maximumIdleCount = 0;

// Idle cycle counter; starts at 1 so loop() triggers idle immediately on boot
//   (refreshSettings() sets _maximumIdleCount and setOperatingMode() resets this to 0)
volatile static uint32_t _idleCounter = 1;

// minimum intensity (0-255 scale)
//
static uint8_t _minimumIntensity = 1;

// master display intensity (0-255 scale)
//
static uint8_t _masterIntensity = _minimumIntensity;

// important daylight savings dates & times
//
static DateTime _currentTime, _dstStart, _dstEnd;

// The current mode of the application
//
static OperatingMode _applicationMode;

// The application's current view mode
//
static ViewMode _viewMode;

// The application's current external control mode
//
static ExternalControl _externalControlMode = ExternalControl::NoActiveExtControlEnum;

// The application's current settings
//
static Settings _settings;

// The current displayed view of the application
//
// View *_currentView = cModeViews[static_cast<uint8_t>(OperatingMode::OperatingModeFixedDisplay)];
static View *_currentView = cModeViews[static_cast<uint8_t>(ViewEnum::TimeDateTempViewEnum)];

// determines if display intensity is adjusted in real-time
//
static bool _autoAdjustIntensities = false;

// DMX-512 signal status the last time we checked
//
static bool _previousDmxState = false;

// True if settings need to be written to FLASH
//
static bool _settingsModified = false;

// Set in initialize(): 0 = defaults used, 1 = loaded from flash, 2 = loaded from DS3234 SRAM
//
static uint8_t _settingsLoadFromFlashResult = 0;

static bool _wasIdle = false;

void initialize() {
  if (_settings.loadFromFlash()) {
    _settingsLoadFromFlashResult = 1;
    _settings.saveToSram();  // keep DS3234 SRAM in sync so recovery works after next firmware update
  } else if (_settings.loadFromSram()) {
    _settingsLoadFromFlashResult = 2;
  }

  // If the STM32 backup domain was reset (counter == 0), try to restore the on-time counter
  // from DS3234 SRAM. When settings came from SRAM we already know it's valid; otherwise
  // validate via CRC before trusting the stored counter.
  if (Hardware::onTimeSeconds() == 0) {
    if (_settingsLoadFromFlashResult == 2 || _settings.sramIsValid()) {
      Hardware::loadOnTimeCounterFromSram();
    }
  }

  GpsReceiver::initialize();
  SerialRemote::initialize();

  // _currentTime = Hardware::getDateTime();
  _currentTime = Application::dateTime();

  AlarmHandler::initialize();

  Dmx512Controller::initialize();
  // make sure the display lights up at start-up time if auto-adjust is disabled
  if (!_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::AutoAdjustIntensity)) {
    setIntensity(255);  // Full brightness
  }

  Hardware::setHvState(true);

  // we boot with _idleCounter=1 and _maximumIdleCount=0; idle mode results in calling
  //  setOperatingMode() from loop() right away, which will also refreshSettings()
}

DateTime dateTime() {
  // Read hardware time once to avoid race condition where _currentDateTime
  // could be updated by ISR between reading _currentTime.second() and Hardware::dateTime().second()
  DateTime hardwareTime = Hardware::dateTime();

  // Check if the second has changed in the hardware RTC
  // With PPS configured for EXTI_TRIGGER_RISING, this will update exactly once per second
  if (_currentTime.second() != hardwareTime.second() || _currentTime.isFirst()) {
    _currentTime = hardwareTime;

    if (_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::DstEnable) &&
        isDst(_currentTime)) {
      _currentTime = _currentTime.addSeconds(cDstOffsetSeconds);
    }
  }
  return _currentTime;
}

void setDateTime(const DateTime &now) {
  _currentTime = now;

  if (_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::DstEnable) && isDst(now)) {
    Hardware::setDateTime(_currentTime.addSeconds(-cDstOffsetSeconds));
  } else {
    Hardware::setDateTime(_currentTime);
  }
}

int32_t temperature(const bool fahrenheit, const bool bcd) { return Hardware::temperature(fahrenheit, bcd); }

int32_t temperatureCx10() { return Hardware::temperatureCx10(); }

int32_t temperatureCx10(Hardware::TempSensorType type) { return Hardware::temperatureCx10(type); }

bool temperatureUpdated() { return Hardware::temperatureUpdated(); }

uint8_t getModeDisplayNumber(OperatingMode mode) {
  if (mode <= OperatingMode::OperatingModeTestDisplay) {
    return cViewDescriptor[static_cast<uint8_t>(mode)].menuItemDisplayNumber;
  }
  return 0;
}

uint8_t getModeDisplayNumber(uint8_t mode) {
  if (mode <= static_cast<uint8_t>(OperatingMode::OperatingModeTestDisplay)) {
    return cViewDescriptor[mode].menuItemDisplayNumber;
  }
  return 0;
}

OperatingMode getOperatingMode() { return _applicationMode; }

void setOperatingMode(OperatingMode mode) {
  // nothing to do if we're already in this mode
  if (_applicationMode == mode) {
    return;
  }

  uint8_t blinkOnExit = 0;
  // first, write settings to FLASH, if needed
  if ((mode != OperatingMode::OperatingModeMainMenu) &&
      (mode < static_cast<uint8_t>(OperatingMode::OperatingModeSetSystemOptions)) && _settingsModified) {
    if (_settings.saveToFlashIfChanged()) {
      _settings.saveToSram();
      _settingsModified = false;
      blinkOnExit = 2;
    } else {
      blinkOnExit = 8;
    }
  }
  // ensure hardware is consistent with current settings
  // (this calls setDisplayBlanking(false), so blink() must come after)
  refreshSettings();
  // set the new mode
  _applicationMode = mode;
  _viewMode = ViewMode::ViewMode0;
  const viewDescriptor &vd = cViewDescriptor[static_cast<uint8_t>(mode)];
  _currentView = cModeViews[static_cast<uint8_t>(vd.view)];
  const Settings::SettingDescriptor *desc = nullptr;
  if (vd.relatedSettingNumber <= static_cast<uint8_t>(Settings::Setting::DmxAddress)) {
    desc = &Settings::cSettingDescriptors[vd.relatedSettingNumber];
  }
  _currentView->enter(desc, vd.relatedSettingNumber, vd.numSettings);
  // reset idle counter so the auto-return doesn't immediately override this mode, except for
  //  OperatingModeDmx512Display which enables controller() immediately upon entering this mode
  _idleCounter = (mode == OperatingMode::OperatingModeDmx512Display) ? _maximumIdleCount : 0;

  // Notify serial remote of mode change
  SerialRemote::notifyModeChange(static_cast<uint8_t>(_applicationMode), static_cast<uint8_t>(_viewMode));

  // Initiate non-blocking blink after all state changes so that
  // setDisplayBlanking(false) from refreshSettings() doesn't cancel it
  if (blinkOnExit) {
    DisplayManager::blink(blinkOnExit);
  }
}

ViewMode getViewMode() { return _viewMode; }

void setViewMode(ViewMode mode) {
  if (_viewMode != mode) {
    _viewMode = mode;

    SerialRemote::notifyModeChange(static_cast<uint8_t>(_applicationMode), static_cast<uint8_t>(_viewMode));
  }
}

uint8_t getOperatingModeRelatedSetting(OperatingMode mode) {
  return cViewDescriptor[static_cast<uint8_t>(mode)].relatedSettingNumber;
}

ExternalControl getExternalControlState() { return _externalControlMode; }

Settings *getSettingsPtr() { return &_settings; }

void refreshSettings() {
  int16_t timeZoneOffsetInMinutes =
      ((int16_t) _settings.getRawSetting(Settings::Setting::TimeZone) - Settings::cTimeZoneUtcValue) *
      Settings::cTimeZoneStepMinutes;

  // if ((_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::DstEnable) == true)
  //     && (isDst(_currentTime) == true))
  // {
  //   timeZoneOffsetInMinutes += 60;
  // }

  // Update hardware things
  Hardware::setVolume(_settings.getRawSetting(Settings::Setting::BeeperVolume));
  Hardware::setTemperatureCalibration(
      Hardware::TempSensorType::STM32ADC,
      (int16_t) _settings.getRawSetting(Settings::Setting::TemperatureCalibrationSTM32) -
          Settings::cCalibrationMidpoint);
  Hardware::setTemperatureCalibration(
      Hardware::TempSensorType::DS3234,
      (int16_t) _settings.getRawSetting(Settings::Setting::TemperatureCalibrationDS3234) -
          Settings::cCalibrationMidpoint);
  Hardware::setTemperatureCalibration(
      Hardware::TempSensorType::DS1722,
      (int16_t) _settings.getRawSetting(Settings::Setting::TemperatureCalibrationDS1722) -
          Settings::cCalibrationMidpoint);
  Hardware::setTemperatureCalibration(Hardware::TempSensorType::LM74,
                                      (int16_t) _settings.getRawSetting(Settings::Setting::TemperatureCalibrationLM74) -
                                          Settings::cCalibrationMidpoint);

  _maximumIdleCount = (uint32_t) _settings.getRawSetting(Settings::Setting::IdleTimeout) * 1000;
  DisplayManager::setDisplayBlanking(false);

  Animator::setAnimationDuration(_settings.getRawSetting(Settings::Setting::EffectDuration));

  GpsReceiver::setTimeZone(timeZoneOffsetInMinutes);

  // Convert stored setting (0-255) to master intensity scale (0-255)
  _minimumIntensity = _settings.getRawSetting(Settings::Setting::MinimumIntensity);
  setIntensityAutoAdjust(
      _settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::AutoAdjustIntensity), true);
}

void notifySettingsChanged() {
  refreshSettings();

  _settingsModified = true;
}

void notifySettingChanged(uint8_t settingNum) {
  SerialRemote::notifySettingChanged(settingNum);
  notifySettingsChanged();
}

bool saveSettingsToFlash() {
  bool result = _settings.saveToFlashIfChanged();
  if (result) {
    _settings.saveToSram();
  }
  return result;
}

// Computes the DateTime of the Nth occurrence of a day-of-week in a given month/year,
// then sets the time. On invalid settings (no matching day found), result is set to
// the default-constructed DateTime() to safely disable DST for the year.
//
static void _computeDstDate(DateTime &result, uint16_t year, uint16_t month, uint16_t dowOrdinal, uint16_t dow,
                            uint16_t hour) {
  uint8_t firstDay = ((dowOrdinal - 1) * 7) + 1;

  do {
    result.setDate(year, month, firstDay++);
  } while ((result.dayOfWeek() != dow) && (firstDay <= 32));

  if (firstDay > 32) {
    // Settings yielded no valid date (e.g., 5th Sunday of a short month).
    // Use epoch/zero to disable DST transitions for this year.
    result = DateTime();
    return;
  }

  result.setTime(hour, 0, 0);
}

// Handles DST date/time computation and setup
//
bool isDst(const DateTime &currentTime) {
  if (_dstStart.year(false) != currentTime.year(false)) {
    _computeDstDate(_dstStart, currentTime.year(false), _settings.getRawSetting(Settings::Setting::DstBeginMonth),
                    _settings.getRawSetting(Settings::Setting::DstBeginDowOrdinal),
                    _settings.getRawSetting(Settings::Setting::DstSwitchDayOfWeek),
                    _settings.getRawSetting(Settings::Setting::DstSwitchHour));
  }

  if (_dstEnd.year(false) != currentTime.year(false)) {
    _computeDstDate(_dstEnd, currentTime.year(false), _settings.getRawSetting(Settings::Setting::DstEndMonth),
                    _settings.getRawSetting(Settings::Setting::DstEndDowOrdinal),
                    _settings.getRawSetting(Settings::Setting::DstSwitchDayOfWeek),
                    _settings.getRawSetting(Settings::Setting::DstSwitchHour));

    // Fall-back hour is specified as local DST (wall clock) time, but isDst() receives
    // standard time from the hardware RTC. Subtract the DST offset so the comparison
    // fires at the correct standard-time moment (e.g. 2:00 AM DST == 1:00 AM standard).
    _dstEnd = _dstEnd.addSeconds(-cDstOffsetSeconds);
  }

  // once everything is calculated above, it becomes this simple...
  return (currentTime >= _dstStart && currentTime < _dstEnd);
}

// Updates the "master" intensity for the display based on lightLevel()
//
void _updateIntensityPercentage(const bool quick = false) {
  // Convert light level (0-4095) to intensity (0-255).
  // >> 4 maps identically to the exact formula at the endpoints and differs by
  // at most 1 count in the middle, which the ±2 deadband below absorbs.
  uint16_t currentIntensity = Hardware::lightLevel() >> 4;

  // Enforce minimum intensity
  if (currentIntensity < _minimumIntensity) {
    currentIntensity = _minimumIntensity;
  }

  if (quick) {
    // Quick mode: snap immediately to target
    _masterIntensity = currentIntensity;
  } else {
    // Smooth mode with deadband and adaptive step size
    int16_t error = currentIntensity - _masterIntensity;

    // Deadband of ±2 counts prevents hunting/flickering from noise
    if (error > 2) {
      // Adaptive step: larger steps when far from target, smaller when close
      // This gives fast initial response with smooth final approach
      uint8_t step = (error > 20) ? 2 : 1;
      _masterIntensity += step;
    } else if (error < -2) {
      uint8_t step = (error < -20) ? 2 : 1;
      _masterIntensity -= step;
    }
    // Within deadband: do nothing (prevents flickering)
  }
}

bool getIntensityAutoAdjust() { return _autoAdjustIntensities; }

void setIntensityAutoAdjust(const bool enable, const bool quickAdjust) {
  if (_autoAdjustIntensities != enable) {
    _autoAdjustIntensities = enable;

    if (enable && quickAdjust) {
      _updateIntensityPercentage(quickAdjust);
    }
  }
}

uint8_t getIntensity() { return _masterIntensity; }

uint8_t getStartupSettingsLoadResult() { return _settingsLoadFromFlashResult; }

void setIntensity(const uint8_t intensity) {
  _autoAdjustIntensities = false;
  _masterIntensity = intensity;  // 0-255 scale, no clamping needed for uint8_t
}

void tick() {
  if (_idleCounter <= _maximumIdleCount) {
    _idleCounter++;
  }

  if (_autoAdjustIntensities) {
    _updateIntensityPercentage();
  }
}

// Sets the HV state and notifies the serial remote; no-op if state is unchanged
//
static void _setHvState(const bool hvEnabled) {
  if (Hardware::getHvState() == hvEnabled) {
    return;
  }
  Hardware::setHvState(hvEnabled);
  SerialRemote::notifyHvStateChanged();
}

// Here lies the main application loop
//
void loop() {
  while (true) {
    // Refresh hardware data (date, time, temperature, etc.)
    Hardware::refresh();

    // We control the master display intensity only if DMX-512 is NOT active
    if (_externalControlMode != ExternalControl::Dmx512ExtControlEnum) {
      DisplayManager::setMasterIntensity(_masterIntensity);
    }

    // Service USB CDC-ACM (enumeration, RX, and TX all handled here)
    UsbSerial::poll();

    // Process any pending serial remote commands
    SerialRemote::process();

    // Check the buttons
    Keys::scanKeys();
    if (Keys::hasKeyPress()) {
      // Get the key from the buffer and process it
      auto key = Keys::getKeyPress();
      // Reset the counter since there has been button/key activity
      _idleCounter = 0;
      _wasIdle = false;
      if (key == Keys::Power) {
        // Toggle the HV supply; don't dispatch to views
        _setHvState(!Hardware::getHvState());
      } else {
        _setHvState(true);
        // If an alarm is active and external control is not, send keypresses there
        if ((AlarmHandler::isAlarmActive()) &&
            (_externalControlMode == Application::ExternalControl::NoActiveExtControlEnum)) {
          AlarmHandler::keyHandler(key);
        } else {
          if (Animator::isRunning()) {
            Animator::keyHandler(key);
          }
          if (_currentView->keyHandler(key)) {
            // The key did something, so make the tick sound
            Hardware::tick();
            // Take control back
            _externalControlMode = ExternalControl::NoActiveExtControlEnum;
            Dmx512Controller::setDmx512Active(false);
            // Make sure the display is visible (skip if blink() is in progress)
            if (!DisplayManager::isBlinkActive()) {
              DisplayManager::setDisplayBlanking(false);
            }
          }
        }
      }
    }
    if (Animator::isRunning()) {
      Animator::loop();
    } else {
      // Execute the loop block of the current view
      _currentView->loop();
    }
    // Process RTTTL playback
    RtttlPlayer::loop();

    // We do not signal alarms if some external control is active
    if (_externalControlMode == Application::ExternalControl::NoActiveExtControlEnum) {
      // Process any necessary alarms
      AlarmHandler::loop();
    }

    bool dmxSignalActive = Dmx512Rx::signalIsActive();

    // If the idle counter has maxed out, kick back to the appropriate display mode
    if (_idleCounter >= _maximumIdleCount) {
      // On transition to idle, set HV state based on current context
      if (!_wasIdle) {
        _wasIdle = true;
        if (dmxSignalActive) {
          _setHvState(true);
        } else {
          _setHvState(_settings.hvState() || (_applicationMode == OperatingMode::OperatingModeTimerCounter));
        }
      }

      // If there is an active signal, update _externalControlMode
      if (dmxSignalActive) {
        _externalControlMode = ExternalControl::Dmx512ExtControlEnum;
        Dmx512Controller::setDmx512Active(true);
      }
      // Kick back to the default display mode
      else if (_externalControlMode == ExternalControl::NoActiveExtControlEnum) {
        // Only change the display mode if it isn't already one of the time/date/temp modes
        if ((_applicationMode != OperatingMode::OperatingModeToggleDisplay) &&
            (_applicationMode != OperatingMode::OperatingModeFixedDisplay) &&
            (_applicationMode != OperatingMode::OperatingModeTimerCounter)) {
          if (_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::StartupToToggle)) {
            setOperatingMode(OperatingMode::OperatingModeToggleDisplay);
          } else {
            setOperatingMode(OperatingMode::OperatingModeFixedDisplay);
          }
        }
      }
    }

    if (_externalControlMode == ExternalControl::Dmx512ExtControlEnum) {
      // Allow the DMX-512 controller to do its thing
      Dmx512Controller::controller();
    }

    // Deal with changes in the DMX-512 signal state
    if (dmxSignalActive != _previousDmxState) {
      _previousDmxState = dmxSignalActive;

      if (_previousDmxState) {
        _externalControlMode = ExternalControl::Dmx512ExtControlEnum;
        Dmx512Controller::setDmx512Active(true);
        _setHvState(true);
      } else {
        _externalControlMode = ExternalControl::NoActiveExtControlEnum;
        Dmx512Controller::setDmx512Active(false);
        AlarmHandler::clearAlarm();  // just in case...
        if (_idleCounter >= _maximumIdleCount) {
          _setHvState(_settings.hvState() || (_applicationMode == OperatingMode::OperatingModeTimerCounter));
        }
      }
      // Setting this mode activates the view but also kicks us back to the menu if there is no signal
      setOperatingMode(OperatingMode::OperatingModeDmx512Display);
    }
  }
}

}  // namespace kbxTubeClock::Application
