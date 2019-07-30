//
// kbx81's Nixie Tube Library
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
#include "NixieTube.h"

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
/// There is only the NixieTube::NixieTube class. Read the documentation of
/// this class for all details.
///


namespace kbxTubeClock {


  NixieTube::NixieTube(const uint8_t glyph, const uint16_t intensity, const uint32_t duration)
    : _activeGlyph(glyph),
      _duration(duration),
      _intensity()
  {
    uint16_t safeIntensity = NixieGlyph::cGlyphMaximumIntensity;

    if (intensity < NixieGlyph::cGlyphMaximumIntensity)
    {
      safeIntensity = intensity;
    }

    for (uint8_t i = 0; i < cGlyphsPerTube; i++)
    {
      _intensity[i] = 0;
    }

    if (glyph < cGlyphsPerTube)
    {
      _intensity[glyph] = safeIntensity;
    }
    else
    {
      _activeGlyph = 0;
      _intensity[_activeGlyph] = safeIntensity;
    }
  }


  bool NixieTube::operator==(const NixieTube &other) const
  {
    for (uint8_t i = 0; i < cGlyphsPerTube; i++)
    {
      if (_intensity[i] != other._intensity[i])
      {
        return false;
      }
    }

    return _activeGlyph == other._activeGlyph &&
            _duration == other._duration;
  }


  bool NixieTube::operator!=(const NixieTube &other) const
  {
    for (uint8_t i = 0; i < cGlyphsPerTube; i++)
    {
      if (_intensity[i] != other._intensity[i])
      {
        return true;
      }
    }

    return _activeGlyph != other._activeGlyph ||
            _duration != other._duration;
  }


  void NixieTube::setGlyph(const uint8_t glyph)
  {
    if ((glyph < cGlyphsPerTube) && (glyph != _activeGlyph))
    {
      _intensity[glyph] = _intensity[_activeGlyph]; // new glyph on
      _intensity[_activeGlyph] = 0;                 // previous glyph off
      _activeGlyph = glyph;                   // update what's active
    }
  }


  void NixieTube::setIntensity(const uint16_t intensity)
  {
    setIntensity(_activeGlyph, intensity);
  }


  void NixieTube::setIntensity(const uint8_t glyph, const uint16_t intensity)
  {
    if (glyph < cGlyphsPerTube)
    {
      if (intensity < NixieGlyph::cGlyphMaximumIntensity)
      {
        _intensity[glyph] = intensity;
      }
      else
      {
        _intensity[glyph] = NixieGlyph::cGlyphMaximumIntensity;
      }
    }
  }


  void NixieTube::setDuration(const uint32_t duration)
  {
    _duration = duration;
  }


  void NixieTube::setOff()
  {
    for (uint8_t glyph = 0; glyph < cGlyphsPerTube; glyph++)
    {
      _intensity[glyph] = 0;
    }
  }


  uint8_t NixieTube::getGlyph() const
  {
    return _activeGlyph;
  }


  NixieGlyph NixieTube::getGlyphRaw() const
  {
    return getGlyphRaw(_activeGlyph);
  }


  NixieGlyph NixieTube::getGlyphRaw(const uint8_t glyph) const
  {
    if (glyph < cGlyphsPerTube)
    {
      return NixieGlyph(_intensity[glyph], _duration);
    }
    return NixieGlyph(_intensity[_activeGlyph], _duration);
  }


  uint8_t NixieTube::getIntensity() const
  {
    return _intensity[_activeGlyph];
  }


  uint8_t NixieTube::getIntensity(const uint8_t glyph) const
  {
    if (glyph < cGlyphsPerTube)
    {
      return _intensity[glyph];
    }

    return _intensity[_activeGlyph];
  }


  uint32_t NixieTube::getDuration() const
  {
    return _duration;
  }


  void NixieTube::adjustIntensity(const uint16_t percentageOfCurrentx100)
  {
    adjustIntensity(_activeGlyph, percentageOfCurrentx100);
  }


  void NixieTube::adjustIntensity(const uint8_t glyph, uint16_t percentageOfCurrentx100)
  {
    uint32_t top;

    if (glyph < cGlyphsPerTube)
    {
      if (percentageOfCurrentx100 > NixieGlyph::cGlyph100Percent)
      {
        percentageOfCurrentx100 = NixieGlyph::cGlyph100Percent;
      }

      top = _intensity[glyph] * percentageOfCurrentx100;
      _intensity[glyph] = top / NixieGlyph::cGlyph100Percent;
    }
  }


  void NixieTube::adjustIntensityAll(const uint16_t percentageOfCurrentx100)
  {
    for (uint8_t glyph = 0; glyph < NixieTube::cGlyphsPerTube; glyph++)
    {
      if (_intensity[glyph] > 0)
      {
        adjustIntensity(glyph, percentageOfCurrentx100);
      }
    }
  }


  void NixieTube::mergeWithNixieTube(uint16_t percentageOfOriginalTubex100, const NixieTube &tube)
  {
    int32_t intensity;

    if (percentageOfOriginalTubex100 > NixieGlyph::cGlyph100Percent)
    {
      percentageOfOriginalTubex100 = NixieGlyph::cGlyph100Percent;
    }

    for (uint8_t i = 0; i < NixieTube::cGlyphsPerTube; i++)
    {
      // new intensity = led0 - ((led0 - led1) * percentage)
      intensity = _intensity[i] - tube.getIntensity(i);
      intensity = _intensity[i] - ((intensity * percentageOfOriginalTubex100) / NixieGlyph::cGlyph100Percent);
      _intensity[i] = intensity;
    }
  }


  void NixieTube::setFromMergedNixieTubes(uint16_t percentageOfTube0x100, const NixieTube &tube0, const NixieTube &tube1)
  {
    int32_t intensity;

    if (percentageOfTube0x100 > NixieGlyph::cGlyph100Percent)
    {
      percentageOfTube0x100 = NixieGlyph::cGlyph100Percent;
    }

    for (uint8_t i = 0; i < NixieTube::cGlyphsPerTube; i++)
    {
      // new intensity = led0 - ((led0 - led1) * percentage)
      intensity = tube0.getIntensity(i) - tube1.getIntensity(i);
      intensity = tube0.getIntensity(i) - ((intensity * percentageOfTube0x100) / NixieGlyph::cGlyph100Percent);
      _intensity[i] = intensity;
    }
  }


}
