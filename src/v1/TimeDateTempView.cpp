//
// kbx81's tube clock TimeDateTempView class
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

#include "Animator.h"
#include "Application.h"
#include "DateTime.h"
#include "Display.h"
#include "DisplayManager.h"
#include "NixieGlyph.h"
#include "Settings.h"
#include "TimeDateTempView.h"


namespace kbxTubeClock {


const uint8_t TimeDateTempView::cIntensityAdjustmentIncrement = 100;


TimeDateTempView::TimeDateTempView()
  : _currentTemperature(0),
    _currentTime(Application::dateTime()),
    _lastAnimationTime(DateTime()),
    _lastSwitchTime(DateTime()),
    _mode(Application::OperatingMode::OperatingModeFixedDisplay)
{
}


void TimeDateTempView::enter()
{
  Settings *pSettings = Application::getSettingsPtr();

  _mode = Application::getOperatingMode();

  _currentTime = Application::dateTime();

  _lastAnimationTime = _currentTime;

  _lastSwitchTime = _currentTime;

  DisplayManager::setStatusLedAutoRefreshing(pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::StatusLedAsAmPm));
}


bool TimeDateTempView::keyHandler(Keys::Key key)
{
  FixedDisplayItem currentDisplayItem = static_cast<FixedDisplayItem>(Application::getViewMode());
  bool tick = false;

  if (key == Keys::Key::A)
  {
    if (currentDisplayItem != FixedDisplayItem::Time)
    {
      Application::setViewMode(static_cast<ViewMode>(FixedDisplayItem::Time));
      tick = true;
    }
    else if (currentDisplayItem != FixedDisplayItem::TimeSeconds)
    {
      Application::setViewMode(static_cast<ViewMode>(FixedDisplayItem::TimeSeconds));
      tick = true;
    }
  }

  if ((key == Keys::Key::B) && (currentDisplayItem != FixedDisplayItem::Date))
  {
    Application::setViewMode(static_cast<ViewMode>(FixedDisplayItem::Date));
    tick = true;
  }

  if ((key == Keys::Key::C) && (currentDisplayItem != FixedDisplayItem::Temperature))
  {
    Application::setViewMode(static_cast<ViewMode>(FixedDisplayItem::Temperature));
    tick = true;
  }

  if ((key == Keys::Key::D) && (Application::getIntensityAutoAdjust() == false))
  {
    const int16_t intensity = Application::getIntensity() - cIntensityAdjustmentIncrement;
    if (intensity > 0)
    {
      Application::setIntensity(intensity);
      tick = true;
    }
    else
    {
      Application::setIntensity(0);
    }
  }

  if ((key == Keys::Key::U) && (Application::getIntensityAutoAdjust() == false))
  {
    const int16_t intensity = Application::getIntensity() + cIntensityAdjustmentIncrement;
    if (intensity <= RgbLed::cLed100Percent)
    {
      Application::setIntensity(intensity);
      tick = true;
    }
    else
    {
      Application::setIntensity(RgbLed::cLed100Percent);
    }
  }

  if (key == Keys::Key::E)
  {
    Application::setOperatingMode(Application::OperatingMode::OperatingModeMainMenu);
    tick = true;
  }

  if (tick == true)
  {
    // _lastAnimationTime = _currentTime.secondsSinceMidnight();
    // _lastSwitchTime = _currentTime.secondsSinceMidnight();
    _lastAnimationTime = _currentTime;
    _lastSwitchTime = _currentTime;
  }

  return tick;
}


void TimeDateTempView::loop()
{
  Application::ExternalControl externalControlState = Application::getExternalControlState();
  Settings *pSettings = Application::getSettingsPtr();
  DateTime animationDisplayTime = _lastAnimationTime.addSeconds(pSettings->getRawSetting(Settings::Setting::EffectFrequency)),
           changeDisplayTime;
  Display  tcDisp;
  FixedDisplayItem currentDisplayItem = static_cast<FixedDisplayItem>(Application::getViewMode()),
                   nextDisplayItem = static_cast<FixedDisplayItem>(Application::getViewMode());
  RgbLed   statusLed;
  uint32_t itemDisplayDuration = 0;
  bool displayFahrenheit = pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::DisplayFahrenheit);

  // update these only as needed -- keeps temperature from bouncing incessantly
  if (_currentTime != Application::dateTime())
  {
    _currentTime = Application::dateTime();
    _currentTemperature = Hardware::temperature(displayFahrenheit);
  }
  // set status LED to appropriate color if enabled as AM/PM indicator
  if (pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::StatusLedAsAmPm) == true)
  {
    // this will set the color and (more importantly) fade duration correctly
    if (_currentTime.isPM() == true)
    {
      statusLed = Application::nixieOrange;
    }
    // turn it off if we're not displaying the time (a bit silly but it works)
    if ((currentDisplayItem != FixedDisplayItem::Time) && (currentDisplayItem != FixedDisplayItem::TimeSeconds))
    {
      statusLed.setOff();
    }
  }

  _setDisplay(&tcDisp, currentDisplayItem);

  DisplayManager::writeDisplay(tcDisp, statusLed);

  // determine the display duration of the current display item as well as the next item to display
  if (_mode == Application::OperatingMode::OperatingModeToggleDisplay)
  {
    switch (currentDisplayItem)
    {
      case FixedDisplayItem::Date:
      itemDisplayDuration = pSettings->getRawSetting(Settings::Setting::DateDisplayDuration);
      nextDisplayItem = FixedDisplayItem::Temperature;
      break;

      case FixedDisplayItem::Temperature:
      itemDisplayDuration = pSettings->getRawSetting(Settings::Setting::TemperatureDisplayDuration);
      nextDisplayItem = FixedDisplayItem::Time;
      break;

      default:
      itemDisplayDuration = pSettings->getRawSetting(Settings::Setting::TimeDisplayDuration);
      nextDisplayItem = FixedDisplayItem::Date;
    }
    // determine when we need to switch the display
    changeDisplayTime = _lastSwitchTime.addSeconds(itemDisplayDuration);

    // rotate the display if it's time to do so
    if (_currentTime >= changeDisplayTime)
    {
      Application::setViewMode(static_cast<ViewMode>(nextDisplayItem));

      _lastSwitchTime = _currentTime;
      // also trigger an animation if it is enabled
      if (pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::TriggerEffectOnRotate) == true)
      {
        // this will trigger an animation based on the condition below
        animationDisplayTime = _currentTime;
      }
    }
  }
  // run an animation if it's time to do so
  if (_currentTime >= animationDisplayTime)
  {
    Animator::run();

    _lastAnimationTime = _currentTime;

    Animator::setInitialDisplay(tcDisp);
    // only modify the final display if it's really necessary (due to rotation)
    if (currentDisplayItem != static_cast<FixedDisplayItem>(Application::getViewMode()))
    {
      _setDisplay(&tcDisp, nextDisplayItem);
    }

    Animator::setFinalDisplay(tcDisp);
  }
}


Display::dateTimeDisplaySelection TimeDateTempView::_getDisplaySelection(const FixedDisplayItem item)
{
  Settings *pSettings = Application::getSettingsPtr();
  uint8_t result = static_cast<uint8_t>(pSettings->getRawSetting(Settings::DateFormat));
  bool    display12Hour = pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::Display12Hour);

  if (item == FixedDisplayItem::Time)
  {
    if (display12Hour == true)
    {
      result = static_cast<uint8_t>(Display::dateTimeDisplaySelection::timeDisplay12Hour);
    }
    else
    {
      result = static_cast<uint8_t>(Display::dateTimeDisplaySelection::timeDisplay24Hour);
    }
  }

  return static_cast<Display::dateTimeDisplaySelection>(result);
}


void TimeDateTempView::_setDisplay(Display *display, const FixedDisplayItem item)
{
  Settings *pSettings = Application::getSettingsPtr();
  uint32_t dotsBitmap = _getDotsBitmap(),
           secondsSinceMidnight = _currentTime.secondsSinceMidnight(false);
  uint16_t duration = pSettings->getRawSetting(Settings::Setting::FadeDuration);
  uint8_t tubeIntensityBitmap = 0b111111;
  NixieGlyph dotOn(NixieGlyph::cGlyphMaximumIntensity, duration),
             dotOff(0, duration);

  // set the display bits as appropriate for time, date, or temperature
  if ((Hardware::rtcIsSet() == true) || (item == FixedDisplayItem::Temperature))
  {
    switch (item)
    {
      case FixedDisplayItem::Temperature:
        dotsBitmap = 0b100;   // upper right dot for temperature display
        display->setDisplayFromBytes(0, _currentTemperature, 0);
        tubeIntensityBitmap = 0b001100;
        break;

      case FixedDisplayItem::TimeSeconds:
        dotsBitmap = 0;       // no dots for seconds display
        display->setDisplayFromWord(secondsSinceMidnight);
        break;

      case FixedDisplayItem::Date:
        dotsBitmap = 0b1010;  // lower dots only for date display

      default:
        display->setDisplayFromDateTime(_currentTime, _getDisplaySelection(item));
    }
  }
  else
  {
    // make the LSbs blink if the clock is not set
    display->setDisplayFromBytes(_currentTime.second() & 1, _currentTime.second() & 1, _currentTime.second() & 1);
    // also make the status LED blink
    // DisplayManager::setStatusLedAutoRefreshing(true);
    // statusLed = tcDisp.getPixelRaw(0);
  }

  if (pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::MSDsOff) == true)
  {
    display->setMsdTubesOff(0b100000);  // only turn off tens of hours as appropriate
  }
  display->setDots(dotsBitmap, dotOn, dotOff);
  display->setTubeIntensities(NixieGlyph::cGlyphMaximumIntensity, 0, tubeIntensityBitmap);
  display->setTubeDurations(duration);
}


uint16_t TimeDateTempView::_getDotsBitmap()
{
  Settings *pSettings = Application::getSettingsPtr();
  uint32_t dotsBitmap = 0b1111;

  switch (pSettings->getRawSetting(Settings::Setting::ColonBehavior))
  {
    case 1:
      dotsBitmap = 0;
      break;

    case 2:
      if (Hardware::getPpsInputState() == false)
      {
        dotsBitmap = 0;
      }
      break;

    case 3:
      if (Hardware::getPpsInputState() == false)
      {
        dotsBitmap &= 0b1010;
      }
      break;

    case 4:
      if (Hardware::getPpsInputState() == false)
      {
        dotsBitmap &= 0b0101;
      }
      break;

    case 5:
      if (Hardware::getPpsInputState() == false)
      {
        dotsBitmap &= 0b1010;
      }
      else
      {
        dotsBitmap &= 0b0101;
      }
      break;

    default:
      break;
  }

  return dotsBitmap;
}


}
