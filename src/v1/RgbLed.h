//
// kbx81's RGB LED Library
// ---------------------------------------------------------------------------
// (c)2017 by kbx81. See LICENSE for details.
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

class RgbLed {
 public:
  /// @brief Create a new RgbLed instance with specified intensities and duration
  ///
  /// @param red Initial value for red element
  /// @param green Initial value for green element
  /// @param blue Initial value for blue element
  /// @param duration Initial (fade) duration for this LED
  ///
  RgbLed(const uint16_t red = 0, const uint16_t green = 0, const uint16_t blue = 0, const uint32_t duration = 0);

 public:
  /// Important constants
  ///
  static const uint32_t cLed100Percent   = 65536;  ///< Fixed-point 100%: 65536 = 100%, 32768 = 50%
  static const uint16_t cLedMaxIntensity = 4095;

  /// @brief Convert an integer percentage (0–100) to the fixed-point scale used by this class.
  /// @param p Percentage (0–100); pct(0) = 0, pct(50) ≈ 32767, pct(100) = 65535
  ///
  static constexpr uint16_t pct(uint8_t p) { return static_cast<uint16_t>((uint32_t(p) * 65535) / 100); }

  /// @brief Compare this RgbLed to another
  /// @param LED to compare with
  ///
  bool operator==(const RgbLed &other) const;
  bool operator!=(const RgbLed &other) const;

  /// @brief Set the intensity of the LED element
  /// @param intensity value for LED
  ///
  void setRed(const uint16_t intensity);
  void setGreen(const uint16_t intensity);
  void setBlue(const uint16_t intensity);
  void setRGB(const uint16_t intensityRed, const uint16_t intensityGreen, const uint16_t intensityBlue);

  /// @brief Set the RGB LED to off (zero)
  ///
  void setOff();

  /// @brief Set the duration over which the LED should change
  /// @param duration value for LED
  ///
  void setDuration(const uint32_t duration);

  /// @brief Get the intensity of the LED element
  /// @return intensity value for LED
  ///
  uint16_t getRed() const;
  uint16_t getGreen() const;
  uint16_t getBlue() const;

  /// @brief Get the duration over which the LED should change
  /// @return fade duration used by LED
  ///
  uint32_t getDuration() const;

  /// @brief Adjusts the RGB LED to a fraction of its original values.
  /// @param scaleFactor Scale factor: 0 = off, 32768 = 50%, 65535 = ~100%. Use pct() to convert from a percentage.
  ///
  void adjustIntensity(uint16_t scaleFactor);

  /// @brief Gamma corrects the RgbLed object (based on 12-bit values only).
  ///
  void gammaCorrect12bit();

 private:
  /// 4096-step (12 bit) brightness table: gamma = 2.2
  ///
  static const uint16_t cGammaTable[4096];

  uint16_t _red, _green, _blue;  ///< Intensities for Red, Green, and Blue elements
  uint32_t _duration;            ///< Duration over which this LED should transition
};

}  // namespace kbxTubeClock
