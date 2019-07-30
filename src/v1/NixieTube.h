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
#pragma once

#include "NixieGlyph.h"

#include <cstdint>


namespace kbxTubeClock {

class NixieTube
{
public:
  /// @brief Create a new NixieTube instance; we assume 10 glyphs and 2 points
  ///
  /// @param glyph Initial active glyph (0 - 9)
  /// @param intensity Initial intensity value for active glyph
  /// @param duration Initial duration value for glyph transitions
  ///
  NixieTube(const uint8_t glyph = 0, const uint16_t intensity = NixieGlyph::cGlyphMaximumIntensity, const uint32_t duration = 0);

public:
  /// Important constants
  ///
  static const uint8_t cGlyphsPerTube = 10;
  static const uint8_t cPointsPerTube = 2;


  /// @brief Compare this NixieTube to another
  /// @param LED to compare with
  ///
  bool operator==(const NixieTube &other) const;
  bool operator!=(const NixieTube &other) const;

  /// @brief Set the active glyph for the tube
  /// @param glyph New active glyph for tube
  ///
  void setGlyph(const uint8_t glyph);

  /// @brief Sets the (active) glyph's intensity
  /// @param intensity value for glyph
  /// @param glyph Glyph number to set
  ///
  void setIntensity(const uint16_t intensity);
  void setIntensity(const uint8_t glyph, const uint16_t intensity);

  /// @brief Set the duration over which the tube should transition glyphs
  /// @param duration New value for glyph transitions
  ///
  void setDuration(const uint32_t duration);

  /// @brief Set the tube's active glyph to off (no glyphs illuminated)
  ///
  void setOff();

  /// @brief Get the tube's currently active glpyh
  /// @return active glyph number
  ///
  uint8_t getGlyph() const;

  /// @brief Get the tube's currently active glpyh as a NixieGlyph object
  /// @param glyph Glyph number to return
  /// @return active raw glyph
  ///
  NixieGlyph getGlyphRaw() const;
  NixieGlyph getGlyphRaw(const uint8_t glyph) const;

  /// @brief Get the tube's currently active glpyh
  /// @param glyph Glyph to get intensity of; returns active glyph if not supplied
  /// @return active glyph number
  ///
  uint8_t getIntensity() const;
  uint8_t getIntensity(const uint8_t glyph) const;

  /// @brief Get the duration over which the tube should transition glyphs
  /// @return Duration used for this tube's glyph transitions
  ///
  uint32_t getDuration() const;

  /// @brief Adjusts the (active) glyph's intensity to a percentage of its original value
  /// @param percentageOfCurrentx100 percentage of original intensity. Percentages are times 100 -- e.g.: 8765 = 87.65%
  /// @param glyph Glyph number to adjust
  ///
  void adjustIntensity(const uint16_t percentageOfCurrentx100);
  void adjustIntensity(const uint8_t glyph, uint16_t percentageOfCurrentx100);
  void adjustIntensityAll(const uint16_t percentageOfCurrentx100);

  /// @brief Merges the nixie tube with another.
  /// @param percentageOfOriginalTubex100 percentage of original NixieTube. Percentages are times 100 -- e.g.: 8765 = 87.65%
  ///
  void mergeWithNixieTube(uint16_t percentageOfOriginalTubex100, const NixieTube &tube);

  /// @brief Sets the nixie tube to values obtained by merging two other NixieTube objects.
  /// @param percentageOfLed0x100 percentage of first NixieTube. Percentages are times 100 -- e.g.: 8765 = 87.65%
  ///
  void setFromMergedNixieTubes(uint16_t percentageOfTube0x100, const NixieTube &tube0, const NixieTube &tube1);

private:
  uint8_t  _activeGlyph;
  uint32_t _duration;       ///< Duration over which this LED should transition
  uint16_t _intensity[cGlyphsPerTube];
};

}
