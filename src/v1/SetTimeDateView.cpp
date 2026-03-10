//
// kbx81's tube clock set time/date view class
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
#include "Application.h"
#include "DateTime.h"
#include "DisplayManager.h"
#include "Hardware.h"
#include "Settings.h"
#include "SetTimeDateView.h"

namespace kbxTubeClock {

// Scale factor for non-selected digits/LED (20% of full intensity)
const uint16_t SetTimeDateView::cLowlightPercentage = RgbLed::pct(20);

SetTimeDateView::SetTimeDateView()
    : _selectedItem(0),
      _workingDateTime(),
      _mode(Application::OperatingMode::OperatingModeSetClock),
      _relatedSetting(0) {}

void SetTimeDateView::enter(const Settings::SettingDescriptor * /*descriptor*/, uint8_t relatedSetting,
                            uint8_t /*numSettings*/) {
  _mode = Application::getOperatingMode();
  _relatedSetting = relatedSetting;

  switch (_mode) {
    case Application::OperatingMode::OperatingModeSetClock:
    case Application::OperatingMode::OperatingModeSetDate:
      _workingDateTime = Application::dateTime();
      break;

    default:
      _workingDateTime = Application::getSettingsPtr()->getTime(_relatedSetting);
      break;
  }

  // ...so we can highlight digits as necessary
  Application::setIntensity(NixieGlyph::cGlyphMaximumIntensity);

  DisplayManager::setStatusLedAutoRefreshing(Application::getSettingsPtr()->getSetting(
      Settings::Setting::SystemOptions, Settings::SystemOptionsBits::Display12Hour));
}

bool SetTimeDateView::keyHandler(Keys::Key key) {
  int16_t year = _workingDateTime.year();
  int8_t month = _workingDateTime.month(), day = _workingDateTime.day(), hour = _workingDateTime.hour(),
         minute = _workingDateTime.minute(), second = _workingDateTime.second();
  bool tick = true;
  DateTime setDateTime = Application::dateTime(), workingDateTime = _workingDateTime;

  if (key == Keys::Key::A) {
    if (_mode == Application::OperatingMode::OperatingModeSetDate) {
      setDateTime.setDate(year, month, day);
    } else {
      setDateTime.setTime(hour, minute, second);
    }

    switch (_mode) {
      case Application::OperatingMode::OperatingModeSetClock:
      case Application::OperatingMode::OperatingModeSetDate:
        Application::setDateTime(setDateTime);
        break;

      default:
        Application::getSettingsPtr()->setTime(_relatedSetting, setDateTime);
        Application::notifySettingsChanged();
        break;
    }

    DisplayManager::blink();
  }

  if (key == Keys::Key::B) {
    if (--_selectedItem < 0) {
      _selectedItem = 2;
    }
  }

  if (key == Keys::Key::C) {
    if (++_selectedItem > 2) {
      _selectedItem = 0;
    }
  }

  if (key == Keys::Key::D) {
    // the setDate and setTime methods validate everything so we'll be lazy here
    if (_mode == Application::OperatingMode::OperatingModeSetDate) {
      switch (_selectedItem) {
        case HourYear:
          --year;
          break;

        case MinuteMonth:
          --month;
          break;

        default:
          --day;
      }
      _workingDateTime.setDate(year, month, day);
    } else {
      switch (_selectedItem) {
        case HourYear:
          --hour;
          break;

        case MinuteMonth:
          --minute;
          break;

        default:
          --second;
      }
      // this 'if' is just to prevent rolling over from 0 to 59
      if ((hour >= 0) && (minute >= 0) && (second >= 0)) {
        _workingDateTime.setTime(hour, minute, second);
      }
    }
    // should we tick?
    if (workingDateTime == _workingDateTime) {
      tick = false;
    }
  }

  if (key == Keys::Key::U) {
    // the setDate and setTime methods validate everything so we'll be lazy here
    if (_mode == Application::OperatingMode::OperatingModeSetDate) {
      switch (_selectedItem) {
        case HourYear:
          ++year;
          break;

        case MinuteMonth:
          ++month;
          break;

        default:
          ++day;
      }
      _workingDateTime.setDate(year, month, day);
    } else {
      switch (_selectedItem) {
        case HourYear:
          ++hour;
          break;

        case MinuteMonth:
          ++minute;
          break;

        default:
          ++second;
      }
      _workingDateTime.setTime(hour, minute, second);
    }
    // should we tick?
    if (workingDateTime == _workingDateTime) {
      tick = false;
    }
  }

  if (key == Keys::Key::E) {
    Application::setOperatingMode(Application::OperatingMode::OperatingModeMainMenu);
  }

  return tick;
}

void SetTimeDateView::loop() {
  RgbLed statusLed;
  Display::dateTimeDisplaySelection dateTimeItem = Display::dateTimeDisplaySelection::dateDisplayYYMMDD;
  Settings *pSettings = Application::getSettingsPtr();
  bool display12Hour =
      pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::Display12Hour);
  uint32_t dotsBitmap = 0b1111;
  uint8_t highlightStart = _selectedItem;
  NixieGlyph dot(NixieGlyph::cGlyphMaximumIntensity);
  NixieTube tube;
  Display tcDisp(_workingDateTime.hour(false, display12Hour), _workingDateTime.minute(), _workingDateTime.second());

  if (_mode == Application::OperatingMode::OperatingModeSetDate) {
    dotsBitmap = 0b1010;

    switch (pSettings->getRawSetting(Settings::DateFormat)) {
      case 2:
        dateTimeItem = Display::dateTimeDisplaySelection::dateDisplayMMDDYY;
        // determine what to highlight
        switch (_selectedItem) {
          case HourYear:
            highlightStart = 0;
            break;

          case MinuteMonth:
            highlightStart = 2;
            break;

          default:
            highlightStart = 1;
        }
        break;

      case 1:
        dateTimeItem = Display::dateTimeDisplaySelection::dateDisplayDDMMYY;
        // determine what to highlight
        switch (_selectedItem) {
          case HourYear:
            highlightStart = 0;
            break;

          case MinuteMonth:
            highlightStart = 1;
            break;

          default:
            highlightStart = 2;
        }
        break;

        // default:
        // dateTimeItem = Display::dateTimeDisplaySelection::dateDisplayYYMMDD;
        // no need to adjust highlightStart as _selectedItem directly corresponds to this arrangement
    }
    tcDisp.setDisplayFromDateTime(_workingDateTime, dateTimeItem);
  } else if (display12Hour == true) {
    if (_workingDateTime.isPM() == true) {
      statusLed.setRGB(pSettings->getRawSetting(Settings::Setting::PMIndicatorRedValue),
                       pSettings->getRawSetting(Settings::Setting::PMIndicatorGreenValue),
                       pSettings->getRawSetting(Settings::Setting::PMIndicatorBlueValue));
    } else {
      statusLed = RgbLed();
    }
    // highlight/lowlight as appropriate
    if (_selectedItem != HourYear) {
      statusLed.adjustIntensity(cLowlightPercentage);
    }
  }
  // set tube highlights/lowlights as appropriate
  const uint8_t highlightTube = highlightStart << 1;
  for (uint8_t t = 0; t < Display::cTubeCount; t++) {
    if ((t != highlightTube) && (t != highlightTube + 1)) {
      tube = tcDisp.getTubeRaw(t);
      tube.adjustIntensityAll(cLowlightPercentage);
      tcDisp.setTubeFromRaw(t, tube);
    }
  }

  tcDisp.setDots(dotsBitmap, dot, true);

  DisplayManager::writeDisplay(tcDisp, statusLed);
}

}  // namespace kbxTubeClock
