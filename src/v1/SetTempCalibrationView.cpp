//
// kbx81's tube clock set temperature calibration view class
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
#include "SetTempCalibrationView.h"


namespace kbxTubeClock {


static const Settings::Setting cCalibrationSettings[SetTempCalibrationView::cSensorCount] = {
  Settings::Setting::TemperatureCalibrationSTM32,
  Settings::Setting::TemperatureCalibrationDS3234,
  Settings::Setting::TemperatureCalibrationDS1722,
  Settings::Setting::TemperatureCalibrationLM74
};


SetTempCalibrationView::SetTempCalibrationView()
  : _selectedSensor(0),
    _calibrationValue{0, 0, 0, 0}
{
}


void SetTempCalibrationView::enter(uint8_t /*relatedSetting*/)
{
  _selectedSensor = static_cast<int8_t>(Application::getViewMode());

  Settings* pSettings = Application::getSettingsPtr();

  for (uint8_t i = 0; i < cSensorCount; i++)
  {
    _calibrationValue[i] = (int16_t)pSettings->getRawSetting(cCalibrationSettings[i])
                           - Settings::cCalibrationMidpoint;
  }
}


bool SetTempCalibrationView::keyHandler(Keys::Key key)
{
  bool tick = true;

  if (key == Keys::Key::A)
  {
    Settings* pSettings = Application::getSettingsPtr();

    for (uint8_t i = 0; i < cSensorCount; i++)
    {
      pSettings->setRawSetting(cCalibrationSettings[i],
        static_cast<uint16_t>(_calibrationValue[i] + Settings::cCalibrationMidpoint));
      Application::notifySettingChanged(cCalibrationSettings[i]);
    }

    DisplayManager::blink();
  }

  if (key == Keys::Key::B)
  {
    if (--_selectedSensor < 0)
    {
      _selectedSensor = cSensorCount - 1;
    }
    Application::setViewMode(static_cast<ViewMode>(_selectedSensor));
  }

  if (key == Keys::Key::C)
  {
    if (++_selectedSensor >= cSensorCount)
    {
      _selectedSensor = 0;
    }
    Application::setViewMode(static_cast<ViewMode>(_selectedSensor));
  }

  if (key == Keys::Key::D)
  {
    if (_calibrationValue[_selectedSensor] > -Settings::cCalibrationMidpoint)
    {
      _calibrationValue[_selectedSensor]--;
    }
    else
    {
      tick = false;
    }
  }

  if (key == Keys::Key::U)
  {
    if (_calibrationValue[_selectedSensor] < Settings::cCalibrationMidpoint)
    {
      _calibrationValue[_selectedSensor]++;
    }
    else
    {
      tick = false;
    }
  }

  if (key == Keys::Key::E)
  {
    Application::setOperatingMode(Application::OperatingMode::OperatingModeMainMenu);
  }

  return tick;
}


void SetTempCalibrationView::loop()
{
  _selectedSensor = static_cast<int8_t>(Application::getViewMode());

  int16_t value = _calibrationValue[_selectedSensor];
  bool negative = (value < 0);
  uint16_t absValue = negative ? -value : value;

  // Dots: left colon (bits 0-1), right upper dot (bit 2) for negative
  uint32_t dotsBitmap = negative ? 0b0111 : 0b0011;

  NixieGlyph dot(NixieGlyph::cGlyphMaximumIntensity);
  Display tcDisp;

  // Display absolute value on tubes 1-0 
  tcDisp.setDisplayFromWord(absValue);

  // Sensor number (1-4) on tubes 0-1
  uint8_t sensorNumber = _selectedSensor + 1;
  tcDisp.setTubeToValue(5, 0);
  tcDisp.setTubeToValue(4, sensorNumber);

  // Tubes 0, 1, 4, 5 on; tubes 2, 3 off
  tcDisp.setTubeIntensities(NixieGlyph::cGlyphMaximumIntensity, 0, 0b110011);
  tcDisp.setDots(dotsBitmap, dot, true);

  DisplayManager::writeDisplay(tcDisp);
}


}
