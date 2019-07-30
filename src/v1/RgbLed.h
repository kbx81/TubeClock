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

class RgbLed
{
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
  static const uint16_t cLed100Percent   = 10000;
  static const uint16_t cLedMaxIntensity =  4095;


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

  /// @brief Adjusts the RGB LED to a percentage of original values.
  /// @param percentageOfCurrentx100 percentage of original intensity. Percentages are times 100 -- e.g.: 8765 = 87.65%
  ///
  void adjustIntensity(uint16_t percentageOfCurrentx100);

  /// @brief Merges the RGB LED with another.
  /// @param percentageOfOriginalLedx100 percentage of original RgbLed. Percentages are times 100 -- e.g.: 8765 = 87.65%
  ///
  void mergeWithRgbLed(uint16_t percentageOfOriginalLedx100, const RgbLed &led);

  /// @brief Sets the RGB LED to values obtained by merging two other RgbLed objects.
  /// @param percentageOfLed0x100 percentage of first RgbLed. Percentages are times 100 -- e.g.: 8765 = 87.65%
  ///
  void setFromMergedRgbLeds(uint16_t percentageOfLed0x100, const RgbLed &led0, const RgbLed &led1);

  /// @brief Gamma corrects the RgbLed object (based on 12-bit values only).
  ///
  void gammaCorrect12bit();

private:
  /// 4096-step (12 bit) brightness table: gamma = 2.2
  ///
  static const uint16_t cGammaTable[4096];

  uint16_t _red, _green, _blue; ///< Intensities for Red, Green, and Blue elements
  uint32_t _duration;           ///< Duration over which this LED should transition
};

}
