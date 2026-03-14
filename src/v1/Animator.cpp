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

namespace kbxTubeClock::Animator {

// Number
//
const uint8_t cMaxAnimationId = 17;

// Per-tube stagger offsets (0-5) for the staggered-start animation
//
static const uint8_t cBaseStagger[Display::cTubeCount] = {3, 0, 5, 1, 4, 2};

// The animation ID triggered by the last call to run()
//
uint16_t _animationId = 0;

// Bitmask of animations not yet played in the current shuffle cycle (bits 0-17)
//
uint32_t _animationsRemaining = (1u << (cMaxAnimationId + 1)) - 1u;

// Bitmask of wipe digits not yet used in the current shuffle cycle (bits 0-9)
//
uint32_t _wipeRemaining = (1u << 10) - 1u;

// Transition digit (0-9) used for wipe animations
//
uint8_t _wipeValue = 0;

// Per-animation random tube order for the random-roll animation
//
uint8_t _rollOrder[Display::cTubeCount];

// LCG state for random animation selection
//
uint32_t _rngState = 1u;

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
void _copyDots(Display &source, Display &destination) {
  for (uint8_t dot = 0; dot < Display::cDotCount; dot++) {
    destination.setDot(dot, source.getDotRaw(dot));
  }

  return;
}

// Returns the current frame number based on totalNumberOfFrames and _duration
//
uint16_t _currentFrame(const uint16_t totalNumberOfFrames) {
  return (totalNumberOfFrames * _lastWrittenFrame) / _duration;
}

// Rolls over values > 9 (input is at most 19 given tube values 0-9 plus offset 0-10)
//
uint8_t _rollOverer(uint8_t value) {
  if (value >= 10) {
    value -= 10;
  }
  return value;
}

// LCG random number generator
//
static uint32_t _rand() {
  _rngState = _rngState * 1664525u + 1013904223u;
  return _rngState;
}

// Picks a random value from a shuffle pool bitmask, resetting it when exhausted.
//  pool covers bits 0..maxIdx; a set bit means that value has not yet been returned this cycle.
//
static uint8_t _pickFromPool(uint32_t &pool, const uint8_t maxIdx) {
  if (pool == 0) {
    pool = (1u << (maxIdx + 1)) - 1u;
  }

  // Count set bits
  uint8_t count = 0;
  for (uint32_t tmp = pool; tmp; tmp >>= 1) {
    count += (tmp & 1u);
  }

  // Pick random index among set bits
  uint8_t pick = static_cast<uint8_t>(_rand() % count);

  // Find and return the Nth set bit index
  for (uint8_t idx = 0; idx <= maxIdx; idx++) {
    if (pool & (1u << idx)) {
      if (pick == 0) {
        pool &= ~(1u << idx);
        return idx;
      }
      pick--;
    }
  }

  return 0;  // unreachable
}

// Picks a random animation from the remaining pool, resetting it when exhausted
//
static uint8_t _pickAnimation() {
  if (_animationsRemaining == 0) {
    _rngState ^= RTC_SSR;
  }
  return _pickFromPool(_animationsRemaining, cMaxAnimationId);
}

// Fills _rollOrder with a Fisher-Yates random permutation of tube indices 0..cTubeCount-1
//
static void _shuffleTubeOrder() {
  for (uint8_t i = 0; i < Display::cTubeCount; i++) {
    _rollOrder[i] = i;
  }
  for (uint8_t i = Display::cTubeCount - 1; i > 0; i--) {
    const uint8_t j = static_cast<uint8_t>(_rand() % (i + 1u));
    const uint8_t tmp = _rollOrder[i];
    _rollOrder[i] = _rollOrder[j];
    _rollOrder[j] = tmp;
  }
}

// All digits roll
//
Display _frameGenerator0() {
  Display frame;
  uint8_t valueOffset = 9 - _currentFrame(9);

  for (uint8_t t = 0; t < Display::cTubeCount; t++) {
    frame.setTubeToValue(t, _rollOverer(_final.getTubeValue(t) + valueOffset));

    frame.setTubeIntensity(t, _final.getTubeIntensity(t));
  }

  return frame;
}

// Shared helper for slide animations. fromLeft=true: slide in from left; fromLeft=false: slide in from right.
//  Initial display scrolls off the opposite side; final display enters from the named side.
//
static Display _frameGeneratorSlide(const bool fromLeft) {
  Display frame;
  const uint8_t frameNumber = _currentFrame(Display::cTubeCount);
  uint8_t t = 0;

  // set from _initial -- does not run when frameNumber == Display::cTubeCount
  for (t = 0; t < Display::cTubeCount - frameNumber; t++) {
    const uint8_t pos = fromLeft ? (t + frameNumber) : t;
    const uint8_t src = fromLeft ? t : (t + frameNumber);
    frame.setTubeToValue(pos, _initial.getTubeValue(src));
    frame.setTubeIntensity(pos, _initial.getTubeIntensity(src));
  }
  // set from _final -- does not run when frameNumber == 0
  for (t = 0; t < frameNumber; t++) {
    const uint8_t pos = fromLeft ? t : (Display::cTubeCount - frameNumber + t);
    const uint8_t src = fromLeft ? (Display::cTubeCount - frameNumber + t) : t;
    frame.setTubeToValue(pos, _final.getTubeValue(src));
    frame.setTubeIntensity(pos, _final.getTubeIntensity(src));
  }

  return frame;
}

// Slide in from left
//
Display _frameGenerator1() { return _frameGeneratorSlide(true); }

// Slide in from right
//
Display _frameGenerator2() { return _frameGeneratorSlide(false); }

// Shared helper for single-digit roll animations. fromLeft=true: roll from left; fromLeft=false: roll from right.
//
static Display _frameGeneratorRoll(const bool fromLeft) {
  Display frame;
  uint8_t frameNumber = _currentFrame(10 * 6);    // 10 digits by 6 tubes
  uint8_t phase = (frameNumber * 0xCCCDu) >> 19,  // gives us 0 to 5
      workingTube = fromLeft ? phase : (Display::cTubeCount - 1 - phase),
          workingValue = frameNumber - phase * 10,  // gives us 0 - 9
      t = 0;

  // set from _initial (not-yet-rolled tubes)
  for (t = fromLeft ? (workingTube + 1) : 0; t < (fromLeft ? Display::cTubeCount : workingTube); t++) {
    frame.setTubeToValue(t, _initial.getTubeValue(t));
    frame.setTubeIntensity(t, _initial.getTubeIntensity(t));
  }
  // set from _final (already-rolled tubes)
  for (t = fromLeft ? 0 : (workingTube + 1); t < (fromLeft ? workingTube : Display::cTubeCount); t++) {
    frame.setTubeToValue(t, _final.getTubeValue(t));
    frame.setTubeIntensity(t, _final.getTubeIntensity(t));
  }

  frame.setTubeToValue(workingTube, _rollOverer(_final.getTubeValue(workingTube) + workingValue));
  frame.setTubeIntensity(workingTube, _final.getTubeIntensity(workingTube));

  return frame;
}

// One digit at a time roll in random tube order
//
static Display _frameGeneratorRollRandom() {
  Display frame;
  uint8_t frameNumber = _currentFrame(10 * 6);    // 10 digits by 6 tubes
  uint8_t phase = (frameNumber * 0xCCCDu) >> 19,  // gives us 0 to 5
      workingTube = _rollOrder[phase],
          workingValue = frameNumber - phase * 10,  // gives us 0 - 9
      t = 0;

  // set from _initial (not-yet-rolled tubes)
  for (t = phase + 1; t < Display::cTubeCount; t++) {
    const uint8_t tube = _rollOrder[t];
    frame.setTubeToValue(tube, _initial.getTubeValue(tube));
    frame.setTubeIntensity(tube, _initial.getTubeIntensity(tube));
  }
  // set from _final (already-rolled tubes)
  for (t = 0; t < phase; t++) {
    const uint8_t tube = _rollOrder[t];
    frame.setTubeToValue(tube, _final.getTubeValue(tube));
    frame.setTubeIntensity(tube, _final.getTubeIntensity(tube));
  }

  frame.setTubeToValue(workingTube, _rollOverer(_final.getTubeValue(workingTube) + workingValue));
  frame.setTubeIntensity(workingTube, _final.getTubeIntensity(workingTube));

  return frame;
}

// One digit at a time roll from left
//
Display _frameGenerator3() { return _frameGeneratorRoll(true); }

// One digit at a time roll from right
//
Display _frameGenerator4() { return _frameGeneratorRoll(false); }

// One digit at a time roll in random tube order
//
Display _frameGenerator5() { return _frameGeneratorRollRandom(); }

// Odds then Evens digits roll
//
Display _frameGenerator6() {
  Display frame;
  uint8_t frameNumber = _currentFrame(10 * 2);          // 10 digits by 2 groups
  uint8_t workingTube = (frameNumber * 0xCCCDu) >> 19,  // gives us 0 or 1 (evens or odds)
      workingValue = frameNumber - workingTube * 10,    // gives us 0 - 9
      t = 0;

  for (t = 0; t < Display::cTubeCount; t++) {
    if ((t & 1) == 0)  // if 't' is (an) even (numbered tube)...
    {
      if (workingTube == 0) {
        frame.setTubeToValue(t, _rollOverer(_final.getTubeValue(t) + workingValue));
      } else {
        frame.setTubeToValue(t, _final.getTubeValue(t));
      }
      frame.setTubeIntensity(t, _final.getTubeIntensity(t));
    } else  // but if 't' is an odd numbered tube...
    {
      if (workingTube == 0) {
        frame.setTubeToValue(t, _initial.getTubeValue(t));
        frame.setTubeIntensity(t, _initial.getTubeIntensity(t));
      } else {
        frame.setTubeToValue(t, _rollOverer(_final.getTubeValue(t) + workingValue));
        frame.setTubeIntensity(t, _final.getTubeIntensity(t));
      }
    }
  }

  return frame;
}

// Pair tables for all LUT-based pair roll animations
//  Adjacent: (0,1), (2,3), (4,5)
//  Mirror: (0,5), (1,4), (2,3)  — inward/outward
//  Spread:   (0,3), (1,4), (2,5)
//
static const uint8_t cPairsAdjA[3] = {0, 2, 4};
static const uint8_t cPairsAdjB[3] = {1, 3, 5};
static const uint8_t cPairsSequentialA[3] = {0, 1, 2};  // shared by mirror and spread
static const uint8_t cPairsMirrorB[3] = {5, 4, 3};
static const uint8_t cPairsSpreadB[3] = {3, 4, 5};

// Shared helper for LUT-based pair roll animations.
//  pairA[p]/pairB[p] are the two tube indices in pair p (0-2).
//  fromLeft=true: pair 0 rolls first; fromLeft=false: pair 2 rolls first.
//
static Display _frameGeneratorPairsLUT(const uint8_t pairA[3], const uint8_t pairB[3], const bool fromLeft) {
  Display frame;
  uint8_t frameNumber = _currentFrame(10 * 3),  // 10 digits by 3 groups
      phase = (frameNumber * 0xCCCDu) >> 19,    // group 0, 1, or 2
      workingValue = frameNumber - phase * 10;  // 0 - 9

  for (uint8_t p = 0; p < 3; p++) {
    const uint8_t t = pairA[p];
    const uint8_t f = pairB[p];
    const uint8_t order = fromLeft ? p : (2u - p);

    if (order < phase)  // this pair already rolled
    {
      frame.setTubeToValue(t, _final.getTubeValue(t));
      frame.setTubeToValue(f, _final.getTubeValue(f));
      frame.setTubeIntensity(t, _final.getTubeIntensity(t));
      frame.setTubeIntensity(f, _final.getTubeIntensity(f));
    } else if (order == phase)  // this pair is rolling now
    {
      frame.setTubeToValue(t, _rollOverer(_final.getTubeValue(t) + workingValue));
      frame.setTubeToValue(f, _rollOverer(_final.getTubeValue(f) + workingValue));
      frame.setTubeIntensity(t, _final.getTubeIntensity(t));
      frame.setTubeIntensity(f, _final.getTubeIntensity(f));
    } else  // this pair hasn't started yet
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
Display _frameGenerator7() { return _frameGeneratorPairsLUT(cPairsSequentialA, cPairsMirrorB, true); }

// Pairs of digits roll outward  <-->
//
Display _frameGenerator8() { return _frameGeneratorPairsLUT(cPairsSequentialA, cPairsMirrorB, false); }

// Adjacent pairs (1/2, 3/4, 5/6) roll left-to-right
//
Display _frameGenerator9() { return _frameGeneratorPairsLUT(cPairsAdjA, cPairsAdjB, true); }

// Adjacent pairs (5/6, 3/4, 1/2) roll right-to-left
//
Display _frameGenerator10() { return _frameGeneratorPairsLUT(cPairsAdjA, cPairsAdjB, false); }

// Spread pairs (1/4, 2/5, 3/6) roll left-to-right
//
Display _frameGenerator11() { return _frameGeneratorPairsLUT(cPairsSequentialA, cPairsSpreadB, true); }

// Spread pairs (3/6, 2/5, 1/4) roll right-to-left
//
Display _frameGenerator12() { return _frameGeneratorPairsLUT(cPairsSequentialA, cPairsSpreadB, false); }

// Variable-speed roll: each tube rolls at speed proportional to distance to its final digit;
//  all tubes finish simultaneously
//
Display _frameGenerator13() {
  Display frame;
  const uint8_t f = _currentFrame(9);  // 0 to 9

  for (uint8_t t = 0; t < Display::cTubeCount; t++) {
    const uint8_t initialVal = _initial.getTubeValue(t);
    const uint8_t finalVal = _final.getTubeValue(t);
    const uint8_t dist = (initialVal - finalVal + 10) % 10;
    const uint8_t eDist = dist ? dist : 10u;  // ensure at least one full cycle
    const uint8_t offset = eDist - eDist * f / 9;

    frame.setTubeToValue(t, _rollOverer(finalVal + offset));
    frame.setTubeIntensity(t, _final.getTubeIntensity(t));
  }

  return frame;
}

// Staggered start: each tube begins its roll at a pseudo-random time
//
Display _frameGenerator14() {
  Display frame;
  const uint8_t currentPhase = _currentFrame(15);  // 0 to 15 (6 stagger slots + 9 roll frames)

  for (uint8_t t = 0; t < Display::cTubeCount; t++) {
    const uint8_t stagger = (cBaseStagger[t] + _initial.getTubeValue(t)) % 6;

    if (currentPhase >= stagger + 10) {
      frame.setTubeToValue(t, _final.getTubeValue(t));
      frame.setTubeIntensity(t, _final.getTubeIntensity(t));
    } else if (currentPhase >= stagger) {
      const uint8_t offset = stagger + 9 - currentPhase;  // 9 down to 0
      frame.setTubeToValue(t, _rollOverer(_final.getTubeValue(t) + offset));
      frame.setTubeIntensity(t, _final.getTubeIntensity(t));
    } else {
      frame.setTubeToValue(t, _initial.getTubeValue(t));
      frame.setTubeIntensity(t, _initial.getTubeIntensity(t));
    }
  }

  return frame;
}

// Slot machine deceleration: fast roll slowing to a stop on final digits (quadratic ease-out)
//
Display _frameGenerator15() {
  Display frame;
  // tn = normalized time 0..256; i = 19*tn*(512-tn)>>16 maps 0..256 -> 0..19 with ease-out
  // tube shows (final + 19 - i) % 10: rolls 1.9 turns, decelerating to final digit
  const uint16_t tn = _currentFrame(256);
  const uint32_t i = (19ul * tn * (512u - tn)) >> 16;

  for (uint8_t t = 0; t < Display::cTubeCount; t++) {
    frame.setTubeToValue(t, (uint8_t) ((_final.getTubeValue(t) + 19u - i) % 10u));
    frame.setTubeIntensity(t, _final.getTubeIntensity(t));
  }

  return frame;
}

// Shared helper for wipe animations. fromLeft=true: wipes left-to-right; fromLeft=false: wipes right-to-left.
//  Phase 1: initial display scrolls out in the wipe direction while X scrolls in from the opposite side.
//  Phase 2: X scrolls out in the wipe direction while the final display scrolls in from the opposite side.
//
static Display _frameGeneratorWipe(const bool fromLeft) {
  Display frame;
  const uint8_t f = _currentFrame(12);  // 0 to 12 (6 frames per phase)
  uint8_t t;

  if (f <= 6) {
    // Phase 1: X scrolls in, initial scrolls out
    const uint8_t step = f;
    if (fromLeft) {
      for (t = 0; t < step; t++) {
        frame.setTubeToValue(t, _wipeValue);
        frame.setTubeIntensity(t, _final.getTubeIntensity(t));
      }
      for (t = step; t < Display::cTubeCount; t++) {
        frame.setTubeToValue(t, _initial.getTubeValue(t - step));
        frame.setTubeIntensity(t, _initial.getTubeIntensity(t - step));
      }
    } else {
      for (t = 0; t < Display::cTubeCount - step; t++) {
        frame.setTubeToValue(t, _initial.getTubeValue(t + step));
        frame.setTubeIntensity(t, _initial.getTubeIntensity(t + step));
      }
      for (t = Display::cTubeCount - step; t < Display::cTubeCount; t++) {
        frame.setTubeToValue(t, _wipeValue);
        frame.setTubeIntensity(t, _final.getTubeIntensity(t));
      }
    }
  } else {
    // Phase 2: final scrolls in, X scrolls out
    const uint8_t step = f - 6;  // 1 to 6
    if (fromLeft) {
      for (t = 0; t < step; t++) {
        frame.setTubeToValue(t, _final.getTubeValue(Display::cTubeCount - step + t));
        frame.setTubeIntensity(t, _final.getTubeIntensity(Display::cTubeCount - step + t));
      }
      for (t = step; t < Display::cTubeCount; t++) {
        frame.setTubeToValue(t, _wipeValue);
        frame.setTubeIntensity(t, _final.getTubeIntensity(t));
      }
    } else {
      for (t = 0; t < Display::cTubeCount - step; t++) {
        frame.setTubeToValue(t, _wipeValue);
        frame.setTubeIntensity(t, _final.getTubeIntensity(t));
      }
      for (t = Display::cTubeCount - step; t < Display::cTubeCount; t++) {
        frame.setTubeToValue(t, _final.getTubeValue(t - (Display::cTubeCount - step)));
        frame.setTubeIntensity(t, _final.getTubeIntensity(t));
      }
    }
  }

  return frame;
}

// Wipe right-to-left: initial scrolls left/X scrolls in from right, then X scrolls left/final scrolls in from right
//
Display _frameGenerator16() { return _frameGeneratorWipe(false); }

// Wipe left-to-right: initial scrolls right/X scrolls in from left, then X scrolls right/final scrolls in from left
//
Display _frameGenerator17() { return _frameGeneratorWipe(true); }

void initialize() { _rngState ^= RTC_SSR; }

void keyHandler(Keys::Key key) {
  if (key != Keys::Key::None) {
    _runCounter = _duration;
    _lastWrittenFrame = _duration;
  }
}

void setAnimationDuration(const uint16_t duration) { _duration = duration; }

void setFinalDisplay(const Display &display) { _final = display; }

void setInitialDisplay(const Display &display) { _initial = display; }

bool isRunning() { return (_lastWrittenFrame < _duration); }

void run(const uint8_t animationId) {
  if (animationId == 0) {
    _animationId = _pickAnimation();
  } else {
    _animationId = animationId - 1;
  }

  if (_animationId == 11 || _animationId == 12) {
    _wipeValue = _pickFromPool(_wipeRemaining, 9);
  }

  if (_animationId == 13) {
    _shuffleTubeOrder();
  }

  _runCounter = 0;
  _lastWrittenFrame = 0;
}

void loop() {
  Display tcDisp;
  _lastWrittenFrame = _runCounter;

  switch (_animationId) {
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
    case 5:
      tcDisp = _frameGenerator5();
      break;
    case 6:
      tcDisp = _frameGenerator6();
      break;
    case 7:
      tcDisp = _frameGenerator7();
      break;
    case 8:
      tcDisp = _frameGenerator8();
      break;
    case 9:
      tcDisp = _frameGenerator9();
      break;
    case 10:
      tcDisp = _frameGenerator10();
      break;
    case 11:
      tcDisp = _frameGenerator11();
      break;
    case 12:
      tcDisp = _frameGenerator12();
      break;
    case 13:
      tcDisp = _frameGenerator13();
      break;
    case 14:
      tcDisp = _frameGenerator14();
      break;
    case 15:
      tcDisp = _frameGenerator15();
      break;
    case 16:
      tcDisp = _frameGenerator16();
      break;
    case 17:
      tcDisp = _frameGenerator17();
      break;
    default:
      tcDisp = _frameGenerator0();
      break;
  }

  _copyDots(_final, tcDisp);

  DisplayManager::writeDisplay(tcDisp);
}

void tick() {
  if (_runCounter < _duration) {
    _runCounter++;
  }
}

}  // namespace kbxTubeClock::Animator
