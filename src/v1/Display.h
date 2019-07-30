//
// kbx81's tube clock Display Library
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

#include "DateTime.h"
#include "NixieGlyph.h"
#include "NixieTube.h"


namespace kbxTubeClock {

/// @brief kbx Tube Clock Display class
///

class Display
{

public:
  /// @brief Selection of time or date (and date format) for setDisplayFromDateTime()
  ///
  enum dateTimeDisplaySelection : uint8_t
  {
    dateDisplayYYMMDD = 0,
    dateDisplayDDMMYY = 1,
    dateDisplayMMDDYY = 2,
    timeDisplay12Hour = 3,
    timeDisplay24Hour = 4
  };

public:
  /// @brief Default constructor
  ///
  Display();

  /// @brief Create a new display instance from a single word
  ///
  /// @param word word to be displayed
  ///
  Display(const uint32_t word);

  /// @brief Create a new display instance from three bytes
  ///
  /// @param byte0 LSB to be displayed
  /// @param byte1 ... to be displayed
  /// @param byte2 MSB to be displayed
  ///
  Display(const uint8_t byte2, const uint8_t byte1, const uint8_t byte0);

public:
  /// Important constants
  ///
  static const uint8_t cTubeCount = 6;
  static const uint8_t cGlyphCount = (cTubeCount * NixieTube::cGlyphsPerTube);
  static const uint8_t cDotCount   = (cTubeCount * NixieTube::cPointsPerTube) + 4;  // + 4 for colons

  /// Compare this Display to another
  ///
  bool operator==(const Display &other) const;
  bool operator!=(const Display &other) const;

  /// @brief Set this display's values from raw data
  ///
  /// @param data Pointer to 6 NixieTube objects used to set this display's state
  ///
  void setDisplayFromRaw(const NixieTube *data);

  /// @brief Set a display instance from a single word
  ///
  /// @param word word to be displayed
  /// @param bitmap Bitmap of tubes to be turned off via setTubesOff() (optional)
  ///
  void setDisplayFromWord(const uint32_t word, const uint8_t bitmap = 0);

  /// @brief Set a display instance from three bytes
  ///
  /// @param byte0 LSB to be displayed
  /// @param byte1 ... to be displayed
  /// @param byte2 MSB to be displayed
  /// @param bitmap Bitmap of tubes to be turned off via setTubesOff() (optional)
  ///
  void setDisplayFromBytes(const uint8_t byte2, const uint8_t byte1, const uint8_t byte0, const uint8_t bitmap = 0);

  /// @brief Set a display instance from six nibbles
  ///
  /// @param byte0 LSn to be displayed
  /// @param byte1 ... to be displayed
  /// @param byte2 ... to be displayed
  /// @param byte3 ... to be displayed
  /// @param byte4 ... to be displayed
  /// @param byte5 MSn to be displayed
  /// @param bitmap Bitmap of tubes to be turned off via setTubesOff() (optional)
  ///
  void setDisplayFromNibbles(const uint8_t byte5, const uint8_t byte4, const uint8_t byte3, const uint8_t byte2, const uint8_t byte1, const uint8_t byte0, const uint8_t bitmap = 0);

  /// @brief Set a display instance from a DateTime object
  ///
  /// @param dateTime DateTime object to be used to set the display
  /// @param item Selects time or date display (and date format if date)
  ///
  void setDisplayFromDateTime(const DateTime dateTime, const uint8_t item, const bool bcd = false);
  void setDisplayFromDateTime(const DateTime dateTime, const dateTimeDisplaySelection item, const bool bcd = false);

  /// @brief Set a single tube's state
  ///
  /// @param tubeNumber Number of tube to set (0 through 5)
  /// @param tubeValue tube's value (0 through 9)
  ///
  void setTubeToValue(const uint8_t tubeNumber, const uint8_t tubeValue);

  /// @brief Set a single tube's state from a set of glyphs
  ///
  /// @param tubeNumber Number of tube to set (0 through 5)
  /// @param tube NixieTube object used to set this tube's state
  ///
  void setTubeFromRaw(const uint8_t tubeNumber, const NixieTube tube);

  /// @brief Set (tube) dot
  ///
  /// @param dotNumber Number of dot to set (0 through 16, first four are colons)
  /// @param dot NixieGlyph object used to set this dot's state
  ///
  void setDot(const uint8_t dotNumber, NixieGlyph dot);

  /// @brief Set multiple (tube) dots
  ///
  /// @param bitmap Bitmap of dots to set (first four bits are colons)
  /// @param dot NixieGlyph object used to set the selected dot(s) state
  /// @param setAllDotDurations If true, set *all* dot durations
  ///
  void setDots(const uint32_t bitmap, const NixieGlyph dot, const bool setAllDotDurations = false);

  /// @brief Set all (tube) dots
  ///
  /// @param bitmap Bitmap of dot states (first four bits are colons)
  /// @param dotOn NixieGlyph object used to set the "on" dot(s) state
  /// @param dotOff NixieGlyph object used to set the "off" dot(s) state
  ///
  void setDots(const uint32_t bitmap, const NixieGlyph dotOn, const NixieGlyph dotOff);

  /// @brief Set a single tube's (fade) duration
  ///
  /// @param tubeNumber Number of tube to set (0 through 5)
  /// @param tubeDuration tube's (fade) duration
  ///
  void setTubeDuration(const uint8_t tubeNumber, const uint16_t tubeDuration);

  /// @brief Set tube (fade) duration(s)
  ///
  /// @param tubeDuration tube (fade) duration
  /// @param bitmap Bitmap of tube(s) to set duration for
  ///
  void setTubeDurations(const uint16_t tubeDuration, const uint8_t bitmap = 0b111111);

  /// @brief Set a single tube's intensity
  ///
  /// @param tubeNumber Number of tube to set (0 through 5)
  /// @param tubeIntensity tube's new intensity
  ///
  void setTubeIntensity(const uint8_t tubeNumber, const uint16_t tubeIntensity);

  /// @brief Set multipe tube intensities
  ///
  /// @param tubeIntensity Tube intensity
  /// @param bitmap Bitmap of tube(s) to set intensity for
  ///
  void setTubeIntensities(const uint16_t tubeIntensity, const uint8_t bitmap = 0b111111);

  /// @brief Set all tube intensities
  ///
  /// @param tubeIntensityHigh Tube intensity for "on" tubes
  /// @param tubeIntensityLow Tube intensity for "off" tubes
  /// @param bitmap Bitmap of tube states (1 = high/on, 0 = low/off)
  ///
  void setTubeIntensities(const uint16_t tubeIntensityHigh, const uint16_t tubeIntensityLow, const uint8_t bitmap);

  /// @brief Set a single pixel to be off
  ///
  /// @param tubeNumber Number of tube to turn off (0 through 5)
  ///
  void setTubeOff(const uint8_t tubeNumber);

  /// @brief Set a number tubes to be off
  ///
  /// @param bitmap Bitmap of tubes to be turned off
  ///
  void setTubesOff(const uint8_t bitmap = 0b111111);

  /// @brief Set MSb pixels in a binary or BCD display to be off
  ///
  /// @param keepOnMask passed through to setTubesOff() to keep selected tubes on
  ///
  void setMsdTubesOff(const uint8_t keepOnMask = 0xfe);

  /// @brief Get a single tube's intensity
  ///
  /// @param tubeNumber Number of tube to set (0 through 5)
  /// @return Intensity value of active glyph in tube
  ///
  uint8_t getTubeIntensity(const uint8_t tubeNumber) const;

  /// @brief Get a single tube's value
  ///
  /// @param tubeNumber Number of tube to set (0 through 5)
  /// @return Value tube displays
  ///
  uint8_t getTubeValue(const uint8_t tubeNumber) const;

  /// @brief Get a single tube's raw state
  ///
  /// @param tubeNumber Number of tube to get (0 through 5)
  /// @return Pointer to NixieTube with data from specified tube
  ///
  NixieTube getTubeRaw(const uint8_t tubeNumber) const;

  /// @brief Get a single dot's raw state
  ///
  /// @param dotNumber Number of dot to get (0 through 16)
  /// @return Pointer to NixieGlyph with data from specified dot
  ///
  NixieGlyph getDotRaw(const uint8_t dotNumber) const;


private:
  /// @brief Converts from uint32 to BCD for tube display; beware of values > 99999999
  /// @param uint32Value Value to BCD encode
  /// @return encoded value
  ///
  uint32_t uint32ToBcd(uint32_t uint32Value);

  NixieTube _tube[cTubeCount];    ///< values currently active on display
  NixieGlyph _dot[cDotCount];     ///< values currently active on display
};


}
