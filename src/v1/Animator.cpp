//
// kbx81's tube clock display animator
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
#include "Animator.h"
#include "Application.h"
#include "Display.h"
#include "DisplayManager.h"
#include "Hardware.h"


namespace kbxTubeClock {

namespace Animator {


// Number
//
const uint8_t cMaxAnimationId = 4;

// The animation ID triggered by the last call to run()
//
uint16_t _animationId = 0;

// The previous animation ID triggered by the last call to run()
//
uint16_t _lastAnimationId = 0;

// The last frame written to the display. Ensures all frames are displayed.
//
uint16_t _lastWrittenFrame = 0;

// Duration for the animation triggered by run()
//
uint16_t _duration = 0;

// Counter for animation in progress
//
uint16_t _runCounter = 0;

// Number of ticks between frames
//
uint16_t _ticksBetweenFrames = 0;

// Initial and final displays for animations
//
Display _final, _initial;


// Copies dots from the source display to destination display
//
void _copyDots(Display &source, Display &destination)
{
  for (uint8_t dot = 0; dot < Display::cDotCount; dot++)
  {
    destination.setDot(dot, source.getDotRaw(dot));
  }

  return;
}


// Returns the current frame number based on totalNumberOfFrames and _duration
//
uint16_t _currentFrame(const uint16_t totalNumberOfFrames)
{
  return (totalNumberOfFrames * _lastWrittenFrame) / _duration;
}


// Rolls over values > 9
//
uint8_t _rollOverer(uint8_t value)
{
  while (value > 9)
  {
    value = value - 10;
  }
  return value;
}


// All digits roll
//
Display _frameGenerator0()
{
  Display frame;
  uint32_t valueOffset = 9 - _currentFrame(9);

  for (uint8_t t = 0; t < Display::cTubeCount; t++)
  {
    frame.setTubeToValue(t, _rollOverer(_final.getTubeValue(t) + valueOffset));

    frame.setTubeIntensity(t, _final.getTubeIntensity(t));
  }

  return frame;
}


// Slide in from left
//
Display _frameGenerator1()
{
  Display frame;
  uint32_t frameNumber = _currentFrame(Display::cTubeCount);  // six tubes
  uint8_t  t = 0;

  // set from _initial -- does not run when frameNumber == 6
  for (t = 0; t < Display::cTubeCount - frameNumber; t++)
  {
    frame.setTubeToValue(t + frameNumber, _initial.getTubeValue(t));

    frame.setTubeIntensity(t + frameNumber, _initial.getTubeIntensity(t));
  }
  // set from _final -- does not run when frameNumber == 0
  for (t = 0; t < frameNumber; t++)
  {
    frame.setTubeToValue(frameNumber - 1 - t, _final.getTubeValue(Display::cTubeCount - 1 - t));

    frame.setTubeIntensity(t, _final.getTubeIntensity(t));
  }

  return frame;
}


// One digit at a time roll from left
//
Display _frameGenerator2()
{
  Display frame;
  uint32_t frameNumber = _currentFrame(10 * 6); // 10 digits by 6 tubes
  uint8_t  workingTube = frameNumber / 10,  // gives us 0 to (Display::cGlyphCount - 1)
           workingValue = frameNumber % 10, // gives us 0 - 9
           t = 0;

  // set from _initial -- does not run when frameNumber == 6
  for (t = workingTube; t < Display::cTubeCount; t++)
  {
    frame.setTubeToValue(t, _initial.getTubeValue(t));

    frame.setTubeIntensity(t, _initial.getTubeIntensity(t));
  }
  // set from _final -- does not run when frameNumber == 0
  for (t = 0; t < workingTube; t++)
  {
    frame.setTubeToValue(t, _final.getTubeValue(t));

    frame.setTubeIntensity(t, _final.getTubeIntensity(t));
  }

  frame.setTubeToValue(workingTube, _rollOverer(_final.getTubeValue(workingTube) + workingValue));

  return frame;
}


// Odds then Evens digits roll
//
Display _frameGenerator3()
{
  Display frame;
  uint32_t frameNumber = _currentFrame(10 * 2); // 10 digits by 2 groups
  uint8_t  workingTube = frameNumber / 10,  // gives us 0 or 1 (evens or odds)
           workingValue = frameNumber % 10, // gives us 0 - 9
           t = 0;

  for (t = 0; t < Display::cTubeCount; t++)
  {
    if ((t & 1) == 0)   // if 't' is (an) even (numbered tube)...
    {
      if (workingTube == 0)
      {
        frame.setTubeToValue(t, _rollOverer(_final.getTubeValue(t) + workingValue));
      }
      else
      {
        frame.setTubeToValue(t, _final.getTubeValue(t));
      }
      frame.setTubeIntensity(t, _final.getTubeIntensity(t));
    }
    else    // but if 't' is an odd numbered tube...
    {
      if (workingTube == 0)
      {
        frame.setTubeToValue(t, _initial.getTubeValue(t));
        frame.setTubeIntensity(t, _initial.getTubeIntensity(t));
      }
      else
      {
        frame.setTubeToValue(t, _rollOverer(_final.getTubeValue(t) + workingValue));
        frame.setTubeIntensity(t, _final.getTubeIntensity(t));
      }
    }
  }

  return frame;
}


// Pairs of digits roll inward  --><--
//
Display _frameGenerator4()
{
  Display frame;
  uint32_t frameNumber = _currentFrame(10 * 3); // 10 digits by 3 groups
  uint8_t  workingValue = frameNumber % 10, // gives us 0 - 9
           t = 0, f = 0;

  for (t = 0; t < Display::cTubeCount / 2; t++)
  {
    f = Display::cTubeCount - 1 - t;

    if (frameNumber >= 20)
    {
      if (t == 2)
      {
        frame.setTubeToValue(t, _rollOverer(_final.getTubeValue(t) + workingValue));
        frame.setTubeToValue(f, _rollOverer(_final.getTubeValue(f) + workingValue));
      }
      else
      {
        frame.setTubeToValue(t, _final.getTubeValue(t));
        frame.setTubeToValue(f, _final.getTubeValue(f));
        frame.setTubeIntensity(t, _final.getTubeIntensity(t));
        frame.setTubeIntensity(f, _final.getTubeIntensity(f));
      }
    }
    else if (frameNumber >= 10)
    {
      if (t == 1)
      {
        frame.setTubeToValue(t, _rollOverer(_final.getTubeValue(t) + workingValue));
        frame.setTubeToValue(f, _rollOverer(_final.getTubeValue(f) + workingValue));
      }
      else if (t < 1)
      {
        frame.setTubeToValue(t, _final.getTubeValue(t));
        frame.setTubeToValue(f, _final.getTubeValue(f));
        frame.setTubeIntensity(t, _final.getTubeIntensity(t));
        frame.setTubeIntensity(f, _final.getTubeIntensity(f));
      }
      else if (t > 1)
      {
        frame.setTubeToValue(t, _initial.getTubeValue(t));
        frame.setTubeToValue(f, _initial.getTubeValue(f));
      }
    }
    else
    {
      if (t == 0)
      {
        frame.setTubeToValue(t, _rollOverer(_final.getTubeValue(t) + workingValue));
        frame.setTubeToValue(f, _rollOverer(_final.getTubeValue(f) + workingValue));
        frame.setTubeIntensity(t, _final.getTubeIntensity(t));
        frame.setTubeIntensity(f, _final.getTubeIntensity(f));
      }
      else
      {
        frame.setTubeToValue(t, _initial.getTubeValue(t));
        frame.setTubeToValue(f, _initial.getTubeValue(f));
      }
    }
  }

  return frame;
}


void initialize()
{
}


void keyHandler(Keys::Key key)
{
  if (key != Keys::Key::None)
  {
    _runCounter = _duration;
    _lastWrittenFrame = _duration;
  }
}


void setDelayBetweenFrames(const uint16_t ticksBetweenFrames)
{
  _ticksBetweenFrames = ticksBetweenFrames;
}


void setAnimationDuration(const uint16_t duration)
{
  _duration = duration;
}


void setFinalDisplay(const Display display)
{
  _final = display;
}


void setInitialDisplay(const Display display)
{
  _initial = display;
}


bool isRunning()
{
  return (_lastWrittenFrame < _duration);
}


void run(const uint8_t animationId)
{
  if (animationId == 0)
  {
    _lastAnimationId = _animationId;
    if (++_animationId > cMaxAnimationId)
    {
      _animationId = 0;
    }
  }
  else
  {
    _lastAnimationId = _animationId;
    _animationId = animationId - 1;
  }

  _runCounter = 0;
  _lastWrittenFrame = 0;
}


void loop()
{
  Display  tcDisp;
  RgbLed   statusLed;
  _lastWrittenFrame = _runCounter;

  switch (_animationId)
  {
    case 1:
      tcDisp = _frameGenerator1();
      break;

    case 2:
      tcDisp = _frameGenerator2();
      break;

    case 3:
      tcDisp = _frameGenerator3();
      break;

    case 4:
      tcDisp = _frameGenerator4();
      break;

    default:
      tcDisp = _frameGenerator0();
      break;
  }

  _copyDots(_final, tcDisp);

  DisplayManager::writeDisplay(tcDisp, statusLed);
}


void tick()
{
  if (_runCounter < _duration)
  {
    _runCounter++;
  }
}


}

}
