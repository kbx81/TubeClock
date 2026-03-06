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

#include "Animator.h"
#include "Application.h"
#include "AlarmHandler.h"
#include "DateTime.h"
#include "Display.h"
#include "DisplayManager.h"
#include "Dmx-512-Controller.h"
#include "Dmx-512-Rx.h"
#include "Dmx-512-View.h"
#include "GpsReceiver.h"
#include "Keys.h"
#include "RtttlPlayer.h"
#include "SerialRemote.h"
#include "Settings.h"
#include "MainMenuView.h"
#include "TestDisplayView.h"
#include "TimeDateTempView.h"
#include "TimerCounterView.h"
#include "SetTimeDateView.h"
#include "SetTempCalibrationView.h"
#include "SetValueView.h"
#include "SetBitsView.h"
#include "SystemStatusView.h"


// Workaround for  "hidden symbol `__dso_handle' isn't defined" linker error
void* __dso_handle;


namespace kbxTubeClock {

namespace Application {

  // Color constant definitions (declared extern in Application.h)
  const RgbLed
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
    nixieOrange(1280, 496, 112);


// An array with all views for the application; must correspond with ViewEnum!
//
static View* const cModeViews[] = {
    new MainMenuView(),
    new TimeDateTempView(),
    new TimerCounterView(),
    new Dmx512View(),
    new SetTimeDateView(),
    new SetBitsView(),
    new SetValueView(),
    new SetTempCalibrationView(),
    new SystemStatusView(),
    new TestDisplayView()
};

// Structure defining menu views
//
struct viewDescriptor {
  uint8_t  menuItemDisplayNumber;
  ViewEnum view;
  uint8_t  relatedSettingNumber;
};


// An array of mode display numbers and their corresponding views
//
static viewDescriptor const cViewDescriptor[] = {
    {  0, ViewEnum::MainMenuViewEnum,     Settings::Setting::SystemOptions }, // OperatingModeMainMenu
    {  1, ViewEnum::TimeDateTempViewEnum, Settings::Setting::SystemOptions }, // OperatingModeFixedDisplay
    {  2, ViewEnum::TimeDateTempViewEnum, Settings::Setting::SystemOptions }, // OperatingModeToggleDisplay
    {  3, ViewEnum::TimerCounterViewEnum, Settings::Setting::SystemOptions }, // OperatingModeTimerCounter
    {  4, ViewEnum::Dmx512ViewEnum,       Settings::Setting::SystemOptions }, // OperatingModeDmx512Display
    {  5, ViewEnum::SetTimeDateViewEnum,  Settings::Setting::SystemOptions }, // OperatingModeSetClock
    {  6, ViewEnum::SetTimeDateViewEnum,  Settings::Setting::SystemOptions }, // OperatingModeSetDate
    {  7, ViewEnum::SetValueViewEnum,     Settings::Setting::TimerResetValue },
    {  8, ViewEnum::SystemStatusViewEnum, Settings::Setting::SystemOptions },
    { 10, ViewEnum::SetBitsViewEnum,      Settings::Setting::SystemOptions },
    { 11, ViewEnum::SetBitsViewEnum,      Settings::Setting::BeepStates },
    { 12, ViewEnum::SetBitsViewEnum,      Settings::Setting::BlinkStates },
    { 13, ViewEnum::SetBitsViewEnum,      Settings::Setting::OnOffStates },
    { 20, ViewEnum::SetValueViewEnum,     Settings::Setting::TimeDisplayDuration },
    { 21, ViewEnum::SetValueViewEnum,     Settings::Setting::DateDisplayDuration },
    { 22, ViewEnum::SetValueViewEnum,     Settings::Setting::TemperatureDisplayDuration },
    { 23, ViewEnum::SetValueViewEnum,     Settings::Setting::FadeDuration },
    { 24, ViewEnum::SetValueViewEnum,     Settings::Setting::DstBeginMonth },
    { 25, ViewEnum::SetValueViewEnum,     Settings::Setting::DstBeginDowOrdinal },
    { 26, ViewEnum::SetValueViewEnum,     Settings::Setting::DstEndMonth },
    { 27, ViewEnum::SetValueViewEnum,     Settings::Setting::DstEndDowOrdinal },
    { 28, ViewEnum::SetValueViewEnum,     Settings::Setting::DstSwitchDayOfWeek },
    { 29, ViewEnum::SetValueViewEnum,     Settings::Setting::DstSwitchHour },
    { 30, ViewEnum::SetValueViewEnum,     Settings::Setting::EffectDuration },
    { 31, ViewEnum::SetValueViewEnum,     Settings::Setting::EffectFrequency },
    { 32, ViewEnum::SetValueViewEnum,     Settings::Setting::MinimumIntensity },
    { 33, ViewEnum::SetValueViewEnum,     Settings::Setting::BeeperVolume },
    { 34, ViewEnum::SetTempCalibrationViewEnum, Settings::Setting::TemperatureCalibrationSTM32 },
    { 35, ViewEnum::SetValueViewEnum,     Settings::Setting::DisplayRefreshInterval },
    { 36, ViewEnum::SetValueViewEnum,     Settings::Setting::DateFormat },
    { 37, ViewEnum::SetValueViewEnum,     Settings::Setting::TimeZone },
    { 38, ViewEnum::SetValueViewEnum,     Settings::Setting::ColonBehavior },
    { 39, ViewEnum::SetValueViewEnum,     Settings::Setting::DmxAddress },
    { 40, ViewEnum::SetTimeDateViewEnum,  Settings::Slot::Slot1 },
    { 41, ViewEnum::SetTimeDateViewEnum,  Settings::Slot::Slot2 },
    { 42, ViewEnum::SetTimeDateViewEnum,  Settings::Slot::Slot3 },
    { 43, ViewEnum::SetTimeDateViewEnum,  Settings::Slot::Slot4 },
    { 44, ViewEnum::SetTimeDateViewEnum,  Settings::Slot::Slot5 },
    { 45, ViewEnum::SetTimeDateViewEnum,  Settings::Slot::Slot6 },
    { 46, ViewEnum::SetTimeDateViewEnum,  Settings::Slot::Slot7 },
    { 47, ViewEnum::SetTimeDateViewEnum,  Settings::Slot::Slot8 },
    { 98, ViewEnum::TestDisplayEnum,      Settings::Setting::SystemOptions }
};

// The maximum idle time before the application switches back to the default view
//   Counter is incremented by tick()
static const uint32_t cMaximumIdleCount = 120000;

// Idle cycle counter
//
volatile static auto _idleCounter = cMaximumIdleCount;

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

// Set in initialize() when settings are loaded from flash
//
static bool _settingsLoadFromFlashResult = false;

static bool _wasIdle = false;


void initialize()
{
  _settingsLoadFromFlashResult = _settings.loadFromFlash();

  GpsReceiver::initialize();
  SerialRemote::initialize();

  // _currentTime = Hardware::getDateTime();
  _currentTime = Application::dateTime();

  AlarmHandler::initialize();

  Dmx512Controller::initialize();
  // make sure the display lights up at start-up time if auto-adjust is disabled
  if (!_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::AutoAdjustIntensity))
  {
    setIntensity(255);  // Full brightness
  }

  Hardware::setHvState(true);

  // we boot with _idleCounter = cMaximumIdleCount; idle mode results in calling
  //  setOperatingMode() from loop() right away, which will also refreshSettings()
}


DateTime dateTime()
{
  // Read hardware time once to avoid race condition where _currentDateTime
  // could be updated by ISR between reading _currentTime.second() and Hardware::dateTime().second()
  DateTime hardwareTime = Hardware::dateTime();

  // Check if the second has changed in the hardware RTC
  // With PPS configured for EXTI_TRIGGER_RISING, this will update exactly once per second
  if (_currentTime.second() != hardwareTime.second() || _currentTime.isFirst())
  {
    _currentTime = hardwareTime;

    if (_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::DstEnable)
        && isDst(_currentTime))
    {
      _currentTime = _currentTime.addSeconds(cDstOffsetSeconds);
    }
  }
  return _currentTime;
}


void setDateTime(const DateTime &now)
{
  _currentTime = now;

  if (_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::DstEnable)
      && isDst(now))
  {
    Hardware::setDateTime(_currentTime.addSeconds(-cDstOffsetSeconds));
  }
  else
  {
    Hardware::setDateTime(_currentTime);
  }
}


int32_t temperature(const bool fahrenheit, const bool bcd)
{
  return Hardware::temperature(fahrenheit, bcd);
}


int32_t temperatureCx10()
{
  return Hardware::temperatureCx10();
}


int32_t temperatureCx10(Hardware::TempSensorType type)
{
  return Hardware::temperatureCx10(type);
}


bool temperatureUpdated()
{
  return Hardware::temperatureUpdated();
}


uint8_t getModeDisplayNumber(OperatingMode mode)
{
  if (mode <= OperatingMode::OperatingModeTestDisplay)
  {
    return cViewDescriptor[static_cast<uint8_t>(mode)].menuItemDisplayNumber;
  }
  return 0;
}


uint8_t getModeDisplayNumber(uint8_t mode)
{
  if (mode <= static_cast<uint8_t>(OperatingMode::OperatingModeTestDisplay))
  {
    return cViewDescriptor[mode].menuItemDisplayNumber;
  }
  return 0;
}


OperatingMode getOperatingMode()
{
  return _applicationMode;
}


void setOperatingMode(OperatingMode mode)
{
  // nothing to do if we're already in this mode
  if (_applicationMode == mode) return;

  const uint16_t writeLedIntensity = 1024;
  bool blinkOnExit = false;
  // first, write settings to FLASH, if needed
  if ((mode != OperatingMode::OperatingModeMainMenu) &&
      (mode < static_cast<uint8_t>(OperatingMode::OperatingModeSetSystemOptions)) &&
      _settingsModified)
  {
    if (_settings.saveToFlashIfChanged())
    {
      _settingsModified = false;
      Hardware::setGreenLed(writeLedIntensity);
    }
    else
    {
      Hardware::setRedLed(writeLedIntensity);
    }
    blinkOnExit = true;
  }
  // ensure hardware is consistent with current settings
  // (this calls setDisplayBlanking(false), so blink() must come after)
  refreshSettings();
  // set the new mode
  _applicationMode = mode;
  _viewMode = ViewMode::ViewMode0;
  const viewDescriptor &vd = cViewDescriptor[static_cast<uint8_t>(mode)];
  _currentView = cModeViews[static_cast<uint8_t>(vd.view)];
  _currentView->enter(vd.relatedSettingNumber);
  // reset idle counter so the auto-return doesn't immediately override this mode
  _idleCounter = 0;
  // this enables controller() immediately upon entering this mode
  if (mode == OperatingMode::OperatingModeDmx512Display)
  {
    _idleCounter = cMaximumIdleCount;
  }

  // Notify serial remote of mode change
  SerialRemote::notifyModeChange(
    static_cast<uint8_t>(_applicationMode),
    static_cast<uint8_t>(_viewMode));

  // Initiate non-blocking blink after all state changes so that
  // setDisplayBlanking(false) from refreshSettings() doesn't cancel it
  if (blinkOnExit)
  {
    DisplayManager::blink();
  }
}


ViewMode getViewMode()
{
  return _viewMode;
}


void setViewMode(ViewMode mode)
{
  if (_viewMode != mode)
  {
    _viewMode = mode;

    SerialRemote::notifyModeChange(
      static_cast<uint8_t>(_applicationMode),
      static_cast<uint8_t>(_viewMode));
  }
}


uint8_t getOperatingModeRelatedSetting(OperatingMode mode)
{
  return cViewDescriptor[static_cast<uint8_t>(mode)].relatedSettingNumber;
}


ExternalControl getExternalControlState()
{
  return _externalControlMode;
}


Settings* getSettingsPtr()
{
  return &_settings;
}


void refreshSettings()
{
  int16_t timeZoneOffsetInMinutes = (_settings.getRawSetting(Settings::Setting::TimeZone) - 56) * 15;

  // if ((_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::DstEnable) == true)
  //     && (isDst(_currentTime) == true))
  // {
  //   timeZoneOffsetInMinutes += 60;
  // }

  // Update hardware things
  Hardware::setVolume(_settings.getRawSetting(Settings::Setting::BeeperVolume));
  Hardware::setTemperatureCalibration(Hardware::TempSensorType::STM32ADC,
    (int16_t)_settings.getRawSetting(Settings::Setting::TemperatureCalibrationSTM32) - Settings::cCalibrationMidpoint);
  Hardware::setTemperatureCalibration(Hardware::TempSensorType::DS3234,
    (int16_t)_settings.getRawSetting(Settings::Setting::TemperatureCalibrationDS3234) - Settings::cCalibrationMidpoint);
  Hardware::setTemperatureCalibration(Hardware::TempSensorType::DS1722,
    (int16_t)_settings.getRawSetting(Settings::Setting::TemperatureCalibrationDS1722) - Settings::cCalibrationMidpoint);
  Hardware::setTemperatureCalibration(Hardware::TempSensorType::LM74,
    (int16_t)_settings.getRawSetting(Settings::Setting::TemperatureCalibrationLM74) - Settings::cCalibrationMidpoint);

  DisplayManager::setDisplayRefreshInterval(_settings.getRawSetting(Settings::Setting::DisplayRefreshInterval));
  DisplayManager::setDisplayBlanking(false);

  Animator::setAnimationDuration(_settings.getRawSetting(Settings::Setting::EffectDuration));

  GpsReceiver::setTimeZone(timeZoneOffsetInMinutes);

  // Convert stored setting (0-255) to master intensity scale (0-255)
  _minimumIntensity = _settings.getRawSetting(Settings::Setting::MinimumIntensity);
  setIntensityAutoAdjust(_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::AutoAdjustIntensity), true);
}


void notifySettingsChanged()
{
  refreshSettings();

  _settingsModified = true;
}


void notifySettingChanged(uint8_t settingNum)
{
  SerialRemote::notifySettingChanged(settingNum);
  notifySettingsChanged();
}


bool saveSettingsToFlash()
{
  return _settings.saveToFlashIfChanged();
}


// Handles DST date/time computation and setup
//
bool isDst(const DateTime &currentTime)
{
  uint8_t firstDay;

  if (_dstStart.year(false) != currentTime.year(false))
  {
    firstDay = ((_settings.getRawSetting(Settings::Setting::DstBeginDowOrdinal) - 1) * 7) + 1;

    do
    {
      _dstStart.setDate(currentTime.year(false), _settings.getRawSetting(Settings::Setting::DstBeginMonth), firstDay++);
    }
    while((_dstStart.dayOfWeek() != _settings.getRawSetting(Settings::Setting::DstSwitchDayOfWeek)) && (firstDay <= 31));

    _dstStart.setTime(_settings.getRawSetting(Settings::Setting::DstSwitchHour), 0, 0);
  }

  if (_dstEnd.year(false) != currentTime.year(false))
  {
    firstDay = ((_settings.getRawSetting(Settings::Setting::DstEndDowOrdinal) - 1) * 7) + 1;

    do
    {
      _dstEnd.setDate(currentTime.year(false), _settings.getRawSetting(Settings::Setting::DstEndMonth), firstDay++);
    }
    while((_dstEnd.dayOfWeek() != _settings.getRawSetting(Settings::Setting::DstSwitchDayOfWeek)) && (firstDay <= 31));

    // Fall-back hour is specified as local DST (wall clock) time, but isDst() receives
    // standard time from the hardware RTC. Subtract the DST offset so the comparison
    // fires at the correct standard-time moment (e.g. 2:00 AM DST == 1:00 AM standard).
    _dstEnd.setTime(_settings.getRawSetting(Settings::Setting::DstSwitchHour), 0, 0);
    _dstEnd = _dstEnd.addSeconds(-cDstOffsetSeconds);
  }

  // once everything is calculated above, it becomes this simple...
  return (currentTime >= _dstStart && currentTime < _dstEnd);
}


// Updates the "master" intensity for the display based on lightLevel()
//
void _updateIntensityPercentage(const bool quick = false)
{
  // Convert light level (0-4095) to intensity (0-255).
  // >> 4 maps identically to the exact formula at the endpoints and differs by
  // at most 1 count in the middle, which the ±2 deadband below absorbs.
  uint16_t currentIntensity = Hardware::lightLevel() >> 4;

  // Enforce minimum intensity
  if (currentIntensity < _minimumIntensity)
  {
    currentIntensity = _minimumIntensity;
  }

  if (quick)
  {
    // Quick mode: snap immediately to target
    _masterIntensity = currentIntensity;
  }
  else
  {
    // Smooth mode with deadband and adaptive step size
    int16_t error = currentIntensity - _masterIntensity;

    // Deadband of ±2 counts prevents hunting/flickering from noise
    if (error > 2)
    {
      // Adaptive step: larger steps when far from target, smaller when close
      // This gives fast initial response with smooth final approach
      uint8_t step = (error > 20) ? 2 : 1;
      _masterIntensity += step;
    }
    else if (error < -2)
    {
      uint8_t step = (error < -20) ? 2 : 1;
      _masterIntensity -= step;
    }
    // Within deadband: do nothing (prevents flickering)
  }
}


bool getIntensityAutoAdjust()
{
  return _autoAdjustIntensities;
}


void setIntensityAutoAdjust(const bool enable, const bool quickAdjust)
{
  if (_autoAdjustIntensities != enable)
  {
    _autoAdjustIntensities = enable;

    if (enable && quickAdjust)
    {
      _updateIntensityPercentage(quickAdjust);
    }
  }
}


uint8_t getIntensity()
{
  return _masterIntensity;
}


bool getStartupSettingsLoadResult()
{
  return _settingsLoadFromFlashResult;
}


void setIntensity(const uint8_t intensity)
{
  _autoAdjustIntensities = false;
  _masterIntensity = intensity;  // 0-255 scale, no clamping needed for uint8_t
}


void tick()
{
  if (_idleCounter <= cMaximumIdleCount)
  {
    _idleCounter++;
  }

  if (_autoAdjustIntensities)
  {
    _updateIntensityPercentage();
  }
}


// Sets the HV state and notifies the serial remote; no-op if state is unchanged
//
static void _setHvState(const bool hvEnabled)
{
  if (Hardware::getHvState() == hvEnabled) return;
  Hardware::setHvState(hvEnabled);
  SerialRemote::notifyHvStateChanged();
}


// Here lies the main application loop
//
void loop()
{
  while(true)
  {
    // Refresh hardware data (date, time, temperature, etc.)
    Hardware::refresh();

    // We control the master display intensity only if DMX-512 is NOT active
    if (_externalControlMode != ExternalControl::Dmx512ExtControlEnum)
    {
      DisplayManager::setMasterIntensity(_masterIntensity);
    }

    // Process any pending serial remote commands
    SerialRemote::process();

    // Check the buttons
    Keys::scanKeys();
    if (Keys::hasKeyPress())
    {
      // Get the key from the buffer and process it
      auto key = Keys::getKeyPress();
      // Reset the counter since there has been button/key activity
      _idleCounter = 0;
      _wasIdle = false;
      if (key == Keys::Power)
      {
        // Toggle the HV supply; don't dispatch to views
        _setHvState(!Hardware::getHvState());
      }
      else
      {
        _setHvState(true);
        // If an alarm is active and external control is not, send keypresses there
        if ((AlarmHandler::isAlarmActive())
         && (_externalControlMode == Application::ExternalControl::NoActiveExtControlEnum))
        {
          AlarmHandler::keyHandler(key);
        }
        else
        {
          if (Animator::isRunning())
          {
            Animator::keyHandler(key);
          }
          if (_currentView->keyHandler(key))
          {
            // The key did something, so make the tick sound
            Hardware::tick();
            // Take control back
            _externalControlMode = ExternalControl::NoActiveExtControlEnum;
            Dmx512Controller::setDmx512Active(false);
            // Make sure the display is visible (skip if blink() is in progress)
            if (!DisplayManager::isBlinkActive())
            {
              DisplayManager::setDisplayBlanking(false);
            }
          }
        }
      }
    }
    if (Animator::isRunning())
    {
      Animator::loop();
    }
    else
    {
      // Execute the loop block of the current view
      _currentView->loop();
    }
    // Process RTTTL playback
    RtttlPlayer::loop();

    // We do not signal alarms if some external control is active
    if (_externalControlMode == Application::ExternalControl::NoActiveExtControlEnum)
    {
      // Process any necessary alarms
      AlarmHandler::loop();
    }

    bool dmxSignalActive = Dmx512Rx::signalIsActive();

    // If the idle counter has maxed out, kick back to the appropriate display mode
    if (_idleCounter >= cMaximumIdleCount)
    {
      // On transition to idle, set HV state based on current context
      if (!_wasIdle)
      {
        _wasIdle = true;
        if (dmxSignalActive)
        {
          _setHvState(true);
        }
        else
        {
          _setHvState(_settings.hvState() || (_applicationMode == OperatingMode::OperatingModeTimerCounter));
        }
      }

      // If there is an active signal, update _externalControlMode
      if (dmxSignalActive)
      {
        _externalControlMode = ExternalControl::Dmx512ExtControlEnum;
        Dmx512Controller::setDmx512Active(true);
      }
      // Kick back to the default display mode
      else if (_externalControlMode == ExternalControl::NoActiveExtControlEnum)
      {
        // Only change the display mode if it isn't already one of the time/date/temp modes
        if ((_applicationMode != OperatingMode::OperatingModeToggleDisplay) &&
            (_applicationMode != OperatingMode::OperatingModeFixedDisplay) &&
            (_applicationMode != OperatingMode::OperatingModeTimerCounter))
        {
          if (_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::StartupToToggle))
          {
            setOperatingMode(OperatingMode::OperatingModeToggleDisplay);
          }
          else
          {
            setOperatingMode(OperatingMode::OperatingModeFixedDisplay);
          }
        }
      }
    }

    if (_externalControlMode == ExternalControl::Dmx512ExtControlEnum)
    {
      // Allow the DMX-512 controller to do its thing
      Dmx512Controller::controller();
    }

    // Deal with changes in the DMX-512 signal state
    if (dmxSignalActive != _previousDmxState)
    {
      _previousDmxState = dmxSignalActive;

      if (_previousDmxState)
      {
        _externalControlMode = ExternalControl::Dmx512ExtControlEnum;
        Dmx512Controller::setDmx512Active(true);
        _setHvState(true);
      }
      else
      {
        _externalControlMode = ExternalControl::NoActiveExtControlEnum;
        Dmx512Controller::setDmx512Active(false);
        AlarmHandler::clearAlarm();   // just in case...
        if (_idleCounter >= cMaximumIdleCount)
        {
          _setHvState(_settings.hvState() || (_applicationMode == OperatingMode::OperatingModeTimerCounter));
        }
      }
      // Setting this mode activates the view but also kicks us back to the menu if there is no signal
      setOperatingMode(OperatingMode::OperatingModeDmx512Display);
    }
  }
}


}

}
