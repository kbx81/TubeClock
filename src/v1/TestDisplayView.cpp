//
// kbx81's tube clock TestDisplayView class
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
#include "Display.h"
#include "DisplayManager.h"
#include "Settings.h"
#include "TestDisplayView.h"


namespace kbxTubeClock {


// maximum value for _displayedColor (2 = Blue)
//
const uint8_t TestDisplayView::cMaxColor = 2;


TestDisplayView::TestDisplayView()
  : _displayedColor(0),
    _intensity(0),
    _mode(Application::OperatingMode::OperatingModeTestDisplay)
{
}


void TestDisplayView::enter()
{
  _mode = Application::getOperatingMode();

  DisplayManager::setStatusLedAutoRefreshing(true);

  // make the display bright so we can see the colors we're working with
  Application::setIntensity(_intensity);
}


bool TestDisplayView::keyHandler(Keys::Key key)
{
  bool tick = true;

  if (key == Keys::Key::A)
  {
    _intensity = 0;
    // set the new display intensity
    Application::setIntensity(_intensity);
  }

  if (key == Keys::Key::B)
  {
    _intensity = 10000;
    // set the new display intensity
    Application::setIntensity(_intensity);
  }

  if (key == Keys::Key::C)
  {
    if (++_displayedColor > cMaxColor)
    {
      _displayedColor = 0;
    }
  }

  if (key == Keys::Key::D)
  {
    // set the new display intensity
    Application::setIntensity(--_intensity);
  }

  if (key == Keys::Key::U)
  {
    // set the new display intensity
    Application::setIntensity(++_intensity);
  }

  if (key == Keys::Key::E)
  {
    Application::setOperatingMode(Application::OperatingMode::OperatingModeMainMenu);
  }

  return tick;
}


void TestDisplayView::loop()
{
  const uint16_t displayBitMask = 0;
  const uint16_t testIntensity = 512;
  uint16_t red = 0, green = 0, blue = 0;

  switch (_displayedColor)
  {
    case 2:
      blue = testIntensity;
      break;

    case 1:
      green = testIntensity;
      break;

    default:
      red = testIntensity;
  }

  // now we can create a new display object with the right colors and bitmask
  RgbLed  testColor(red, green, blue);
  Display bcDisp(displayBitMask);

  DisplayManager::writeDisplay(bcDisp);
  // Hardware::setStatusLed(testColor);
}


}
