//
// kbx81's Nixie Glyph Crossfader Library
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
#include "NixieGlyph.h"
#include "NixieGlyphCrossfader.h"

/// @mainpage
///
/// @section intro_sec Introduction
///
/// This library contains a class to manage crossfading of glyphs on the Tube
/// Clock's Nixie display.
///
/// @section requirements_sec Requirements
///
/// This library is written in a manner so as to be compatible on a range of
/// CPUs/MCUs. It has been tested on Arduino and STM32F0 platforms. It requires
/// a modern C++ compiler (C++11).
///
/// @section classes_sec Classes
///
/// There is only the NixieGlyphCrossfader::NixieGlyphCrossfader class. Read the
/// documentation of this class for all details.
///

namespace kbxTubeClock {


NixieGlyphCrossfader::NixieGlyphCrossfader()
  : _active(NixieGlyph()),
    _start(NixieGlyph()),
    _target(NixieGlyph())
{
}


bool NixieGlyphCrossfader::fadeIsActive()
{
  return (_active.getDuration() < _target.getDuration());
}


NixieGlyph NixieGlyphCrossfader::getActive() const
{
  return _active;
}


NixieGlyph NixieGlyphCrossfader::getTargetWithDuration() const
{
  return _target;
}


void NixieGlyphCrossfader::resetFade()
{
  // reset the crossfade timer/counter
  _active.setDuration(0);
  // where we are is where the fade starts now
  _start = _active;
}


void NixieGlyphCrossfader::startNewFade(const NixieGlyph &newTarget)
{
  _target = newTarget;
  resetFade();
}


void NixieGlyphCrossfader::startNewFadeIfDifferent(const NixieGlyph &newTarget)
{
  if (newTarget != _target)
  {
    _target = newTarget;
    resetFade();
  }
}


void NixieGlyphCrossfader::tick()
{
  // the _active NixieGlyph object's duration value is the current number of ticks into the fade
  uint32_t currentTick  = _active.getDuration(),
           totalTicks   = _target.getDuration(),
           percentTicks = NixieGlyph::cGlyph100Percent;

  if (currentTick < totalTicks)
  {
    if (totalTicks != 0)  // do not divide by zero omg
    {
      percentTicks = (NixieGlyph::cGlyph100Percent * currentTick) / totalTicks;
    }
    // set _active to the merged values of _start and _target
    _active.setFromMergedNixieGlyphs(percentTicks, _start, _target);
    _active.setDuration(currentTick + 1);
  }
  else if (currentTick >= totalTicks)
  {
    // ensure _active has reached the _target values
    _active = _target;
    // this kicks us into a "do nothing" state
    // _active.setDuration(currentTick + 1);
  }
}


void NixieGlyphCrossfader::updateFadeDuration(const uint32_t newDuration)
{
  _target.setDuration(newDuration);
}


void NixieGlyphCrossfader::updateFadeDuration(const NixieGlyph &newDuration)
{
  _target.setDuration(newDuration.getDuration());
}


void NixieGlyphCrossfader::updateFadeTarget(const NixieGlyph &newTarget, const bool startNewFade)
{
  auto duration = _target.getDuration();

  _target = newTarget;
  _target.setDuration(duration);

  if (startNewFade == true)
  {
    resetFade();
  }
}


void NixieGlyphCrossfader::updateFadeTargetAndDuration(const NixieGlyph &newTarget, const bool startNewFade)
{
  _target = newTarget;

  if (startNewFade == true)
  {
    resetFade();
  }
}


};
