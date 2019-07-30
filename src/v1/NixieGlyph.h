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
#pragma once


#include <cstdint>


namespace kbxTubeClock {

class NixieGlyph
{
public:
  /// @brief Create a new NixieGlyph instance with specified intensity and duration
  ///
  /// @param intensity Initial value for glyph
  /// @param duration Initial (fade) duration for this glyph
  ///
  NixieGlyph(const uint16_t intensity = 0, const uint32_t duration = 0);

public:
  /// Important constants
  ///
  static const uint16_t cGlyph100Percent = 10000;
  static const uint16_t cGlyphMaximumIntensity = 100;


  /// @brief Compare this NixieGlyph to another
  /// @param LED to compare with
  ///
  bool operator==(const NixieGlyph &other) const;
  bool operator!=(const NixieGlyph &other) const;

  /// @brief Set the glyph to off (zero)
  ///
  void setOff();

  /// @brief Set the intensity of the glyph element
  /// @param intensity value for glyph
  ///
  void setIntensity(const uint16_t intensity);

  /// @brief Set the duration over which the glyph should change
  /// @param duration value for glyph
  ///
  void setDuration(const uint32_t duration);

  /// @brief Get the intensity of the glpyh element
  /// @return intensity value for glyph
  ///
  uint16_t getIntensity() const;

  /// @brief Get the duration over which the LED should change
  /// @return fade duration used by LED
  ///
  uint32_t getDuration() const;

  /// @brief Adjusts the glyph to a percentage of its original value
  /// @param percentageOfCurrentx100 percentage of original intensity. Percentages are times 100 -- e.g.: 8765 = 87.65%
  ///
  void adjustIntensity(uint16_t percentageOfCurrentx100);

  /// @brief Merges the nixie glyph with another.
  /// @param percentageOfOriginalGlyphx100 percentage of original NixieGlyph. Percentages are times 100 -- e.g.: 8765 = 87.65%
  ///
  void mergeWithNixieGlyph(uint16_t percentageOfOriginalGlyphx100, const NixieGlyph &glyph);

  /// @brief Sets the nixie tube to values obtained by merging two other NixieGlyph objects.
  /// @param percentageOfGlyph0x100 percentage of first NixieGlyph. Percentages are times 100 -- e.g.: 8765 = 87.65%
  ///
  void setFromMergedNixieGlyphs(uint16_t percentageOfGlyph0x100, const NixieGlyph &glyph0, const NixieGlyph &glyph1);

  /// @brief Gamma corrects the NixieGlyph object (based on 12-bit values only).
  ///
  void gammaCorrect12bit();

private:
  /// 4096-step (12 bit) brightness table: gamma = 2.2
  ///
  static const uint16_t cGammaTable[4096];

  uint16_t _intensity;      ///< Intensity of the glyph
  uint32_t _duration;       ///< Duration over which this LED should transition
};

}
