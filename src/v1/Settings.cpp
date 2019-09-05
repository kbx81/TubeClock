//
// kbx81's tube clock settings class
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
#include "Hardware.h"
#include "RgbLed.h"
#include "Settings.h"


namespace kbxTubeClock {

  // where settings will be read/written in FLASH
  const uint32_t Settings::cSettingsFlashAddress = 0x0801f000;

  // conatins a mask for bitfields and a maximum value for other types of data
  const uint16_t Settings::cSettingData[] = { 0x03ff,   // SystemOptions
                                              0x07ff,   // BeepStates
                                              0x07ff,   // BlinkStates
                                              0x00ff,   // OnOffStates
                                              300,      // TimeDisplayDuration
                                              300,      // DateDisplayDuration
                                              300,      // TemperatureDisplayDuration
                                              1000,     // FadeDuration
                                              12,       // DstBeginMonth
                                              4,        // DstBeginDowOrdinal
                                              12,       // DstEndMonth
                                              4,        // DstEndDowOrdinal
                                              6,        // DstSwitchDayOfWeek
                                              23,       // DstSwitchHour
                                              1000,     // EffectDuration
                                              43200,    // EffectFrequency
                                              1000,     // MinimumIntensity
                                              7,        // BeeperVolume
                                              31,       // TemperatureCalibration
                                              100,      // DisplayRefreshInterval
                                              2,        // DateFormat
                                              112,      // TimeZone
                                              5,        // ColonBehavior
                                              65535,    // TimerResetValue
                                              512 - 8 }; // DmxAddress

  Settings::Settings()
  : _crc(0)
  {
    initialize();
  }


  void Settings::initialize()
  {
    // set default time slots every three hours starting at 0100 hours
    for (uint8_t i = static_cast<uint8_t>(Slot::Slot1); i <= static_cast<uint8_t>(Slot::Slot8); i++)
    {
      _time[i].setTime((3 * i) + 1, 30, 0);
    }

    _setting[static_cast<uint8_t>(SystemOptions)] = 0b1011100000;
    _setting[static_cast<uint8_t>(BeepStates)] = 0b11100000000;
    _setting[static_cast<uint8_t>(BlinkStates)] = 0b11100000000;
    _setting[static_cast<uint8_t>(OnOffStates)] = 0b11111111;
    _setting[static_cast<uint8_t>(TimeDisplayDuration)] = 24;
    _setting[static_cast<uint8_t>(DateDisplayDuration)] = 3;
    _setting[static_cast<uint8_t>(TemperatureDisplayDuration)] = 3;
    _setting[static_cast<uint8_t>(FadeDuration)] = 100;
    _setting[static_cast<uint8_t>(DstBeginMonth)] = 3;
    _setting[static_cast<uint8_t>(DstBeginDowOrdinal)] = 2;
    _setting[static_cast<uint8_t>(DstEndMonth)] = 11;
    _setting[static_cast<uint8_t>(DstEndDowOrdinal)] = 1;
    _setting[static_cast<uint8_t>(DstSwitchDayOfWeek)] = 0;
    _setting[static_cast<uint8_t>(DstSwitchHour)] = 2;
    _setting[static_cast<uint8_t>(EffectDuration)] = 500;
    _setting[static_cast<uint8_t>(EffectFrequency)] = 300;
    _setting[static_cast<uint8_t>(MinimumIntensity)] = 30;
    _setting[static_cast<uint8_t>(BeeperVolume)] = 7;
    _setting[static_cast<uint8_t>(TemperatureCalibration)] = 10;
    _setting[static_cast<uint8_t>(DisplayRefreshInterval)] = 0;
    _setting[static_cast<uint8_t>(DateFormat)] = 0;
    _setting[static_cast<uint8_t>(TimeZone)] = 56;
    _setting[static_cast<uint8_t>(ColonBehavior)] = 0;
    _setting[static_cast<uint8_t>(TimerResetValue)] = 30;
    _setting[static_cast<uint8_t>(DmxAddress)] = 0;
}


  bool Settings::loadFromFlash()
  {
    Hardware::readFlash(cSettingsFlashAddress, sizeof(Settings), (uint8_t*)this);

    uint32_t loadedCrc = _crc;  // save before zeroing for the CRC computation

    _crc = 0;   // always compute the CRC with this as zero

    if (loadedCrc != Hardware::getCRC((uint32_t*)this, sizeof(Settings) / 4))
    {
      initialize();

      return false;
    }
    return true;
  }


  uint32_t Settings::saveToFlash()
  {
    _crc = 0;   // always compute the CRC with this as zero

    // compute the new CRC value and save it before we write it all to FLASH
    _crc = Hardware::getCRC((uint32_t*)this, sizeof(Settings) / 4);

    return Hardware::writeFlash(cSettingsFlashAddress, (uint8_t*)this, sizeof(Settings));
  }


  bool Settings::getSetting(const uint8_t setting, const uint8_t item)
  {
    if (setting <= static_cast<uint8_t>(Setting::DmxAddress))
    {
      return (_setting[setting] & (1 << item));
    }
    return false;
  }


  void Settings::setSetting(const uint8_t setting, const uint8_t item, const bool state)
  {
    if (setting <= static_cast<uint8_t>(Setting::DmxAddress))
    {
      if (state)
      {
        _setting[setting] |= (1 << item);
      }
      else
      {
        _setting[setting] &= ~(1 << item);
      }
    }
  }


  uint16_t Settings::getRawSetting(const uint8_t setting)
  {
    if (setting <= static_cast<uint8_t>(Setting::DmxAddress))
    {
      return _setting[setting];
    }
    return 0;
  }


  void Settings::setRawSetting(const uint8_t setting, const uint16_t value)
  {
    if (setting <= static_cast<uint8_t>(Setting::DmxAddress))
    {
      _setting[setting] = value;
    }
  }


  DateTime Settings::getTime(const Slot setting)
  {
    return getTime(static_cast<uint8_t>(setting));
  }


  DateTime Settings::getTime(const uint8_t setting)
  {
    if (setting <= static_cast<uint8_t>(Slot::Slot8))
    {
      return _time[static_cast<uint8_t>(setting)];
    }
    return _time[static_cast<uint8_t>(Slot::Slot1)];
  }


  void Settings::setTime(const Slot setting, const DateTime time)
  {
    return setTime(static_cast<uint8_t>(setting), time);
  }


  void Settings::setTime(const uint8_t setting, const DateTime time)
  {
    if (setting <= static_cast<uint8_t>(Slot::Slot8))
    {
      _time[static_cast<uint8_t>(setting)] = time;
    }
  }


  bool Settings::hvState()
  {
    DateTime currentTime = Application::dateTime();
    int32_t i, delta,
            currentSecondsSinceMidnight = (int32_t)currentTime.secondsSinceMidnight(false),
            // minSecs = 0,  // first/earliest slot
            prevSecs = -Application::cSecondsInADay, // slot most recently passed
            // nextSecs = Application::cSecondsInADay,  // next slot that will pass
            maxSecs = 0;  // latest/last slot
    uint8_t // minSlot = 0,  // index of first/earliest slot
            prevSlot = 0, // index of slot most recently passed
            // nextSlot = 0, // index of next slot that will pass
            maxSlot = 0;  // index of latest/last slot

    if (Hardware::rtcIsSet() == true)
    {
      // find minimum, maximum, just passed, and next to pass values
      for (i = static_cast<uint8_t>(Slot::Slot1); i <= static_cast<uint8_t>(Slot::Slot8); i++)
      {
        delta = (int32_t)_time[i].secondsSinceMidnight(false) - currentSecondsSinceMidnight;

        // now, if delta is less than zero, slot i is before the current time;
        //   if delta is greater than zero, slot i is after the current time.
        // next we'll sort out which is just before and which is just after

        // if (((int32_t)_time[i].secondsSinceMidnight(false) < minSecs) || !i)
        // {
        //   minSecs = _time[i].secondsSinceMidnight(false);
        //   minSlot = i;
        // }

        if ((delta <= 0) && (delta > prevSecs))
        {
          prevSecs = delta;
          prevSlot = i;
        }

        // if ((delta > 0) && (delta < nextSecs))
        // {
        //   nextSecs = delta;
        //   nextSlot = i;
        // }

        if ((int32_t)_time[i].secondsSinceMidnight(false) > maxSecs)
        {
          maxSecs = _time[i].secondsSinceMidnight(false);
          maxSlot = i;
        }
      }
      // if no slots have been passed yet, then the last one passed is the max/latest one
      if (prevSecs == (int32_t)-Application::cSecondsInADay)
      {
        prevSlot = maxSlot;
      }

      return getSetting(Setting::OnOffStates, prevSlot);
    }
    else
    {
      return true;
    }
  }


}
