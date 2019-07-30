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
#include "Hardware.h"
#include "GpsReceiver.h"
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
    tick = true;
  }

  if (key == Keys::Key::U)
  {
    if (++_selectedView > static_cast<uint8_t>(DisplayItem::VoltageBattery))
    {
      _selectedView = 0;
    }
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
      break;

    case DisplayItem::PeripheralStatus:
      if (Hardware::getTempSensorType() == Hardware::TempSensorType::DS1722)
      {
        tcDisp.setTubeToValue(0, 1);
      }

      if (Hardware::getTempSensorType() == Hardware::TempSensorType::LM74)
      {
        tcDisp.setTubeToValue(1, 1);
      }

      if (Hardware::getRTCType() == Hardware::RtcType::DS323x)
      {
        tcDisp.setTubeToValue(2, 1);
      }

      if (GpsReceiver::isConnected() == true)
      {
        tcDisp.setTubeToValue(3, 1);
      }

      if (Hardware::getPeripheralRefreshTrigger() == Hardware::PeripheralRefreshTrigger::PpsExti)
      {
        if (Hardware::getPpsInputState() == true)
        {
          statusLed = Application::green;
        }
      }
      else
      {
        statusLed = Application::yellow;
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
