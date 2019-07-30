//
// kbx81's tube clock set bits view class
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
#include "DisplayManager.h"
#include "Hardware.h"
#include "Settings.h"
#include "SetBitsView.h"


namespace kbxTubeClock {

SetBitsView::SetBitsView()
  : _setBits(0),
    _bitsMask(0),
    _selectedBit(0),
    _mode(Application::OperatingMode::OperatingModeSetSystemOptions),
    _settings(Settings())
{
}


void SetBitsView::enter()
{
  _mode = Application::getOperatingMode();

  _settings = Application::getSettings();

  _setBits = _settings.getRawSetting(Application::getOperatingModeRelatedSetting(_mode));
  _bitsMask = Settings::cSettingData[Application::getOperatingModeRelatedSetting(_mode)];

  _selectedBit = 0;
}


bool SetBitsView::keyHandler(Keys::Key key)
{
  bool tick = true;

  if (key == Keys::Key::A)
  {
    _settings.setRawSetting(Application::getOperatingModeRelatedSetting(_mode), _setBits);
    Application::setSettings(_settings);

    DisplayManager::doubleBlink();
  }

  if (key == Keys::Key::B)
  {
    if ((_setBits & (1 << _selectedBit)) != 0)
    {
      _setBits &= ~(1 << _selectedBit);
    }
    else
    {
      tick = false;
    }
  }

  if (key == Keys::Key::C)
  {
    if ((_setBits & (1 << _selectedBit)) == 0)
    {
      _setBits |= (1 << _selectedBit);
    }
    else
    {
      tick = false;
    }
  }

  if (key == Keys::Key::D)
  {
    if (--_selectedBit > 15)
    {
      _selectedBit = 0;

      tick = false;
    }
  }

  if (key == Keys::Key::U)
  {
    if (++_selectedBit > __builtin_ctz(~_bitsMask) - 1)
    {
      _selectedBit = __builtin_ctz(~_bitsMask) - 1;

      tick = false;
    }
  }

  if (key == Keys::Key::E)
  {
    Application::setOperatingMode(Application::OperatingMode::OperatingModeMainMenu);
  }

  return tick;
}


void SetBitsView::loop()
{
  // create a new display object with the right colors and bitmask
  Display tcDisp(0, _selectedBit, (_setBits >> _selectedBit) & 1);
  tcDisp.setTubesOff(0b110010);

  DisplayManager::writeDisplay(tcDisp);
}


}
