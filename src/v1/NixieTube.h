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

class NixieTube {
 public:
  /// @brief Create a new NixieTube instance; we assume 10 glyphs and 2 points
  ///
  /// @param glyph Initial active glyph (0 - 9)
  /// @param intensity Initial intensity value for active glyph (0-255)
  /// @param duration Initial duration value for glyph transitions
  ///
  NixieTube(const uint8_t glyph = 0, const uint8_t intensity = NixieGlyph::cGlyphMaximumIntensity,
            const uint32_t duration = 0);

 public:
  /// Important constants
  ///
  static const uint8_t  cGlyphsPerTube  = 10;
  static const uint8_t  cPointsPerTube  = 2;
  static const uint32_t cGlyph100Percent = 65536;  ///< Fixed-point 100%: 65536 = 100%, 32768 = 50%

  /// @brief Convert an integer percentage (0–100) to the fixed-point scale used by this class.
  /// @param p Percentage (0–100); pct(0) = 0, pct(50) ≈ 32767, pct(100) = 65535
  ///
  static constexpr uint16_t pct(uint8_t p) { return static_cast<uint16_t>((uint32_t(p) * 65535) / 100); }

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
  /// @param intensity value for glyph (0-255)
  /// @param glyph Glyph number to set
  ///
  void setIntensity(const uint8_t intensity);
  void setIntensity(const uint8_t glyph, const uint8_t intensity);

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

  /// @brief Adjusts a specific glyph's intensity by a scale factor (65536 = 100%, i.e. no change)
  /// @param glyph Glyph number to scale
  /// @param scaleFactor Scale factor: 0 = off, 32768 = 50%, 65535 = ~100%. Use pct() to convert from a percentage.
  ///
  void adjustIntensity(const uint8_t glyph, const uint16_t scaleFactor);

  /// @brief Adjusts all glyphs' intensities by a scale factor (65536 = 100%, i.e. no change)
  /// @param scaleFactor Scale factor: 0 = off, 32768 = 50%, 65535 = ~100%. Use pct() to convert from a percentage.
  ///
  void adjustIntensityAll(const uint16_t scaleFactor);

  /// @brief Linearly interpolates this tube's intensities toward a target tube.
  /// @param blendFactor Interpolation factor: 0 = no change, 65535 = fully adopt target. Use pct() to convert.
  /// @param target The tube to interpolate toward
  ///
  void lerpToward(uint16_t blendFactor, const NixieTube &target);

  /// @brief Sets this tube's intensities to the linear interpolation of two other tubes.
  /// @param blendFactor Interpolation factor: 0 = fully from, 65535 = fully to. Use pct() to convert.
  /// @param from The start tube (result when blendFactor = 0)
  /// @param to The end tube (result approaches this when blendFactor = 65535)
  ///
  void setFromLerp(uint16_t blendFactor, const NixieTube &from, const NixieTube &to);

 private:
  uint32_t _duration;  ///< Duration over which this tube should transition
  uint8_t _intensity[cGlyphsPerTube];
  uint8_t _activeGlyph;
};

}  // namespace kbxTubeClock
