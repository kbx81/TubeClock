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
#include "Display.h"
#include "NixieGlyph.h"
#include "NixieTube.h"

/// @mainpage
///
/// @section intro_sec Introduction
///
/// This library contains a class to manage the Tube Clock's display.
///
/// @section requirements_sec Requirements
///
/// This library is written in a manner so as to be compatible on a range of
/// CPUs/MCUs. It has been tested on Arduino and STM32F0 platforms. It requires
/// a modern C++ compiler (C++11).
///
/// @section classes_sec Classes
///
/// There is only the Display::Display class. Read the documentation of this
/// class for all details.
///

namespace kbxTubeClock {


Display::Display()
{
  for (uint8_t tube = 0; tube < cTubeCount; tube++)
  {
    _tube[tube].setIntensity(NixieGlyph::cGlyphMaximumIntensity);
  }
}


Display::Display(const uint32_t word)
{
  for (uint8_t tube = 0; tube < cTubeCount; tube++)
  {
    _tube[tube].setIntensity(NixieGlyph::cGlyphMaximumIntensity);
  }

  setDisplayFromWord(word);
}


Display::Display(const uint8_t byte2, const uint8_t byte1, const uint8_t byte0)
{
  for (uint8_t tube = 0; tube < cTubeCount; tube++)
  {
    _tube[tube].setIntensity(NixieGlyph::cGlyphMaximumIntensity);
  }

  setDisplayFromBytes(byte2, byte1, byte0);
}


bool Display::operator==(const Display &other) const
{
  for (uint8_t i = 0; i < cTubeCount; i++)
  {
    if (_tube[i] != other._tube[i])
    {
      return false;
    }
  }

  return true;
}


bool Display::operator!=(const Display &other) const
{
  for (uint8_t i = 0; i < cTubeCount; i++)
  {
    if (_tube[i] != other._tube[i])
    {
      return true;
    }
  }

  return false;
}


void Display::setDisplayFromRaw(const NixieTube *data)
{
  for (uint8_t i = 0; i < cTubeCount; i++)
  {
    _tube[i] = data[i];
  }
}


void Display::setDisplayFromWord(const uint32_t word, const uint8_t bitmap)
{
  auto encoded = uint32ToBcd(word);

  for (uint8_t i = 0; i < cTubeCount; i++)
  {
    _tube[i].setGlyph((encoded >> (i * 4)) & 0x0f);
  }

  setTubesOff(bitmap);
}


void Display::setDisplayFromBytes(const uint8_t byte2, const uint8_t byte1, const uint8_t byte0, const uint8_t bitmap)
{
  auto encoded0 = uint32ToBcd(byte0),
       encoded1 = uint32ToBcd(byte1),
       encoded2 = uint32ToBcd(byte2);
  uint8_t i = 0;

  _tube[i++].setGlyph(encoded0 & 0x0f);
  _tube[i++].setGlyph((encoded0 >> 4) & 0x0f);
  _tube[i++].setGlyph(encoded1 & 0x0f);
  _tube[i++].setGlyph((encoded1 >> 4) & 0x0f);
  _tube[i++].setGlyph(encoded2 & 0x0f);
  _tube[i++].setGlyph((encoded2 >> 4) & 0x0f);

  setTubesOff(bitmap);
}


void Display::setDisplayFromNibbles(const uint8_t byte5, const uint8_t byte4, const uint8_t byte3, const uint8_t byte2, const uint8_t byte1, const uint8_t byte0, const uint8_t bitmap)
{
  uint8_t i = 0;

  _tube[i++].setGlyph(byte0);
  _tube[i++].setGlyph(byte1);
  _tube[i++].setGlyph(byte2);
  _tube[i++].setGlyph(byte3);
  _tube[i++].setGlyph(byte4);
  _tube[i++].setGlyph(byte5);

  setTubesOff(bitmap);
}


void Display::setDisplayFromDateTime(const DateTime dateTime, const uint8_t item, const bool bcd)
{
  switch (item)
  {
    case 0:
    setDisplayFromBytes(dateTime.yearShort(bcd), dateTime.month(bcd), dateTime.day(bcd));
    break;

    case 1:
    setDisplayFromBytes(dateTime.day(bcd), dateTime.month(bcd), dateTime.yearShort(bcd));
    break;

    case 2:
    setDisplayFromBytes(dateTime.month(bcd), dateTime.day(bcd), dateTime.yearShort(bcd));
    break;

    case 3:
    setDisplayFromBytes(dateTime.hour(bcd, true), dateTime.minute(bcd), dateTime.second(bcd));
    break;

    default:
    setDisplayFromBytes(dateTime.hour(bcd, false), dateTime.minute(bcd), dateTime.second(bcd));
  }
}


void Display::setDisplayFromDateTime(const DateTime dateTime, const dateTimeDisplaySelection item, const bool bcd)
{
  setDisplayFromDateTime(dateTime, static_cast<uint8_t>(item), bcd);
}


void Display::setTubeToValue(const uint8_t tubeNumber, const uint8_t tubeValue)
{
  if (tubeNumber < cTubeCount)
  {
    _tube[tubeNumber].setGlyph(tubeValue);
  }
}


void Display::setTubeFromRaw(const uint8_t tubeNumber, const NixieTube tube)
{
  if (tubeNumber < cTubeCount)
  {
    _tube[tubeNumber] = tube;
  }
}


void Display::setDot(const uint8_t dotNumber, NixieGlyph dot)
{
  if (dotNumber < cDotCount)
  {
    _dot[dotNumber] = dot;
  }
}


void Display::setDots(const uint32_t bitmap, const NixieGlyph dot, const bool setAllDotDurations)
{
  for (uint8_t i = 0; (i < cDotCount) && (((bitmap >> i) != 0) || (setAllDotDurations == true)); i++)
  {
    if (((bitmap >> i) & 1) == 1)
    {
      _dot[i] = dot;
    }
    else if (setAllDotDurations == true)
    {
      _dot[i].setDuration(dot.getDuration());
    }
  }
}


void Display::setDots(const uint32_t bitmap, const NixieGlyph dotOn, const NixieGlyph dotOff)
{
  for (uint8_t i = 0; i < cDotCount; i++)
  {
    if (((bitmap >> i) & 1) == 1)
    {
      _dot[i] = dotOn;
    }
    else
    {
      _dot[i] = dotOff;
    }
  }
}


void Display::setTubeDuration(const uint8_t tubeNumber, const uint16_t tubeDuration)
{
  if (tubeNumber < cTubeCount)
  {
    _tube[tubeNumber].setDuration(tubeDuration);
  }
}


void Display::setTubeDurations(const uint16_t tubeDuration, const uint8_t bitmap)
{
  for (uint8_t i = 0; (i < cTubeCount) && ((bitmap >> i) != 0); i++)
  {
    if (((bitmap >> i) & 1) == 1)
    {
      _tube[i].setDuration(tubeDuration);
    }
  }
}


void Display::setTubeIntensity(const uint8_t tubeNumber, const uint16_t tubeIntensity)
{
  if (tubeNumber < cTubeCount)
  {
    _tube[tubeNumber].setIntensity(tubeIntensity);
  }
}


void Display::setTubeIntensities(const uint16_t tubeIntensity, const uint8_t bitmap)
{
  for (uint8_t i = 0; (i < cTubeCount) && ((bitmap >> i) != 0); i++)
  {
    if (((bitmap >> i) & 1) == 1)
    {
      _tube[i].setIntensity(tubeIntensity);
    }
  }
}


void Display::setTubeIntensities(const uint16_t tubeIntensityHigh, const uint16_t tubeIntensityLow, const uint8_t bitmap)
{
  for (uint8_t i = 0; i < cTubeCount; i++)
  {
    if (((bitmap >> i) & 1) == 1)
    {
      _tube[i].setIntensity(tubeIntensityHigh);
    }
    else
    {
      _tube[i].setIntensity(tubeIntensityLow);
    }
  }
}


void Display::setTubeOff(const uint8_t tubeNumber)
{
  if (tubeNumber < cTubeCount)
  {
    _tube[tubeNumber].setOff();
  }
}


void Display::setTubesOff(const uint8_t bitmap)
{
  for (uint8_t i = 0; (i < cTubeCount) && ((bitmap >> i) != 0); i++)
  {
    if (((bitmap >> i) & 1) == 1)
    {
      _tube[i].setOff();
    }
  }
}


void Display::setMsdTubesOff(const uint8_t keepOnMask)
{
  uint8_t i = 0, tubesToTurnOff = 0;

  for (i = 0; i < cTubeCount; i++)
  {
    if (_tube[cTubeCount - 1 - i].getGlyph() == 0)
    {
      tubesToTurnOff |= (1 << (cTubeCount - 1 - i));
    }
    else
    {
      break;
    }
  }

  setTubesOff(tubesToTurnOff & keepOnMask);
}


uint8_t Display::getTubeIntensity(const uint8_t tubeNumber) const
{
  if (tubeNumber < cTubeCount)
  {
    return _tube[tubeNumber].getIntensity();
  }

  return 0;   // safe default
}


uint8_t Display::getTubeValue(const uint8_t tubeNumber) const
{
  if (tubeNumber < cTubeCount)
  {
    return _tube[tubeNumber].getGlyph();
  }

  return 0;   // safe default
}


NixieTube Display::getTubeRaw(const uint8_t tubeNumber) const
{
  if (tubeNumber < cTubeCount)
  {
    return _tube[tubeNumber];
  }

  return NixieTube();   // safe default
}


NixieGlyph Display::getDotRaw(const uint8_t dotNumber) const
{
  if (dotNumber < cDotCount)
  {
    return _dot[dotNumber];
  }

  return NixieGlyph();   // safe default
}


uint32_t Display::uint32ToBcd(uint32_t uint32Value)
{
    uint32_t result = 0;
    uint8_t shift = 0;

    if (uint32Value > 99999999)
    {
      uint32Value = 0;
    }

    while (uint32Value != 0)
    {
        result +=  (uint32Value % 10) << shift;
        uint32Value = uint32Value / 10;
        shift += 4;
    }
    return result;
}


};
