//
// kbx81's tube clock timer/counter view class
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
#pragma once


#include <cstdint>

#include "Application.h"
#include "Settings.h"
#include "View.h"


namespace kbxTubeClock {

class TimerCounterView : public View {

enum TimerMode : uint8_t
{
  TimerStop = ViewMode::ViewMode0,
  TimerRunUp = ViewMode::ViewMode1,
  TimerRunDown = ViewMode::ViewMode2,
  TimerReset = ViewMode::ViewMode3
};

// The view which displays the timer/counter
//
public: // Implement the TimerCounterView class
  static const uint32_t cMaxBcdValue;

  TimerCounterView();
  virtual void enter() override;
  virtual bool keyHandler(Keys::Key key) override;
  virtual void loop() override;

private:
  // the last time (in seconds) that we saw
  //
  uint32_t _lastTime;

  // the timer/counter's value
  //
  uint32_t _timerValue;

  // the fade duration that will be used for the display
  //
  uint16_t _fadeDuration;

  // direction for the timer to count
  //
  bool _countUp;

  // true if the alarm is ready to be activated
  //
  bool _alarmReady;

};

}
