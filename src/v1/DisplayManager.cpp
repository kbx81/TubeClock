//
// kbx81's tube clock DisplayManager Library
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

// As hardware version 4+ uses the TLC5951 drivers, "master" dimming is
// implemented at the hardware level by leveraging the global brightness control
// (BC) and dot control (DC) mechanisms. This makes crossfading at (very) low
// intensity levels smoother and also reduces the workload on the CPU.
//
// That said, the display workflow might be the most confusing aspect of this
// codebase. The following flow diagram should help clarify how it works:
//
// writeDisplay()/writeStatusLed() ->
//   _loadCrossfaders() ->
//     refresh() updates crossfades when _refreshIntensitiesNow is set via tick() ->
//       _setDisplayPwmTriad() ->
//         _displayBufferOut (TCL59xx PWM data buffer) ->
//           DMA to SPI to TLC59xx shift registers & latches

#include "Display.h"
#include "DisplayManager.h"
#include "Hardware.h"
#include "NixieGlyph.h"
#include "NixieTube.h"
#include "NixieGlyphCrossfader.h"
#include "RgbLed.h"


namespace kbxTubeClock {

namespace DisplayManager
{

// length of delays used for doubleBlink confirmation; passed to Hardware::delay()
//
static const uint8_t cDoubleBlinkDuration = 50;

// total number of glyphs in all tubes
//
static const uint8_t cGlyphCount = Display::cGlyphCount + Display::cDotCount;

// number of HV562x devices we're controlling/are linked together
//
static const uint8_t cPwmNumberOfDevices = 3;

// number of channels/outputs per HV562x device
//
static const uint8_t cPwmChannelsPerDevice = 32;

// number of bytes to send via the SPI to HV562x devices
//
static const uint8_t cSpiBytesToSend = (cPwmChannelsPerDevice * cPwmNumberOfDevices) / 8;

// SoftPWM tick counter
//
static uint16_t _pwmTickCounter = 0;

// SoftPWM values
//
static uint16_t _pwmValues[cPwmNumberOfDevices * cPwmChannelsPerDevice];

// HV562x buffers
//
static uint32_t _displayBufferIn[cPwmNumberOfDevices];
static uint32_t _displayBufferOut[cPwmNumberOfDevices];

// display buffers for processing crossfades
//
static RgbLed _statusLed;

// LED crossfaders (main display + status LED)
//
static NixieGlyphCrossfader _crossfader[cGlyphCount];

// true if status LED should be refreshed by refresh()
//
static bool _autoRefreshStatusLed = false;

// if true, BLANK pin will be held high to turn off all LEDs
//
static bool _displayBlank = true;

// master display intensity times 100
//   10000 = 100%
static uint16_t _intensityPercentage = 0;

// set by tick() to true so refresh() will refresh the NixieTubeCrossfaders
//
volatile static bool _refreshIntensitiesNow = false;

// TLC59xx refresh interval; slows down refreshing of the drivers to reduce
//  flicker on boards with TLC5947 ICs. not useful for the TLC5951s.
static uint8_t _driverRefreshInterval = 0;


// Modifies a triad of PWM channels in the buffer corresponding to a single RGB LED (pixel)
//
void _setDisplayPwmValue(const uint8_t glyphNumber, const uint16_t glyphValue)
{
  if (glyphNumber < cGlyphCount)
  {
    _pwmValues[(cPwmNumberOfDevices * cPwmChannelsPerDevice) - 1 - glyphNumber] = glyphValue;
  }
  // status LED
  // else if (ledNumber == Display::cPixelCount)
  // {
  //   _statusLed = ledValue;
  // }
}


void initialize()
{
  // Initialize the display drivers' global brightness and dot control levels
  setMasterIntensity(NixieGlyph::cGlyph100Percent);

  _displayBufferOut[0] = 0;
  _displayBufferOut[1] = 0;
  _displayBufferOut[2] = 0;

  for (uint8_t i = 0; i < cPwmNumberOfDevices * cPwmChannelsPerDevice; i++)
  {
    _pwmValues[i] = 0;
  }

  // Write the first block of data to the drivers (to zero them out)
  while (Hardware::spiTransfer(Hardware::SpiPeripheral::HvDrivers, (uint8_t*)_displayBufferIn, (uint8_t*)_displayBufferOut, cSpiBytesToSend, false) != Hardware::HwReqAck::HwReqAckOk);
}


void refresh()
{
  if (_refreshIntensitiesNow == true)
  {
    NixieGlyph activeGlyph;

    // refresh all crossfaders, adjust intensities, write to the PWM buffer
    for (uint8_t glyphCrossfader = 0; glyphCrossfader < cGlyphCount; glyphCrossfader++)
    {
      _crossfader[glyphCrossfader].tick();

      activeGlyph = _crossfader[glyphCrossfader].getActive();
      // adjust the brightness based on _intensityPercentage
      activeGlyph.adjustIntensity(_intensityPercentage);

      // activeTube.gammaCorrect12bit();

      _setDisplayPwmValue(glyphCrossfader, activeGlyph.getIntensity());
    }

    _refreshIntensitiesNow = false;  // all done...until the next tick...
  }
}


void tick()
{
  _refreshIntensitiesNow = true;
  // refresh the status LED here to play nicely with strobing via DMX-512
  if (_autoRefreshStatusLed == true)
  {
    if (_displayBlank == true)
    {
      Hardware::setStatusLed(RgbLed());
    }
    else
    {
      RgbLed status = _statusLed;
      status.adjustIntensity(_intensityPercentage);
      Hardware::setStatusLed(status);
    }
  }
}


void tickPWM()
{
  uint32_t pwmBits = 0;

  for (uint8_t d = 0; d < cPwmNumberOfDevices; d++)
  {
    for (uint8_t i = 0; i < cPwmChannelsPerDevice; i++)
    {
      if (_pwmValues[i + (d * cPwmChannelsPerDevice)] > _pwmTickCounter)
      {
        pwmBits |= (1 << i);
      }
    }
    _displayBufferOut[d] = pwmBits;
    pwmBits = 0;
  }

  // Write the data to the drivers
  Hardware::spiTransfer(Hardware::SpiPeripheral::HvDrivers, (uint8_t*)_displayBufferIn, (uint8_t*)_displayBufferOut, cSpiBytesToSend, false);

  // increment the tick counter and reset it if it's time
  if (++_pwmTickCounter > NixieGlyph::cGlyphMaximumIntensity)
  {
    _pwmTickCounter = 0;
  }
}


void setStatusLedAutoRefreshing(const bool autoRefreshEnabled)
{
  _autoRefreshStatusLed = autoRefreshEnabled;
}


uint16_t getMasterIntensity()
{
  return _intensityPercentage;
}


void setMasterIntensity(const uint16_t intensity)
{
  // ensure the passed intensity value is appropriate & safe
  if (intensity < NixieGlyph::cGlyph100Percent)
  {
    _intensityPercentage = intensity;
  }
  else
  {
    _intensityPercentage = NixieGlyph::cGlyph100Percent;
  }
}


bool getDisplayBlanking()
{
  return _displayBlank;
}


void setDisplayBlanking(const bool blank)
{
  if (_displayBlank != blank)
  {
    _displayBlank = blank;

    if (blank == true)
    {
      if (_autoRefreshStatusLed == true)
      {
        Hardware::setStatusLed(RgbLed());
      }
      // blank the display
      Hardware::setDisplayHardwareBlanking(true);
    }
    else
    {
      if (_autoRefreshStatusLed == true)
      {
        Hardware::setStatusLed(_statusLed);
      }
      // unblank the display
      Hardware::setDisplayHardwareBlanking(false);
    }
  }
}


uint8_t getDisplayRefreshInterval()
{
  return _driverRefreshInterval;
}


void setDisplayRefreshInterval(const uint8_t interval)
{
  _driverRefreshInterval = interval;
}


void doubleBlink()
{
  setDisplayBlanking(true);
  Hardware::delay(cDoubleBlinkDuration);
  setDisplayBlanking(false);
  Hardware::delay(cDoubleBlinkDuration);
  setDisplayBlanking(true);
  Hardware::delay(cDoubleBlinkDuration);
  setDisplayBlanking(false);
}


// Updates all main display LEDs in the crossfade buffers
//
void _loadCrossfaders(const Display &display)
{
  for (uint8_t tube = 0; tube < Display::cTubeCount; tube++)
  {
    for (uint8_t glyph = 0; glyph < NixieTube::cGlyphsPerTube; glyph++)
    {
      _crossfader[(tube * NixieTube::cGlyphsPerTube) + glyph].startNewFadeIfDifferent(display.getTubeRaw(tube).getGlyphRaw(NixieTube::cGlyphsPerTube - 1 - glyph));
    }
  }
  // ...and here we load the dot crossfaders
  for (uint8_t dot = 0; dot < Display::cDotCount; dot++)
  {
    _crossfader[(Display::cGlyphCount) + dot].startNewFadeIfDifferent(display.getDotRaw(dot));
  }
}


void writeDisplay(const Display &display)
{
  _loadCrossfaders(display);
}


void writeDisplay(const Display &display, const RgbLed &statusLed)
{
  _loadCrossfaders(display);

  // _crossfader[Display::cPixelCount].startNewFadeIfDifferent(statusLed);
  _statusLed = statusLed;
}


void writeStatusLed(const RgbLed &statusLed)
{
  // _crossfader[Display::cPixelCount].startNewFadeIfDifferent(statusLed);
  _statusLed = statusLed;
}


}

}
