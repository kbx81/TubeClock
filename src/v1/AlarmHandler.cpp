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
#include <cstring>

#include <libopencm3/stm32/rtc.h>
#include "AlarmHandler.h"
#include "Application.h"
#include "DateTime.h"
#include "DisplayManager.h"
#include "Hardware.h"
#include "Keys.h"
#include "RtttlPlayer.h"
#include "Settings.h"

namespace kbxTubeClock::AlarmHandler {

// Latching external alarm input number
//
constexpr uint8_t cLatchingAlarmInputNumber = 0;

// Momentary external alarm input number
//
constexpr uint8_t cMomentaryAlarmInputNumber = 1;

// Alarm melody as an RTTTL string (loops until alarm is cleared)
// BPM=200 → whole=1200ms; default duration=eighth(150ms), octave=6
//   c7=150ms, 4g=300ms, 16p≈75ms, 4g=300ms, a=150ms, g=150ms, p=150ms, b=150ms, c7=150ms, 2p=600ms
//
static const char cAlarmTone[] = "alarm:d=8,o=6,b=180:c7,4g,16p,4g,a,g,p,b,c7,2p";

// RTTTL header for hourly chime (notes added based on hour value)
//
static const char cHourlyHeader[] = "chime:d=16,o=6,b=180:";

// Notes for hourly chime
//
static const char cHourly0[] = "e5";     // bit 0
static const char cHourly1[] = "e7";     // bit 1
static const char cHourlyP[] = ",16p,";  // pause between notes (commas included)

// Number of time slot alarms available
//
constexpr uint8_t cTimeSlotAlarmCount = 8;

// Number of external alarms available
//
constexpr uint8_t cExtAlarmCount = 3;

// Total number of alarms available
//
constexpr uint8_t cAlarmCount = cTimeSlotAlarmCount + cExtAlarmCount;

// Bit indicating the timer/counter alarm should sound
//
constexpr uint8_t cExtAlarmShift = 8;

// Bit indicating the timer/counter alarm should sound
//
constexpr uint8_t cTimerCounterAlarmBit = 8;

// Bit indicating the latching (external) alarm should sound
//
constexpr uint8_t cExtLatchingAlarmBit = 9;

// Bit indicating the momentary (external) alarm should sound
//
constexpr uint8_t cExtMomentaryAlarmBit = 10;

// Indicates which alarm(s) is/are active. Low 8 bits correspond to time slots
//
uint16_t _activeAlarms = 0;

// Hourly chime state; initial Pending state produces a start-up beep
//
enum class HourlyState : uint8_t { Idle = 0, Pending, Playing };
HourlyState _hourlyState = HourlyState::Pending;

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
uint16_t _getActiveTimeSlotAlarms(const DateTime &current, Settings *pSettings) {
  DateTime slot;
  uint16_t activeAlarms = 0;
  // first we'll look at the time slots
  for (uint8_t i = static_cast<uint8_t>(Settings::Slot::Slot1); i <= static_cast<uint8_t>(Settings::Slot::Slot8); i++) {
    // first check if the alarm is enabled
    if ((pSettings->getSetting(Settings::Setting::BeepStates, i) == true) ||
        (pSettings->getSetting(Settings::Setting::BlinkStates, i) == true)) {
      // get the current slot's time
      slot = pSettings->getTime(i);
      // set the alarm slot's date to today so we can compare them
      slot.setDate(current.year(false), current.month(false), current.day(false));
      // if the alarm time has passed and hasn't been acknowledged since...
      if ((slot <= current) && (_ackTime[i] < slot)) {
        // ...we have an active alarm
        activeAlarms |= (1 << i);
      }
    }
  }
  return activeAlarms;
}

// returns a bitmap indicating which external alarms are active
//
uint16_t _getActiveExternalAlarms() {
  uint16_t activeAlarms = _activeAlarms;
  bool latchingAlarmPinState = Hardware::alarmInput(cLatchingAlarmInputNumber),
       momentaryAlarmPinState = Hardware::alarmInput(cMomentaryAlarmInputNumber);
  // we're only interested in setting this bit, so ORing works nicely
  if (latchingAlarmPinState == true) {
    activeAlarms |= (1 << cExtLatchingAlarmBit);
  }

  // this 'if' is here so we only refresh the settings when the pin changes
  if (_previousExtMomentaryAlarmPinState != momentaryAlarmPinState) {
    _previousExtMomentaryAlarmPinState = momentaryAlarmPinState;

    if (momentaryAlarmPinState == true) {
      activeAlarms |= (1 << cExtMomentaryAlarmBit);
    } else {
      activeAlarms &= (~(1 << cExtMomentaryAlarmBit));
    }

    if (activeAlarms == 0) {
      // restore the display, etc. in case the alarm(s) changed anything
      Application::refreshSettings();
    }
  }
  // lastly, mask off time slot bits; timer/counter alarm bit remains unchanged
  return activeAlarms & (0xffff << cExtAlarmShift);
}

// Builds and starts an RTTTL string encoding 'hour' as binary beeps.
// Each bit of hour (LSb first) maps to e6 (bit=0) or e7 (bit=1),
// separated by 16th-note rests. At BPM=200, one 16th note = 75ms.
// Hour zero gets a single low (e6) beep.
//
void _startHourlyChime(uint8_t hour) {
  // Max size: cHourlyHeader(21) + 5 notes * "e7"(2) + 4 pauses * ",16p,"(5) = 21+10+20 = 51
  char buf[56];
  uint8_t pos = sizeof(cHourlyHeader) - 1;

  memcpy(buf, cHourlyHeader, pos);

  if (hour == 0) {
    buf[pos++] = 'e';
    buf[pos++] = '6';
  } else {
    while (hour > 0) {
      static_assert(sizeof(cHourly0) == sizeof(cHourly1), "Note strings must have equal length");
      const char *note = (hour & 1) ? cHourly1 : cHourly0;
      memcpy(buf + pos, note, sizeof(cHourly0) - 1);
      pos += sizeof(cHourly0) - 1;
      hour >>= 1;
      if (hour > 0) {
        memcpy(buf + pos, cHourlyP, sizeof(cHourlyP) - 1);
        pos += sizeof(cHourlyP) - 1;
      }
    }
    buf[pos] = 0;  // null terminator
  }
  RtttlPlayer::play(buf, pos);
}

// executes alarms based on _activeAlarms and _hourlyAlarmActive
//
void _executeAlarms(const DateTime &current, Settings *pSettings) {
  uint8_t hour = current.hour(
      false, pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::Display12Hour));
  bool weBeeping = false, weBlinking = false;
  // determine if we need to be beeping and/or blinking
  if (_activeAlarms != 0) {
    for (uint8_t i = 0; i < cAlarmCount; i++) {
      // if this alarm is active...
      if (((_activeAlarms >> i) & 1) != 0) {
        // check if the alarm is enabled for beeping
        if (pSettings->getSetting(Settings::Setting::BeepStates, i) == true) {
          weBeeping = true;
        }
        // check if the alarm is enabled for blinking
        if (pSettings->getSetting(Settings::Setting::BlinkStates, i) == true) {
          weBlinking = true;
        }
      }
    }
    // finally, make beeping and blinking happen as necessary
    if (weBeeping == true) {
      if (!RtttlPlayer::isPlaying()) {
        RtttlPlayer::play(cAlarmTone, sizeof(cAlarmTone) - 1);
      }
      // cancel this since some other, more important alarm is active
      _hourlyState = HourlyState::Idle;
    }

    if (weBlinking == true) {
      // make the display bright!
      Application::setIntensity(NixieGlyph::cGlyphMaximumIntensity);
      // blink based on seconds, or...
      // DisplayManager::setDisplayBlanking(current.second(false) & 1);
      // blink based on sub-seconds register for faster blinking
      DisplayManager::setDisplayBlanking(RTC_SSR & (1 << 6));
    }
  } else if (_hourlyState != HourlyState::Idle) {
    if (hour == _lastHourlyBeepHour) {
      // ignore the request if we already beeped this hour
      _hourlyState = HourlyState::Idle;
    } else if (_hourlyState == HourlyState::Playing) {
      if (!RtttlPlayer::isPlaying()) {
        // chime has finished playing
        _lastHourlyBeepHour = hour;
        _hourlyState = HourlyState::Idle;
      }
    } else if (!RtttlPlayer::isPlaying()) {
      _startHourlyChime(hour);
      _hourlyState = HourlyState::Playing;
    }
  }
}

void initialize() {
  for (uint8_t i = 0; i < cTimeSlotAlarmCount; i++) {
    _ackTime[i] = Application::dateTime();
  }
  // Suppress the startup chime if the RTC is not valid; without this guard the initial
  // Pending state fires _executeAlarms() with incorrect/zeroed time, bypassing the
  // rtcIsSet() check that normally protects the top-of-hour trigger path.
  if (!Hardware::rtcIsSet()) {
    _hourlyState = HourlyState::Idle;
  }
}

void keyHandler(Keys::Key key) {
  if (key == Keys::Key::A) {
    clearAlarm();
  }
}

void loop() {
  const DateTime current = Application::dateTime();
  Settings *pSettings = Application::getSettingsPtr();

  // first, check/get the external alarm inputs
  _activeAlarms = _getActiveExternalAlarms();

  // next, if the clock is set, check if any alarm boundaries have been crossed, triggering an alarm
  if (Hardware::rtcIsSet()) {
    _activeAlarms |= _getActiveTimeSlotAlarms(current, pSettings);

    // if the minutes and seconds are both zero, it's the top of the hour, so trigger the hourly alarm
    if ((pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::HourlyChime) == true) &&
        (current.second(false) == 0) && (current.minute(false) == 0)) {
      if (_hourlyState == HourlyState::Idle) {
        _hourlyState = HourlyState::Pending;
      }
    }
  }

  _executeAlarms(current, pSettings);
}

bool isAlarmActive() { return _activeAlarms; }

void clearAlarm() {
  _hourlyState = HourlyState::Idle;

  if (_activeAlarms == 0) {
    return;
  }

  DateTime current = Application::dateTime();

  for (uint8_t i = 0; (i < cAlarmCount) && (_activeAlarms != 0); i++) {
    if (_activeAlarms & (1 << i)) {
      if (i < cTimeSlotAlarmCount) {
        _ackTime[i] = current;
      }
      _activeAlarms &= (~(1 << i));
    }
  }
  RtttlPlayer::stop();
  // restore the display, etc. in case the alarm(s) changed anything
  Application::refreshSettings();
}

void activateTimerCounterAlarm() { _activeAlarms |= (1 << cTimerCounterAlarmBit); }

void playChime(uint8_t hour) {
  Settings *pSettings = Application::getSettingsPtr();
  bool is12h = pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::Display12Hour);
  if (hour > 23) {
    hour = Application::dateTime().hour(false, is12h);
  } else if (is12h) {
    if (hour == 0) {
      hour = 12;
    } else if (hour > 12) {
      hour -= 12;
    }
  }
  _lastHourlyBeepHour = 255;  // prevent dedup suppression on next loop iteration
  _startHourlyChime(hour);
  _hourlyState = HourlyState::Playing;
}

}  // namespace kbxTubeClock::AlarmHandler
