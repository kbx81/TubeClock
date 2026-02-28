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
  : _setValue(0),
    _maxValue(0),
    _relatedSetting(Settings::Setting::TimerResetValue),
    _settings(Settings())
{
}


void SetValueView::enter(uint8_t relatedSetting)
{
  _relatedSetting = relatedSetting;

  _settings = Application::getSettings();

  _setValue = _settings.getRawSetting(_relatedSetting);
  _maxValue = Settings::cSettingData[_relatedSetting];
}


bool SetValueView::keyHandler(Keys::Key key)
{
  bool tick = true;

  if (key == Keys::Key::A)
  {
    _settings.setRawSetting(_relatedSetting, _setValue);
    Application::setSettings(_settings);

    DisplayManager::blink();
  }

  if (key == Keys::Key::B)
  {
    if (_setValue == 1)
    {
      tick = false;
    }
    else
    {
      _setValue = 1;
    }
  }

  if (key == Keys::Key::C)
  {
    if (_setValue == _maxValue)
    {
      tick = false;
    }
    else
    {
      _setValue = _maxValue;
    }
  }

  if (key == Keys::Key::D)
  {
    if (--_setValue > _maxValue)
    {
      _setValue = 0;
      tick = false;
    }

    if (!_setValue && (_maxValue == 12 || _maxValue == 4))  // it's a month or an ordinal
    {
      _setValue = 1;
      tick = false;
    }
  }

  if (key == Keys::Key::U)
  {
    if (++_setValue > _maxValue)
    {
      _setValue = _maxValue;
      tick = false;
    }
  }

  if (key == Keys::Key::E)
  {
    Application::setOperatingMode(Application::OperatingMode::OperatingModeMainMenu);
  }

  return tick;
}


void SetValueView::loop()
{
  int16_t offsetInMinutes = (_setValue - 56) * 15;
  uint32_t dotsBitmap = 0;
  auto displayedValue = _setValue;
  DateTime offsetTime(2001, 1, 1);  // ensure date after 2000/1/1 in case of subtraction
  NixieGlyph dot(NixieGlyph::cGlyphMaximumIntensity);

  switch (_relatedSetting)
  {
    // this is because DMX-512 starts counting at one and not at zero
    case Settings::Setting::DmxAddress:
      displayedValue += 1;
      break;

    // make time zone display logical
    case Settings::Setting::TimeZone:
      if (offsetInMinutes < 0)
      {
        dotsBitmap = 0b0001;    // negative value/offset indicator
        offsetInMinutes *= -1;  // ensure correct display of the offset when it's negative
      }

      offsetTime = offsetTime.addSeconds(offsetInMinutes * 60);

      displayedValue = (offsetTime.hour() * 100) + offsetTime.minute();

      break;

    default:
      break;
  }

  // now we can create a new display object with the right colors and bitmask
  Display tcDisp(displayedValue);
  tcDisp.setDots(dotsBitmap, dot);

  if (_settings.getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::MSDsOff) == true)
  {
    tcDisp.setMsdTubesOff();
  }

  DisplayManager::writeDisplay(tcDisp);
}


}
