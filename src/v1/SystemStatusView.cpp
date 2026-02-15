//
// kbx81's tube clock system status view class
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
#include "DS1722.h"
#include "DS3234.h"
#include "GpsReceiver.h"
#include "Hardware.h"
#include "LM74.h"
#include "Settings.h"
#include "SystemStatusView.h"

#if HARDWARE_VERSION == 1
  #include "Hardware_v1.h"
#else
  #error HARDWARE_VERSION must be defined with a value of 1
#endif


namespace kbxTubeClock {


SystemStatusView::SystemStatusView()
  : _selectedView(0)
{
}


void SystemStatusView::enter()
{
  DisplayManager::setStatusLedAutoRefreshing(true);
}


bool SystemStatusView::keyHandler(Keys::Key key)
{
  bool tick = false;

  if (key == Keys::Key::A)
  {
  }

  if (key == Keys::Key::B)
  {
  }

  if (key == Keys::Key::C)
  {
    if (_selectedView == DisplayItem::TubeLifetime)
    {
      Hardware::onTimeSecondsReset();
    }
  }

  if (key == Keys::Key::D)
  {
    if (--_selectedView < 0)
    {
      _selectedView = static_cast<uint8_t>(DisplayItem::VoltageBattery);
    }
    Application::setViewMode(static_cast<ViewMode>(_selectedView));
    tick = true;
  }

  if (key == Keys::Key::U)
  {
    if (++_selectedView > static_cast<uint8_t>(DisplayItem::VoltageBattery))
    {
      _selectedView = 0;
    }
    Application::setViewMode(static_cast<ViewMode>(_selectedView));
    tick = true;
  }

  if (key == Keys::Key::E)
  {
    Application::setOperatingMode(Application::OperatingMode::OperatingModeMainMenu);
  }

  return tick;
}


void SystemStatusView::loop()
{
  _selectedView = static_cast<int8_t>(Application::getViewMode());

  auto voltageBatt = Hardware::voltageBatt(),
       voltageVddA = Hardware::voltageVddA();
  uint32_t dotsBitmap = 0b0011;
  uint8_t tubeIntensityBitmap = 0b111111;
  RgbLed statusLed;
  NixieGlyph dot(NixieGlyph::cGlyphMaximumIntensity);
  Display tcDisp;

  switch (_selectedView)
  {
    case DisplayItem::TubeLifetime:
      dotsBitmap = 0;

      if (Hardware::onTimeSeconds() < Application::cSecondsInAnHour)
      {
        tcDisp.setDisplayFromWord(Hardware::onTimeSeconds());
      }
      else
      {
        tcDisp.setDisplayFromWord(Hardware::onTimeSeconds() / Application::cSecondsInAnHour);
      }
      // If the PPS trigger is selected, light the status LED when the PPS signal is high
      if (Hardware::getPeripheralRefreshTrigger() == Hardware::PeripheralRefreshTrigger::PpsExti)
      {
        if (Hardware::getPpsInputState() == true)
        {
          statusLed = Application::nixieOrange;
        }
      }
      break;

    case DisplayItem::GpsStatus:
      if (GpsReceiver::isConnected() == true)
      {
        tubeIntensityBitmap = 0b110011;
        tcDisp.setDisplayFromWord(GpsReceiver::getSatellitesInView());
        if (GpsReceiver::isValid() == true)
        {
          statusLed = Application::green;
        }
        else
        {
          statusLed = Application::red;
        }
      }
      else
      {
        tubeIntensityBitmap = 0b110000;
      }
      break;

    case DisplayItem::StatusDS3234:
      tubeIntensityBitmap = 0b111111;
      tcDisp.setTubeToValue(0, 4);
      tcDisp.setTubeToValue(1, 3);
      tcDisp.setTubeToValue(2, 2);
      tcDisp.setTubeToValue(3, 3);
      if (DS3234::isConnected() == true)
      {
        statusLed = Application::green;
      }
      else
      {
        statusLed = Application::red;
      }
      break;

    case DisplayItem::StatusDS1722:
      tubeIntensityBitmap = 0b111111;
      tcDisp.setTubeToValue(0, 2);
      tcDisp.setTubeToValue(1, 2);
      tcDisp.setTubeToValue(2, 7);
      tcDisp.setTubeToValue(3, 1);
      if (DS1722::isConnected() == true)
      {
        statusLed = Application::green;
      }
      else
      {
        statusLed = Application::red;
      }
      break;

    case DisplayItem::StatusLM74:
      tubeIntensityBitmap = 0b110011;
      tcDisp.setTubeToValue(0, 4);
      tcDisp.setTubeToValue(1, 7);
      if (LM74::isConnected() == true)
      {
        statusLed = Application::green;
      }
      else
      {
        statusLed = Application::red;
      }
      break;

    case DisplayItem::VoltageVddA:
      dotsBitmap = 0b1000011;
      tcDisp.setDisplayFromWord(voltageVddA);
      break;

    case DisplayItem::VoltageBattery:
    default:
      dotsBitmap = 0b1000011;
      tcDisp.setDisplayFromWord(voltageBatt);
  }

  if (_selectedView != DisplayItem::TubeLifetime)
  {
    tcDisp.setTubeToValue(4, _selectedView);
  }
  tcDisp.setTubeIntensities(NixieGlyph::cGlyphMaximumIntensity, 0, tubeIntensityBitmap);
  tcDisp.setDots(dotsBitmap, dot, true);
  DisplayManager::writeDisplay(tcDisp, statusLed);
}


}
