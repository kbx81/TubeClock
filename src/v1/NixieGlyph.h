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

class NixieGlyph {
 public:
  /// @brief Create a new NixieGlyph instance with specified intensity and duration
  ///
  /// @param intensity Initial value for glyph (0-255)
  /// @param duration Initial (fade) duration for this glyph
  ///
  NixieGlyph(const uint8_t intensity = 0, const uint32_t duration = 0);

 public:
  /// Important constants
  ///
  static const uint8_t cGlyphMaximumIntensity = 255;  // Maximum PWM intensity value

  /// @brief Compare this NixieGlyph to another
  /// @param LED to compare with
  ///
  bool operator==(const NixieGlyph &other) const;
  bool operator!=(const NixieGlyph &other) const;

  /// @brief Set the glyph to off (zero)
  ///
  void setOff();

  /// @brief Set the intensity of the glyph element
  /// @param intensity value for glyph (0-255)
  ///
  void setIntensity(const uint8_t intensity);

  /// @brief Set the duration over which the glyph should change
  /// @param duration value for glyph
  ///
  void setDuration(const uint32_t duration);

  /// @brief Get the intensity of the glpyh element
  /// @return intensity value for glyph (0-255)
  ///
  uint8_t getIntensity() const;

  /// @brief Get the duration over which the glyph should change
  /// @return fade duration used by glyph
  ///
  uint32_t getDuration() const;

  /// @brief Sets the nixie glyph to a linear interpolation between two other glyphs
  /// @param currentTick Current tick in the fade
  /// @param totalTicks Total ticks for the fade
  /// @param start Starting glyph intensity
  /// @param target Target glyph intensity
  ///
  void setFromLinearInterpolation(uint32_t currentTick, uint32_t totalTicks, const NixieGlyph &start,
                                  const NixieGlyph &target);

 private:
  uint8_t _intensity;  ///< Intensity of the glyph (0-255)
  uint32_t _duration;  ///< Duration over which this glyph should transition
};

}  // namespace kbxTubeClock
