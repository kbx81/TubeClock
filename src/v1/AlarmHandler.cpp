//
// kbx81's tube clock alarm handler
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
#include <cstdint>

#include <libopencm3/stm32/rtc.h>
#include "AlarmHandler.h"
#include "Application.h"
#include "DateTime.h"
#include "DisplayManager.h"
#include "Hardware.h"
#include "Keys.h"
#include "Pitches.h"
#include "Settings.h"


namespace kbxTubeClock {

namespace AlarmHandler {


// Latching external alarm input number
//
const uint8_t cLatchingAlarmInputNumber = 0;

// Momentary external alarm input number
//
const uint8_t cMomentaryAlarmInputNumber = 1;

// Alarm tone durations and frequencies
//
const uint16_t cHourlyAlarmToneDuration = 75;
// const uint16_t cHourlyAlarmToneFrequencies[] = { NOTE_D6, NOTE_A6 };
const uint16_t cHourlyAlarmToneFrequencies[] = { NOTE_E6, NOTE_E7 };
// Be creative and create your own melody!
//   *** Array lengths MUST MATCH ***
// const uint16_t cAlarmToneDurations[]   = { 75, 75, 75, 75, 75, 75, 75, 700 };
// const uint16_t cAlarmToneFrequencies[] = { NOTE_D7, NOTE_REST, NOTE_D7, NOTE_REST, NOTE_D7, NOTE_REST, NOTE_D7, NOTE_REST };
const uint16_t cAlarmToneDurations[]   = { 150, 300, 50, 300, 150, 150, 150, 150, 150, 700 };
const uint16_t cAlarmToneFrequencies[] = { NOTE_C7, NOTE_G6, NOTE_REST, NOTE_G6, NOTE_A6, NOTE_G6, NOTE_REST, NOTE_B6, NOTE_C7, NOTE_REST };

// Number of time slot alarms available
//
const uint8_t cTimeSlotAlarmCount = 8;

// Number of external alarms available
//
const uint8_t cExtAlarmCount = 3;

// Total number of alarms available
//
const uint8_t cAlarmCount = cTimeSlotAlarmCount + cExtAlarmCount;

// Bit indicating the timer/counter alarm should sound
//
const uint8_t cExtAlarmShift = 8;

// Bit indicating the timer/counter alarm should sound
//
const uint8_t cTimerCounterAlarmBit = 8;

// Bit indicating the latching (external) alarm should sound
//
const uint8_t cExtLatchingAlarmBit = 9;

// Bit indicating the momentary (external) alarm should sound
//
const uint8_t cExtMomentaryAlarmBit = 10;

// Indicates which alarm(s) is/are active. Low 8 bits correspond to time slots
//
uint16_t _activeAlarms = 0;

// Counter for alarm beeps
//
uint8_t _beepCounter = 0;

// Indicates hourly alarm should sound
//  Initial state of 'true' produces a nice start-up beep
bool _hourlyAlarmActive = true;

// The state of the pin the last time we checked it
//
bool _previousExtMomentaryAlarmPinState = false;

// Indicates the last hour during which the hourly alarm sounded
//  Initial value is not a valid hour so it will always fire after startup
uint8_t _lastHourlyBeepHour = 255;

// Times at which each alarm was last acknowledged
//
DateTime _ackTime[cTimeSlotAlarmCount];


// checks if any alarm boundaries have been crossed
// returns a bitmap indicating which alarms are active
uint16_t _getActiveTimeSlotAlarms()
{
  DateTime slot, current = Application::dateTime();
  Settings *pSettings = Application::getSettingsPtr();
  uint16_t activeAlarms = 0;
  // first we'll look at the time slots
  for (uint8_t i = static_cast<uint8_t>(Settings::Slot::Slot1); i <= static_cast<uint8_t>(Settings::Slot::Slot8); i++)
  {
    // first check if the alarm is enabled
    if ((pSettings->getSetting(Settings::Setting::BeepStates, i) == true)
     || (pSettings->getSetting(Settings::Setting::BlinkStates, i) == true))
    {
      // get the current slot's time
      slot = pSettings->getTime(i);
      // set the alarm slot's date to today so we can compare them
      slot.setDate(current.year(false), current.month(false), current.day(false));
      // if the time it was last acknowledged is before the alarm slot's time...
      if (_ackTime[i] < slot)
      {
        // ...we have an active alarm
        activeAlarms |= (1 << i);
      }
    }
  }
  return activeAlarms;
}


// returns a bitmap indicating which external alarms are active
//
uint16_t _getActiveExternalAlarms()
{
  uint16_t activeAlarms = _activeAlarms;
  bool latchingAlarmPinState = Hardware::alarmInput(cLatchingAlarmInputNumber),
       momentaryAlarmPinState = Hardware::alarmInput(cMomentaryAlarmInputNumber);
  // we're only interested in setting this bit, so ORing works nicely
  if (latchingAlarmPinState == true)
  {
    activeAlarms |= (1 << cExtLatchingAlarmBit);
  }

  // this 'if' is here so we only refresh the settings when the pin changes
  if (_previousExtMomentaryAlarmPinState != momentaryAlarmPinState)
  {
    _previousExtMomentaryAlarmPinState = momentaryAlarmPinState;

    if (momentaryAlarmPinState == true)
    {
      activeAlarms |= (1 << cExtMomentaryAlarmBit);
    }
    else
    {
      activeAlarms &= (~(1 << cExtMomentaryAlarmBit));
    }

    if (activeAlarms == 0)
    {
      // restore the display, etc. in case the alarm(s) changed anything
      Application::refreshSettings();
    }
  }
  // lastly, mask off time slot bits; timer/counter alarm bit remains unchanged
  return activeAlarms & (0xffff << cExtAlarmShift);
}


// executes alarms based on _activeAlarms and _hourlyAlarmActive
//
void _executeAlarms()
{
  DateTime current = Application::dateTime();
  Settings *pSettings = Application::getSettingsPtr();
  uint8_t tone = 0, hour = current.hour(false, pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::Display12Hour));
  bool weBeeping = false, weBlinking = false;
  // determine if we need to be beeping and/or blinking
  if (_activeAlarms != 0)
  {
    for (uint8_t i = 0; i < cAlarmCount; i++)
    {
      // if this alarm is active...
      if (((_activeAlarms >> i) & 1) != 0)
      {
        // check if the alarm is enabled for beeping
        if (pSettings->getSetting(Settings::Setting::BeepStates, i) == true)
        {
          weBeeping = true;
        }
        // check if the alarm is enabled for blinking
        if (pSettings->getSetting(Settings::Setting::BlinkStates, i) == true)
        {
          weBlinking = true;
        }
      }
    }
    // finally, make beeping and blinking happen as necessary
    if (weBeeping == true)
    {
      if (_beepCounter >= sizeof(cAlarmToneDurations) / 2) // words / 2 = bytes
      {
        _beepCounter = 0;
      }

      if (Hardware::tone(cAlarmToneFrequencies[_beepCounter], cAlarmToneDurations[_beepCounter]) != Hardware::HwReqAck::HwReqAckError)
      {
        _beepCounter++;
      }
      // cancel this since some other, more important alarm is active
      _hourlyAlarmActive = false;
    }

    if (weBlinking == true)
    {
      // make the display bright!
      Application::setIntensity(NixieGlyph::cGlyph100Percent);
      // blink based on seconds, or...
      // DisplayManager::setDisplayBlanking(current.second(false) & 1);
      // blink based on sub-seconds register for faster blinking
      DisplayManager::setDisplayBlanking(RTC_SSR & (1 << 6));
    }
  }
  else if (_hourlyAlarmActive == true)
  {
    if (hour == _lastHourlyBeepHour)
    {
      // ignore the request if we already beeped this hour
      _hourlyAlarmActive = false;
    }
    else
    {
      // emit at least one (low) beep at hour zero
      if (hour == 0)
      {
        Hardware::tone(cHourlyAlarmToneFrequencies[tone], cHourlyAlarmToneDuration);
        _beepCounter = 0;
        _lastHourlyBeepHour = hour;
        _hourlyAlarmActive = false;
      }
      else
      {
        // we use bit 0 to determine if we should beep or pause/rest...so...
        // below we roll bit 0 off to get the number of bits we need to rotate
        tone = (hour >> (_beepCounter >> 1));

        // if tone is greater than zero, let's make some (more) beeps!
        if (tone > 0)
        {
          // here we check if we should beep or pause by checking bit 0
          if ((_beepCounter & 1) == 0)
          {
            // this makes a beep based on the LSb of the hour
            if (Hardware::tone(cHourlyAlarmToneFrequencies[tone & 1], cHourlyAlarmToneDuration) != Hardware::HwReqAck::HwReqAckError)
            {
              // we end up here if Hardware accepted our beep, so we can move onto the next beep/bit
              _beepCounter++;
            }
          }
          else
          {
            // this produces a pause/rest
            if (Hardware::tone(1, cHourlyAlarmToneDuration) != Hardware::HwReqAck::HwReqAckError)
            {
              // we end up here if Hardware accepted our beep, so we can move onto the next beep/bit
              _beepCounter++;
            }
          }
        }
        else
        {
          // if tone was zero, there are no more (relevant) bits to beep about
          _beepCounter = 0;
          _lastHourlyBeepHour = hour;
          _hourlyAlarmActive = false;
        }
      }
    }
  }
  else
  {
    _beepCounter = 0;
  }
}


void initialize()
{
  for (uint8_t i = 0; i < cTimeSlotAlarmCount; i++)
  {
    _ackTime[i] = Application::dateTime();
  }
}


void keyHandler(Keys::Key key)
{
  if (key == Keys::Key::A)
  {
    clearAlarm();
  }
}


void loop()
{
  DateTime current = Application::dateTime();
  Settings *pSettings = Application::getSettingsPtr();

  // first, check/get the external alarm inputs
  _activeAlarms = _getActiveExternalAlarms();

  // next, if the clock is set, check if any alarm boundaries have been crossed, triggering an alarm
  if (Hardware::rtcIsSet())
  {
    _activeAlarms |= _getActiveTimeSlotAlarms();

    // if the minutes and seconds are both zero, it's the top of the hour, so trigger the hourly alarm
    if ((pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::HourlyChime) == true)
        && (current.second(false) == 0) && (current.minute(false) == 0))
    {
      _hourlyAlarmActive = true;
    }
  }

  _executeAlarms();
}


bool isAlarmActive()
{
  return _activeAlarms;
}


void clearAlarm()
{
  if (_activeAlarms != 0)
  {
    DateTime current = Application::dateTime();

    for (uint8_t i = 0; (i < cAlarmCount) && (_activeAlarms != 0); i++)
    {
      if (_activeAlarms & (1 << i))
      {
        if (i < cTimeSlotAlarmCount)
        {
          _ackTime[i] = current;
        }
        _activeAlarms &= (~(1 << i));
      }
    }
    _beepCounter = 0;
    // restore the display, etc. in case the alarm(s) changed anything
    Application::refreshSettings();
  }
  _hourlyAlarmActive = false;
}


void activateTimerCounterAlarm()
{
  _activeAlarms |= (1 << cTimerCounterAlarmBit);
}


}

}
