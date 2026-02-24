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
const uint8_t cMaxAnimationId = 10;

// Per-tube stagger offsets (0-5) for the staggered-start animation
//
static const uint8_t cBaseStagger[Display::cTubeCount] = {3, 0, 5, 1, 4, 2};

// The animation ID triggered by the last call to run()
//
uint16_t _animationId = 0;

// The last frame written to the display. Ensures all frames are displayed.
//
uint16_t _lastWrittenFrame = 0;

// Duration for the animation triggered by run()
//
uint16_t _duration = 0;

// Counter for animation in progress
//
uint16_t _runCounter = 0;

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


// Rolls over values > 9 (input is at most 18 given tube values 0-9 plus offset 0-9)
//
uint8_t _rollOverer(uint8_t value)
{
  if (value >= 10) value -= 10;
  return value;
}


// All digits roll
//
Display _frameGenerator0()
{
  Display frame;
  uint8_t valueOffset = 9 - _currentFrame(9);

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
  uint8_t frameNumber = _currentFrame(Display::cTubeCount);  // six tubes
  uint8_t t = 0;

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
  uint8_t frameNumber = _currentFrame(10 * 6); // 10 digits by 6 tubes
  uint8_t workingTube = (frameNumber * 0xCCCDu) >> 19,   // gives us 0 to (Display::cGlyphCount - 1)
           workingValue = frameNumber - workingTube * 10, // gives us 0 - 9
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
  frame.setTubeIntensity(workingTube, _final.getTubeIntensity(workingTube));

  return frame;
}


// Odds then Evens digits roll
//
Display _frameGenerator3()
{
  Display frame;
  uint8_t frameNumber = _currentFrame(10 * 2); // 10 digits by 2 groups
  uint8_t workingTube = (frameNumber * 0xCCCDu) >> 19,   // gives us 0 or 1 (evens or odds)
           workingValue = frameNumber - workingTube * 10, // gives us 0 - 9
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


// Shared helper for pair animations. outerFirst=true: pairs roll inward (outer first);
//  outerFirst=false: pairs roll outward (inner first).
//  Each pair's turn order is determined by comparing its index to the current phase (0-2).
//
static Display _frameGeneratorPairs(const bool outerFirst)
{
  Display frame;
  uint8_t frameNumber  = _currentFrame(10 * 3); // 10 digits by 3 groups
  uint8_t phase        = (frameNumber * 0xCCCDu) >> 19, // group 0, 1, or 2
          workingValue = frameNumber - phase * 10,        // 0 - 9
          t = 0, f = 0;

  for (t = 0; t < Display::cTubeCount / 2; t++)
  {
    f = Display::cTubeCount - 1 - t;

    // outerFirst: order is 0,1,2 for t=0,1,2  (outer pair rolls first)
    // innerFirst: order is 2,1,0 for t=0,1,2  (inner pair rolls first)
    const uint8_t order = outerFirst ? t : (Display::cTubeCount / 2 - 1 - t);

    if (order < phase)       // this pair already rolled
    {
      frame.setTubeToValue(t, _final.getTubeValue(t));
      frame.setTubeToValue(f, _final.getTubeValue(f));
      frame.setTubeIntensity(t, _final.getTubeIntensity(t));
      frame.setTubeIntensity(f, _final.getTubeIntensity(f));
    }
    else if (order == phase) // this pair is rolling now
    {
      frame.setTubeToValue(t, _rollOverer(_final.getTubeValue(t) + workingValue));
      frame.setTubeToValue(f, _rollOverer(_final.getTubeValue(f) + workingValue));
      frame.setTubeIntensity(t, _final.getTubeIntensity(t));
      frame.setTubeIntensity(f, _final.getTubeIntensity(f));
    }
    else                     // this pair hasn't started yet
    {
      frame.setTubeToValue(t, _initial.getTubeValue(t));
      frame.setTubeToValue(f, _initial.getTubeValue(f));
      frame.setTubeIntensity(t, _initial.getTubeIntensity(t));
      frame.setTubeIntensity(f, _initial.getTubeIntensity(f));
    }
  }

  return frame;
}


// Pairs of digits roll inward  --><--
//
Display _frameGenerator4() { return _frameGeneratorPairs(true);  }


// Pairs of digits roll outward  <-->
//
Display _frameGenerator5() { return _frameGeneratorPairs(false); }


// Intensity crossfade: fade out initial values, swap, fade in final values
//
Display _frameGenerator6()
{
  Display frame;
  const uint16_t progress = _currentFrame(256); // 0 to 256

  for (uint8_t t = 0; t < Display::cTubeCount; t++)
  {
    if (progress < 128)
    {
      frame.setTubeToValue(t, _initial.getTubeValue(t));
      frame.setTubeIntensity(t, ((uint16_t)_initial.getTubeIntensity(t) * (128 - progress)) >> 7);
    }
    else
    {
      frame.setTubeToValue(t, _final.getTubeValue(t));
      frame.setTubeIntensity(t, ((uint16_t)_final.getTubeIntensity(t) * (progress - 128)) >> 7);
    }
  }

  return frame;
}


// Variable-speed roll: each tube rolls at speed proportional to distance to its final digit;
//  all tubes finish simultaneously
//
Display _frameGenerator7()
{
  Display frame;
  const uint8_t f = _currentFrame(9); // 0 to 9

  for (uint8_t t = 0; t < Display::cTubeCount; t++)
  {
    const uint8_t initialVal = _initial.getTubeValue(t);
    const uint8_t finalVal   = _final.getTubeValue(t);
    const uint8_t dist       = (initialVal - finalVal + 10) % 10;
    const uint8_t offset     = dist - dist * f / 9;

    frame.setTubeToValue(t, _rollOverer(finalVal + offset));
    frame.setTubeIntensity(t, _final.getTubeIntensity(t));
  }

  return frame;
}


// Staggered start: each tube begins its roll at a pseudo-random time
//
Display _frameGenerator8()
{
  Display frame;
  const uint8_t currentPhase = _currentFrame(15); // 0 to 15 (6 stagger slots + 9 roll frames)

  for (uint8_t t = 0; t < Display::cTubeCount; t++)
  {
    const uint8_t stagger = (cBaseStagger[t] + _initial.getTubeValue(t)) % 6;

    if (currentPhase >= stagger + 10)
    {
      frame.setTubeToValue(t, _final.getTubeValue(t));
      frame.setTubeIntensity(t, _final.getTubeIntensity(t));
    }
    else if (currentPhase >= stagger)
    {
      const uint8_t offset = stagger + 9 - currentPhase; // 9 down to 0
      frame.setTubeToValue(t, _rollOverer(_final.getTubeValue(t) + offset));
      frame.setTubeIntensity(t, _final.getTubeIntensity(t));
    }
    else
    {
      frame.setTubeToValue(t, _initial.getTubeValue(t));
      frame.setTubeIntensity(t, _initial.getTubeIntensity(t));
    }
  }

  return frame;
}


// Brightness sweep: final values appear as an intensity wave moves left to right
//
Display _frameGenerator9()
{
  Display frame;
  const uint16_t progress = _currentFrame(256); // 0 to 256

  for (uint8_t t = 0; t < Display::cTubeCount; t++)
  {
    frame.setTubeToValue(t, _final.getTubeValue(t));

    const uint16_t tubeStart = (uint16_t)t * 32u; // staggered start: 0, 32, 64, 96, 128, 160

    if (progress >= tubeStart + 64u)
    {
      frame.setTubeIntensity(t, _final.getTubeIntensity(t));
    }
    else if (progress >= tubeStart)
    {
      // Linear ramp over 64 steps
      frame.setTubeIntensity(t, ((uint16_t)_final.getTubeIntensity(t) * (progress - tubeStart)) >> 6);
    }
    // else: intensity stays at 0 (default)
  }

  return frame;
}


// Slot machine deceleration: fast roll slowing to a stop on final digits (quadratic ease-out)
//
Display _frameGenerator10()
{
  Display frame;
  // tn = normalized time 0..256; i = 19*tn*(512-tn)>>16 maps 0..256 -> 0..19 with ease-out
  // tube shows (final + 19 - i) % 10: rolls 1.9 turns, decelerating to final digit
  const uint16_t tn = _currentFrame(256);
  const uint32_t i  = (19ul * tn * (512u - tn)) >> 16;

  for (uint8_t t = 0; t < Display::cTubeCount; t++)
  {
    frame.setTubeToValue(t, (uint8_t)((_final.getTubeValue(t) + 19u - i) % 10u));
    frame.setTubeIntensity(t, _final.getTubeIntensity(t));
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


void setAnimationDuration(const uint16_t duration)
{
  _duration = duration;
}


void setFinalDisplay(const Display &display)
{
  _final = display;
}


void setInitialDisplay(const Display &display)
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
    if (++_animationId > cMaxAnimationId)
    {
      _animationId = 0;
    }
  }
  else
  {
    _animationId = animationId - 1;
  }

  _runCounter = 0;
  _lastWrittenFrame = 0;
}


void loop()
{
  Display  tcDisp;
  _lastWrittenFrame = _runCounter;

  switch (_animationId)
  {
    case 1:  tcDisp = _frameGenerator1();  break;
    case 2:  tcDisp = _frameGenerator2();  break;
    case 3:  tcDisp = _frameGenerator3();  break;
    case 4:  tcDisp = _frameGenerator4();  break;
    case 5:  tcDisp = _frameGenerator5();  break;
    case 6:  tcDisp = _frameGenerator6();  break;
    case 7:  tcDisp = _frameGenerator7();  break;
    case 8:  tcDisp = _frameGenerator8();  break;
    case 9:  tcDisp = _frameGenerator9();  break;
    case 10: tcDisp = _frameGenerator10(); break;
    default: tcDisp = _frameGenerator0();  break;
  }

  _copyDots(_final, tcDisp);

  DisplayManager::writeDisplay(tcDisp);
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
