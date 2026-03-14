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
#include "Dmx-512-Controller.h"
#include "NixieGlyph.h"
#include "Settings.h"
#include "TimeDateTempView.h"

namespace kbxTubeClock {

const uint8_t TimeDateTempView::cIntensityAdjustmentIncrement = 5;  // Increment by 5 on 0-255 scale (~2% steps)

TimeDateTempView::TimeDateTempView()
    : _currentTemperature(0),
      _currentTime(Application::dateTime()),
      _statusLed(RgbLed()),
      _animationElapsed(0),
      _switchElapsed(0),
      _mode(Application::OperatingMode::OperatingModeFixedDisplay),
      _prevDisplayItem(FixedDisplayItem::Time) {}

void TimeDateTempView::enter(const Settings::SettingDescriptor * /*descriptor*/, uint8_t /*relatedSetting*/,
                             uint8_t /*numSettings*/) {
  Settings *pSettings = Application::getSettingsPtr();

  _mode = Application::getOperatingMode();

  _currentTime = Application::dateTime();

  _animationElapsed = 0;

  _switchElapsed = 0;

  _prevDisplayItem = static_cast<FixedDisplayItem>(Application::getViewMode());

  _pmOnStale = true;

  _refreshStatusLed = true;

  DisplayManager::setStatusLedAutoRefreshing(
      pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::StatusLedAsAmPm));
}

bool TimeDateTempView::keyHandler(Keys::Key key) {
  FixedDisplayItem currentDisplayItem = static_cast<FixedDisplayItem>(Application::getViewMode());
  bool tick = false;

  if (key == Keys::Key::A) {
    if (currentDisplayItem != FixedDisplayItem::Time) {
      Application::setViewMode(static_cast<ViewMode>(FixedDisplayItem::Time));
      tick = true;
    } else if (currentDisplayItem != FixedDisplayItem::TimeSeconds) {
      Application::setViewMode(static_cast<ViewMode>(FixedDisplayItem::TimeSeconds));
      tick = true;
    }
  }

  if ((key == Keys::Key::B) && (currentDisplayItem != FixedDisplayItem::Date)) {
    Application::setViewMode(static_cast<ViewMode>(FixedDisplayItem::Date));
    tick = true;
  }

  if ((key == Keys::Key::C) && (currentDisplayItem != FixedDisplayItem::Temperature)) {
    Application::setViewMode(static_cast<ViewMode>(FixedDisplayItem::Temperature));
    tick = true;
  }

  if ((key == Keys::Key::D) && (Application::getIntensityAutoAdjust() == false)) {
    const int16_t intensity = Application::getIntensity() - cIntensityAdjustmentIncrement;
    if (intensity > 0) {
      Application::setIntensity(intensity);
      tick = true;
    } else {
      Application::setIntensity(0);
    }
  }

  if ((key == Keys::Key::U) && (Application::getIntensityAutoAdjust() == false)) {
    const int16_t intensity = Application::getIntensity() + cIntensityAdjustmentIncrement;
    if (intensity <= 255) {
      Application::setIntensity(intensity);
      tick = true;
    } else {
      Application::setIntensity(255);  // Maximum brightness
    }
  }

  if (key == Keys::Key::E) {
    Application::setOperatingMode(Application::OperatingMode::OperatingModeMainMenu);
    tick = true;
  }

  return tick;
}

void TimeDateTempView::loop() {
  Application::ExternalControl externalControlState = Application::getExternalControlState();
  Settings *pSettings = Application::getSettingsPtr();
  Display tcDisp;
  FixedDisplayItem currentDisplayItem = static_cast<FixedDisplayItem>(Application::getViewMode()),
                   nextDisplayItem = currentDisplayItem;

  if (currentDisplayItem != _prevDisplayItem) {
    _animationElapsed = 0;
    _switchElapsed = 0;
  }
  _prevDisplayItem = currentDisplayItem;
  bool displayFahrenheit =
      pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::DisplayFahrenheit);

  // update these only as needed -- keeps temperature from bouncing incessantly
  if (_currentTime != Application::dateTime()) {
    _currentTime = Application::dateTime();
    _currentTemperature = Hardware::temperature(displayFahrenheit);
    // advance counters on each tick
    _animationElapsed++;
    _switchElapsed++;
  }
  // set status LED to appropriate color if enabled as AM/PM indicator
  if (pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::StatusLedAsAmPm) == true) {
    RgbLed pmLed(pSettings->getRawSetting(Settings::Setting::PMIndicatorRedValue),
                 pSettings->getRawSetting(Settings::Setting::PMIndicatorGreenValue),
                 pSettings->getRawSetting(Settings::Setting::PMIndicatorBlueValue));
    bool pmOn = _currentTime.isPM() == true &&
                (currentDisplayItem == FixedDisplayItem::Time || currentDisplayItem == FixedDisplayItem::TimeSeconds);
    if (_pmOnStale || pmOn != _prevPmOn) {
      // When PM indicator turns on, LED auto-adjust follows tube auto-adjust.
      // When it turns off, LED auto-adjust is disabled so auto-brightness can't turn it back on.
      Application::setLedIntensityAutoAdjust(pmOn && Application::getIntensityAutoAdjust());
      _prevPmOn = pmOn;
      _pmOnStale = false;
    }
    pmLed.setIntensity(pmOn ? Application::getIntensity() : 0);
    _setStatusLed(pmLed);
  }

  _setDisplay(&tcDisp, currentDisplayItem);

  DisplayManager::writeDisplay(tcDisp);

  if (_refreshStatusLed == true) {
    DisplayManager::writeStatusLed(_statusLed);
    _refreshStatusLed = false;
  }

  // rotate the display if it's time to do so
  if ((_mode == Application::OperatingMode::OperatingModeToggleDisplay) &&
      (_switchElapsed >= _getItemDuration(currentDisplayItem))) {
    switch (currentDisplayItem) {
      case FixedDisplayItem::Date:
        nextDisplayItem = FixedDisplayItem::Temperature;
        break;

      case FixedDisplayItem::Temperature:
        nextDisplayItem = FixedDisplayItem::Time;
        break;

      default:
        nextDisplayItem = FixedDisplayItem::Date;
    }

    Application::setViewMode(static_cast<ViewMode>(nextDisplayItem));
    _switchElapsed = 0;
    // also trigger an animation if it is enabled
    if (pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::TriggerEffectOnRotate) ==
        true) {
      // this will trigger an animation based on the condition below
      _animationElapsed = pSettings->getRawSetting(Settings::Setting::EffectFrequency);
    }
  }
  // run an animation if it's time to do so
  if ((_animationElapsed >= pSettings->getRawSetting(Settings::Setting::EffectFrequency)) &&
      (externalControlState != Application::ExternalControl::Dmx512ExtControlEnum)) {
    Animator::run();

    _animationElapsed = 0;

    Animator::setInitialDisplay(tcDisp);
    // only modify the final display if it's really necessary (due to rotation)
    if (currentDisplayItem != static_cast<FixedDisplayItem>(Application::getViewMode())) {
      _setDisplay(&tcDisp, nextDisplayItem);
    }

    Animator::setFinalDisplay(tcDisp);
  }
}

uint16_t TimeDateTempView::_getItemDuration(const FixedDisplayItem item) {
  Settings *pSettings = Application::getSettingsPtr();

  switch (item) {
    case FixedDisplayItem::Date:
      return pSettings->getRawSetting(Settings::Setting::DateDisplayDuration);

    case FixedDisplayItem::Temperature:
      return pSettings->getRawSetting(Settings::Setting::TemperatureDisplayDuration);

    default:
      return pSettings->getRawSetting(Settings::Setting::TimeDisplayDuration);
  }
}

Display::dateTimeDisplaySelection TimeDateTempView::_getDisplaySelection(const FixedDisplayItem item) {
  Settings *pSettings = Application::getSettingsPtr();
  uint8_t result = static_cast<uint8_t>(pSettings->getRawSetting(Settings::DateFormat));
  bool display12Hour =
      pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::Display12Hour);

  if (item == FixedDisplayItem::Time) {
    if (display12Hour == true) {
      result = static_cast<uint8_t>(Display::dateTimeDisplaySelection::timeDisplay12Hour);
    } else {
      result = static_cast<uint8_t>(Display::dateTimeDisplaySelection::timeDisplay24Hour);
    }
  }

  return static_cast<Display::dateTimeDisplaySelection>(result);
}

void TimeDateTempView::_setDisplay(Display *display, const FixedDisplayItem item) {
  Application::ExternalControl externalControlState = Application::getExternalControlState();
  Settings *pSettings = Application::getSettingsPtr();
  uint32_t dotsBitmap = _getDotsBitmap();
  uint16_t duration = pSettings->getRawSetting(Settings::Setting::FadeDuration);
  bool msdsOff = pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::MSDsOff);
  uint8_t tubeIntensityBitmap = 0b111111;

  if (externalControlState == Application::ExternalControl::Dmx512ExtControlEnum) {
    duration = Dmx512Controller::fadeDuration();
  }

  NixieGlyph dotOn(NixieGlyph::cGlyphMaximumIntensity, duration), dotOff(0, duration);

  // set the display bits as appropriate for time, date, or temperature
  if ((Hardware::rtcIsSet() == true) || (item == FixedDisplayItem::Temperature)) {
    switch (item) {
      case FixedDisplayItem::Temperature:
        dotsBitmap = 0b100;  // upper right dot for temperature display
        display->setDisplayFromBytes(0, _currentTemperature, 0);
        tubeIntensityBitmap = 0b001100;
        break;

      case FixedDisplayItem::TimeSeconds:
        dotsBitmap = 0;  // no dots for seconds display
        display->setDisplayFromWord(_currentTime.secondsSinceMidnight(false));
        for (uint8_t i = display->cTubeCount - 1; msdsOff && i > 0; i--) {
          if (display->getTubeValue(i) == 0) {
            tubeIntensityBitmap &= ~(1 << i);  // blank tube if digit is 0
          } else {
            break;  // stop blanking tubes once we hit a non-zero digit
          }
        }
        break;

      case FixedDisplayItem::Date:
        dotsBitmap = 0b1010;  // lower dots only for date display
        [[fallthrough]];

      default:
        display->setDisplayFromDateTime(_currentTime, _getDisplaySelection(item));
        if (msdsOff == true && display->getTubeValue(display->cTubeCount - 1) == 0) {
          tubeIntensityBitmap = 0b11111;
        }
    }
  } else {
    // make the LSbs blink if the clock is not set
    display->setDisplayFromBytes(_currentTime.second() & 1, _currentTime.second() & 1, _currentTime.second() & 1);
    // also make the status LED blink
    // DisplayManager::setStatusLedAutoRefreshing(true);
    // statusLed = tcDisp.getPixelRaw(0);
  }

  if (pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::MSDsOff) == true) {
    display->setMsdTubesOff(0b100000);  // only turn off tens of hours as appropriate
  }
  display->setDots(dotsBitmap, dotOn, dotOff);
  display->setTubeIntensities(NixieGlyph::cGlyphMaximumIntensity, 0, tubeIntensityBitmap);
  display->setTubeDurations(duration);
}

void TimeDateTempView::_setStatusLed(const RgbLed &statusLed) {
  if (_statusLed == statusLed) {
    return;
  }
  _statusLed = statusLed;
  _refreshStatusLed = true;
}

uint16_t TimeDateTempView::_getDotsBitmap() {
  Settings *pSettings = Application::getSettingsPtr();
  uint32_t dotsBitmap = 0b1111;

  switch (pSettings->getRawSetting(Settings::Setting::ColonBehavior)) {
    case 1:
      dotsBitmap = 0;
      break;

    case 2:
      if (Hardware::getPpsInputState() == false) {
        dotsBitmap = 0;
      }
      break;

    case 3:
      if (Hardware::getPpsInputState() == false) {
        dotsBitmap &= 0b1010;
      }
      break;

    case 4:
      if (Hardware::getPpsInputState() == false) {
        dotsBitmap &= 0b0101;
      }
      break;

    case 5:
      if (Hardware::getPpsInputState() == false) {
        dotsBitmap &= 0b1010;
      } else {
        dotsBitmap &= 0b0101;
      }
      break;

    default:
      break;
  }

  return dotsBitmap;
}

}  // namespace kbxTubeClock
