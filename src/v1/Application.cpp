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
#include "SerialRemote.h"
#include "Settings.h"
#include "MainMenuView.h"
#include "TestDisplayView.h"
#include "TimeDateTempView.h"
#include "TimerCounterView.h"
#include "SetTimeDateView.h"
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
    nixieOrange(256, 32, 2);


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
    { 34, ViewEnum::SetValueViewEnum,     Settings::Setting::TemperatureCalibration },
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

// daylight savings state
//
static DstState _dstState;

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

static bool _wasIdle = false;


void initialize()
{
  if (_settings.loadFromFlash() == false)
  {
    // blink to alert that settings could not be loaded
    Hardware::blinkStatusLed(Application::green, Application::orange, 12, 80);
    Hardware::delay(500);
    Hardware::setStatusLed(RgbLed());
  }

  GpsReceiver::initialize();
  SerialRemote::initialize();

  // _currentTime = Hardware::getDateTime();
  _currentTime = Application::dateTime();

  AlarmHandler::initialize();

  Dmx512Controller::initialize();
  // make sure the display lights up at start-up time if auto-adjust is disabled
  if (_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::AutoAdjustIntensity) == false)
  {
    setIntensity(255);  // Full brightness
  }
  // setOperatingMode() will refreshSettings()
  if (_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::StartupToToggle) == true)
  {
    setOperatingMode(OperatingMode::OperatingModeToggleDisplay);
  }
  else
  {
    setOperatingMode(OperatingMode::OperatingModeFixedDisplay);
  }
}


DateTime dateTime()
{
  // Read hardware time once to avoid race condition where _currentDateTime
  // could be updated by ISR between reading _currentTime.second() and Hardware::dateTime().second()
  DateTime hardwareTime = Hardware::dateTime();

  // Check if the second has changed in the hardware RTC
  // With PPS configured for EXTI_TRIGGER_RISING, this will update exactly once per second
  if (_currentTime.second() != hardwareTime.second())
  {
    _currentTime = hardwareTime;

    if ((_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::DstEnable) == true)
        && (isDst(_currentTime) == true))
    {
      _currentTime = _currentTime.addSeconds(cDstOffsetSeconds);
    }
  }
  return _currentTime;
}


void setDateTime(const DateTime &now)
{
  _currentTime = now;

  if ((_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::DstEnable) == true)
      && (isDst(now) == true))
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
  return cViewDescriptor[static_cast<uint8_t>(mode)].menuItemDisplayNumber;
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
  // first, write settings to FLASH, if needed
  if ((mode != OperatingMode::OperatingModeMainMenu) &&
      (mode < static_cast<uint8_t>(OperatingMode::OperatingModeSetSystemOptions)) &&
      (_settingsModified == true))
  {
    if (_settings.saveToFlash() == 0)
    {
      _settingsModified = false;
      Hardware::setGreenLed(writeLedIntensity);
    }
    else
    {
      Hardware::setRedLed(writeLedIntensity);
    }
    DisplayManager::doubleBlink();
    Hardware::setStatusLed(RgbLed());
  }
  // ensure hardware is consistent with current settings
  refreshSettings();
  // set the new mode
  _applicationMode = mode;
  _viewMode = ViewMode::ViewMode0;
  _currentView = cModeViews[static_cast<uint8_t>(cViewDescriptor[static_cast<uint8_t>(mode)].view)];
  _currentView->enter();
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


Settings getSettings()
{
  return _settings;
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
  Hardware::setTemperatureCalibration((int8_t)(-(_settings.getRawSetting(Settings::Setting::TemperatureCalibration))));

  DisplayManager::setDisplayRefreshInterval(_settings.getRawSetting(Settings::Setting::DisplayRefreshInterval));
  DisplayManager::setDisplayBlanking(false);

  Animator::setAnimationDuration(_settings.getRawSetting(Settings::Setting::EffectDuration));

  GpsReceiver::setTimeZone(timeZoneOffsetInMinutes);

  // Convert stored setting (0-255) to master intensity scale (0-255)
  _minimumIntensity = _settings.getRawSetting(Settings::Setting::MinimumIntensity);
  setIntensityAutoAdjust(_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::AutoAdjustIntensity), true);
}


void setSettings(const Settings &settings)
{
  _settings = settings;

  refreshSettings();

  _settingsModified = true;
}


// Handles DST date/time computation and setup
//
bool isDst(const DateTime &currentTime)
{
  uint8_t firstDay;

  if (_dstStart.year(false) != currentTime.year(false))
  {
    _dstState = DstState::Reset;
    firstDay = ((_settings.getRawSetting(Settings::Setting::DstBeginDowOrdinal) - 1) * 7) + 1;

    do
    {
      _dstStart.setDate(currentTime.year(false), _settings.getRawSetting(Settings::Setting::DstBeginMonth), firstDay++);
    }
    while((_dstStart.dayOfWeek() != _settings.getRawSetting(Settings::Setting::DstSwitchDayOfWeek)) && (firstDay <= 31));

    _dstStart.setTime(_settings.getRawSetting(Settings::Setting::DstSwitchHour), 0, 0);

    if (currentTime >= _dstStart)
    {
      _dstState = DstState::Spring;
    }
  }

  if (_dstEnd.year(false) != currentTime.year(false))
  {
    firstDay = ((_settings.getRawSetting(Settings::Setting::DstEndDowOrdinal) - 1) * 7) + 1;

    do
    {
      _dstEnd.setDate(currentTime.year(false), _settings.getRawSetting(Settings::Setting::DstEndMonth), firstDay++);
    }
    while((_dstEnd.dayOfWeek() != _settings.getRawSetting(Settings::Setting::DstSwitchDayOfWeek)) && (firstDay <= 31));

    _dstEnd.setTime(_settings.getRawSetting(Settings::Setting::DstSwitchHour), 0, 0);

    if (currentTime >= _dstEnd)
    {
      _dstState = DstState::Fall;
    }
  }

  // once everything is calculated above, it becomes this simple...
  return (currentTime >= _dstStart && currentTime < _dstEnd);
}


// Handles updating of the clock hardware for DST
//
void _refreshDst()
{
  // _currentTime = Hardware::getDateTime();
  //
  // if ((_currentTime >= _dstEnd) && (_dstState == DstState::Spring))
  // {
  //   _dstState = DstState::Fall;
  //   Hardware::setDstState(isDst(_currentTime), true);
  // }
  //
  // if ((_currentTime >= _dstStart) && (_dstState == DstState::Reset))
  // {
  //   _dstState = DstState::Spring;
  //   Hardware::setDstState(isDst(_currentTime), true);
  // }
}


// Updates the "master" intensity for the display based on lightLevel()
//
void _updateIntensityPercentage(const bool quick = false)
{
  // Convert light level (0-4095) to intensity (0-255)
  // Need accurate conversion here - the bit shift approximation was too inaccurate
  uint16_t currentIntensity = (Hardware::lightLevel() * 255) / 4095;

  // Enforce minimum intensity
  if (currentIntensity < _minimumIntensity)
  {
    currentIntensity = _minimumIntensity;
  }

  if (quick == true)
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

    if ((enable == true) && (quickAdjust == true))
    {
      _updateIntensityPercentage(quickAdjust);
    }
  }
}


uint8_t getIntensity()
{
  return _masterIntensity;
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

  if (_autoAdjustIntensities == true)
  {
    _updateIntensityPercentage();
  }
}


// Here lies the main application loop
//
void loop()
{
  while(true)
  {
    // Refresh hardware data (date, time, temperature, etc.)
    Hardware::refresh();

    // Check up on DST and adjust the time if enabled based on settings
    // if (_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::DstEnable) == true)
    // {
    //   _refreshDst();
    // }
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
      Hardware::setHvState(true);
      // If an alarm is active and external control is not, send keypresses there
      if ((AlarmHandler::isAlarmActive())
       && (_externalControlMode == Application::ExternalControl::NoActiveExtControlEnum))
      {
        AlarmHandler::keyHandler(key);
      }
      else
      {
        if (Animator::isRunning() == true)
        {
          Animator::keyHandler(key);
        }
        if (_currentView->keyHandler(key) == true)
        {
          // The key did something, so make the tick sound
          Hardware::tick();
          // Take control back
          _externalControlMode = ExternalControl::NoActiveExtControlEnum;
          Dmx512Controller::setDmx512Active(false);
          // Make sure the display is visible
          DisplayManager::setDisplayBlanking(false);
        }
      }
    }
    if (Animator::isRunning() == true)
    {
      Animator::loop();
    }
    else
    {
      // Execute the loop block of the current view
      _currentView->loop();
    }
    // We do not signal alarms if some external control is active
    if (_externalControlMode == Application::ExternalControl::NoActiveExtControlEnum)
    {
      // Process any necessary alarms
      AlarmHandler::loop();
    }

    // If the idle counter has maxed out, kick back to the appropriate display mode
    if (_idleCounter >= cMaximumIdleCount)
    {
      // On transition to idle, set HV state based on current context
      if (!_wasIdle)
      {
        _wasIdle = true;
        if (Dmx512Rx::signalIsActive() == true)
        {
          Hardware::setHvState(true);
        }
        else
        {
          Hardware::setHvState(_settings.hvState() || (_applicationMode == OperatingMode::OperatingModeTimerCounter));
        }
      }

      // If there is an active signal, update _externalControlMode
      if (Dmx512Rx::signalIsActive() == true)
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
          if (_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::StartupToToggle) == true)
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
    if (Dmx512Rx::signalIsActive() != _previousDmxState)
    {
      _previousDmxState = Dmx512Rx::signalIsActive();

      if (_previousDmxState == true)
      {
        _externalControlMode = ExternalControl::Dmx512ExtControlEnum;
        Dmx512Controller::setDmx512Active(true);
        Hardware::setHvState(true);
      }
      else
      {
        _externalControlMode = ExternalControl::NoActiveExtControlEnum;
        Dmx512Controller::setDmx512Active(false);
        AlarmHandler::clearAlarm();   // just in case...
        if (_idleCounter >= cMaximumIdleCount)
        {
          Hardware::setHvState(_settings.hvState() || (_applicationMode == OperatingMode::OperatingModeTimerCounter));
        }
      }
      // Setting this mode activates the view but also kicks us back to the menu if there is no signal
      setOperatingMode(OperatingMode::OperatingModeDmx512Display);
    }
  }
}


}

}
