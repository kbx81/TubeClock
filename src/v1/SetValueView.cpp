//
// kbx81's tube clock set an arbitrary value view class
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
#include "SetValueView.h"

namespace kbxTubeClock {

SetValueView::SetValueView()
    : _descriptor{0, 0, Settings::SettingTransform::None},
      _value{0, 0, 0, 0},
      _relatedSetting(Settings::Setting::TimerResetValue),
      _numSettings(1),
      _selectedItem(0) {}

void SetValueView::enter(const Settings::SettingDescriptor *descriptor, uint8_t relatedSetting,
                         uint8_t numSettings /* = 1 */) {
  _relatedSetting = relatedSetting;
  _numSettings = numSettings;
  _descriptor = *descriptor;

  Settings *pSettings = Application::getSettingsPtr();

  if (numSettings > 1) {
    _selectedItem = static_cast<int8_t>(Application::getViewMode());
    for (uint8_t i = 0; i < numSettings; i++) {
      _value[i] = pSettings->getRawSetting(relatedSetting + i);
    }
  } else {
    _selectedItem = 0;
    _value[0] = pSettings->getRawSetting(relatedSetting);
  }
}

bool SetValueView::keyHandler(Keys::Key key) {
  bool tick = true;

  if (key == Keys::Key::A) {
    Settings *pSettings = Application::getSettingsPtr();
    for (uint8_t i = 0; i < _numSettings; i++) {
      pSettings->setRawSetting(_relatedSetting + i, _value[i]);
      Application::notifySettingChanged(_relatedSetting + i);
    }
    DisplayManager::blink();
  }

  if (_numSettings == 1) {
    if (key == Keys::Key::B) {
      if (_value[0] == _descriptor.minValue) {
        tick = false;
      } else {
        _value[0] = _descriptor.minValue;
      }
    }

    if (key == Keys::Key::C) {
      if (_value[0] == _descriptor.maxOrMask) {
        tick = false;
      } else {
        _value[0] = _descriptor.maxOrMask;
      }
    }

    if (key == Keys::Key::D) {
      if (--_value[0] > _descriptor.maxOrMask || _value[0] < _descriptor.minValue) {
        _value[0] = _descriptor.minValue;
        tick = false;
      }
    }

    if (key == Keys::Key::U) {
      if (++_value[0] > _descriptor.maxOrMask) {
        _value[0] = _descriptor.maxOrMask;
        tick = false;
      }
    }
  } else {
    if (key == Keys::Key::B) {
      if (--_selectedItem < 0) {
        _selectedItem = static_cast<int8_t>(_numSettings) - 1;
      }
      Application::setViewMode(static_cast<ViewMode>(_selectedItem));
    }

    if (key == Keys::Key::C) {
      if (++_selectedItem >= static_cast<int8_t>(_numSettings)) {
        _selectedItem = 0;
      }
      Application::setViewMode(static_cast<ViewMode>(_selectedItem));
    }

    if (key == Keys::Key::D) {
      if (_value[_selectedItem] > _descriptor.minValue) {
        _value[_selectedItem]--;
      } else {
        tick = false;
      }
    }

    if (key == Keys::Key::U) {
      if (_value[_selectedItem] < _descriptor.maxOrMask) {
        _value[_selectedItem]++;
      } else {
        tick = false;
      }
    }
  }

  if (key == Keys::Key::E) {
    Application::setOperatingMode(Application::OperatingMode::OperatingModeMainMenu);
  }

  return tick;
}

void SetValueView::loop() {
  NixieGlyph dot(NixieGlyph::cGlyphMaximumIntensity);

  if (_numSettings == 1) {
    uint32_t dotsBitmap = 0;
    uint16_t displayedValue = _value[0];
    DateTime offsetTime(2001, 1, 1);  // ensure date after 2000/1/1 in case of subtraction

    switch (_descriptor.transform) {
      case Settings::SettingTransform::PlusOne:
        displayedValue += 1;
        break;

      case Settings::SettingTransform::TimeZone: {
        int16_t offsetInMinutes = (static_cast<int16_t>(_value[0]) - 56) * 15;
        if (offsetInMinutes < 0) {
          dotsBitmap = 0b0001;    // negative value/offset indicator
          offsetInMinutes *= -1;  // ensure correct display of the offset when it's negative
        }
        offsetTime = offsetTime.addSeconds(offsetInMinutes * 60);
        displayedValue = (offsetTime.hour() * 100) + offsetTime.minute();
        break;
      }

      default:
        break;
    }

    Display tcDisp(displayedValue);
    tcDisp.setDots(dotsBitmap, dot);

    tcDisp.setMsdTubesOff();

    if (displayedValue < 10000) {  // if value fits in 4 digits, show the setting number in tubes 4 and 5
      auto menuItemDisplayNumber = Application::getModeDisplayNumber(Application::getOperatingMode());
      tcDisp.setTubeToValue(4, menuItemDisplayNumber % 10);
      tcDisp.setTubeToValue(5, menuItemDisplayNumber / 10 % 10);
      tcDisp.setTubeIntensity(4, NixieGlyph::cGlyphMaximumIntensity);
      tcDisp.setTubeIntensity(5, NixieGlyph::cGlyphMaximumIntensity);
      tcDisp.setDot(1, NixieGlyph(NixieGlyph::cGlyphMaximumIntensity));
    }

    DisplayManager::writeDisplay(tcDisp);
    return;
  }

  _selectedItem = static_cast<int8_t>(Application::getViewMode());

  uint8_t itemNumber = static_cast<uint8_t>(_selectedItem) + 1;
  Display tcDisp;

  if (_descriptor.transform == Settings::SettingTransform::Calibration) {
    int16_t signedValue = static_cast<int16_t>(_value[_selectedItem]) - Settings::cCalibrationMidpoint;
    bool negative = (signedValue < 0);
    uint16_t absValue = negative ? static_cast<uint16_t>(-signedValue) : static_cast<uint16_t>(signedValue);

    // Dots: left colon (bits 0-1), right upper dot (bit 2) for negative
    uint32_t dotsBitmap = negative ? 0b0111 : 0b0011;

    tcDisp.setDisplayFromWord(absValue);
    tcDisp.setTubeToValue(5, 0);
    tcDisp.setTubeToValue(4, itemNumber);
    // Tubes 0, 1, 4, 5 on; tubes 2, 3 off
    tcDisp.setTubeIntensities(NixieGlyph::cGlyphMaximumIntensity, 0, 0b110011);
    tcDisp.setDots(dotsBitmap, dot, true);
  } else {
    tcDisp.setDisplayFromWord(_value[_selectedItem]);
    tcDisp.setTubeToValue(5, 0);
    tcDisp.setTubeToValue(4, itemNumber);
    // All tubes on: tube 5 = 0, tube 4 = item number, tubes 3-0 = value
    tcDisp.setTubeIntensities(NixieGlyph::cGlyphMaximumIntensity, 0, 0b111111);
    tcDisp.setDots(0b0011, dot, true);
  }

  DisplayManager::writeDisplay(tcDisp);
}

}  // namespace kbxTubeClock
