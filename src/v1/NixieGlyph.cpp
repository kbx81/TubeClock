//
// kbx81's Nixie Glyph Library
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

/// @mainpage
///
/// @section intro_sec Introduction
///
/// This library contains a class to manage the Tube Clock's glyphs.
///
/// @section requirements_sec Requirements
///
/// This library is written in a manner so as to be compatible on a range of
/// CPUs/MCUs. It has been tested on Arduino and STM32F0 platforms. It requires
/// a modern C++ compiler (C++11).
///
/// @section classes_sec Classes
///
/// There is only the NixieGlyph::NixieGlyph class. Read the documentation of
/// this class for all details.
///


namespace kbxTubeClock {


  NixieGlyph::NixieGlyph(const uint8_t intensity, const uint32_t duration)
    : _intensity(intensity),
      _duration(duration)
  {
  }


  bool NixieGlyph::operator==(const NixieGlyph &other) const
  {
    return _intensity == other._intensity &&
            _duration == other._duration;
  }


  bool NixieGlyph::operator!=(const NixieGlyph &other) const
  {
    return _intensity != other._intensity ||
            _duration != other._duration;
  }


  void NixieGlyph::setOff()
  {
    _intensity = 0;
  }


  void NixieGlyph::setIntensity(const uint8_t intensity)
  {
    _intensity = intensity;
  }


  void NixieGlyph::setDuration(const uint32_t duration)
  {
    _duration = duration;
  }


  uint8_t NixieGlyph::getIntensity() const
  {
    return _intensity;
  }


  uint32_t NixieGlyph::getDuration() const
  {
    return _duration;
  }


  void NixieGlyph::setFromLinearInterpolation(uint32_t currentTick, uint32_t totalTicks, const NixieGlyph &start, const NixieGlyph &target)
  {
    // Early exit: if at or past target, snap to target intensity
    // This also handles totalTicks == 0 case (currentTick >= 0 is always true)
    if (currentTick >= totalTicks)
    {
      _intensity = target._intensity;
      return;
    }

    // Optimized linear interpolation avoiding division on Cortex-M0
    // Division takes 18-35 cycles, multiplication takes ~1 cycle
    //
    // Original formula: intensity = start + ((delta * currentTick) / totalTicks)
    //
    // Optimization strategy: Use bit shifts for power-of-2 denominators,
    // otherwise fall back to standard division (compiler will optimize this)

    int32_t delta = static_cast<int32_t>(target._intensity) - static_cast<int32_t>(start._intensity);
    int32_t progress;

    // Optimize for common fade durations that are powers of 2
    // These can use right-shift instead of division (1-2 cycles vs 18-35 cycles)
    if (totalTicks == 128)
    {
      progress = (delta * static_cast<int32_t>(currentTick)) >> 7;
    }
    else if (totalTicks == 256)
    {
      progress = (delta * static_cast<int32_t>(currentTick)) >> 8;
    }
    else if (totalTicks == 512)
    {
      progress = (delta * static_cast<int32_t>(currentTick)) >> 9;
    }
    else if (totalTicks == 1024)
    {
      progress = (delta * static_cast<int32_t>(currentTick)) >> 10;
    }
    else
    {
      // Fall back to division for non-power-of-2 durations
      // Still saves cycles for the common cases above
      progress = (delta * static_cast<int32_t>(currentTick)) / static_cast<int32_t>(totalTicks);
    }

    int32_t interpolated = static_cast<int32_t>(start._intensity) + progress;
    _intensity = static_cast<uint8_t>(interpolated);
  }


}
