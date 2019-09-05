//
// kbx81's tube clock MainMenuView class
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
#include "MainMenuView.h"
#include "Settings.h"


namespace kbxTubeClock {


MainMenuView::MainMenuView()
  : _selectedMode(0),
    _previousMode(0)
{
}


void MainMenuView::enter()
{
  // correct the mode if it's out of range
  if ((_selectedMode == 0) || (_selectedMode > Application::OperatingMode::OperatingModeSlot8Time))
  {
    _selectedMode = Application::OperatingMode::OperatingModeFixedDisplay;
  }
  // if the clock is not set, select the SetClock mode automagically
  if (Hardware::rtcIsSet() == false)
  {
    _selectedMode = Application::OperatingMode::OperatingModeSetClock;
  }
  // if the clock is set but not the date, select the SetDate mode automagically
  else if (Application::dateTime().yearShort(false) == 0)
  {
    _selectedMode = Application::OperatingMode::OperatingModeSetDate;
  }

  DisplayManager::setStatusLedAutoRefreshing(false);
  // Make sure the status LEDs are off
  Hardware::setStatusLed(RgbLed());
}


bool MainMenuView::keyHandler(Keys::Key key)
{
  bool tick = true;

  if ((key == Keys::Key::A) &&
      (_selectedMode == static_cast<uint8_t>(Application::OperatingMode::OperatingModeTestDisplay) + 1))
  {
    Hardware::setStatusLed(Application::red);

    Hardware::eraseFlash(Settings::cSettingsFlashAddress);
    _selectedMode = 1;

    DisplayManager::doubleBlink();
    DisplayManager::doubleBlink();
    Hardware::setStatusLed(RgbLed());
  }

  if (key == Keys::Key::B)
  {
    Application::setOperatingMode(Application::OperatingMode::OperatingModeFixedDisplay);
  }

  if (key == Keys::Key::C)
  {
    Application::setOperatingMode(Application::OperatingMode::OperatingModeToggleDisplay);
  }

  if (key == Keys::Key::D)
  {
    if (--_selectedMode == 0)
    {
      _selectedMode = 1;

      tick = false;
    }
  }

  if (key == Keys::Key::U)
  {
    _selectedMode++;

    if (_selectedMode > static_cast<uint8_t>(Application::OperatingMode::OperatingModeTestDisplay) + 1)
    {
      _selectedMode = static_cast<uint8_t>(Application::OperatingMode::OperatingModeTestDisplay) + 1;

      tick = false;
    }
  }

  if (key == Keys::Key::E)
  {
    if (_selectedMode <= static_cast<uint8_t>(Application::OperatingMode::OperatingModeTestDisplay))
    {
      Application::setOperatingMode((Application::OperatingMode)_selectedMode);
    }
  }

  return tick;
}


void MainMenuView::loop()
{
  Display tcDisp;
  uint8_t modeIndicator = Application::getModeDisplayNumber(_selectedMode);

  if (modeIndicator == 0)
  {
    // fix up the last "Erase FLASH" mode value
    modeIndicator = 99;
  }

  tcDisp.setDisplayFromBytes(modeIndicator, 0 , 0, 0b1111);
  tcDisp.setDot(1, NixieGlyph(NixieGlyph::cGlyphMaximumIntensity));

  DisplayManager::writeDisplay(tcDisp);
}


}
