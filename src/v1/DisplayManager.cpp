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

#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include "Display.h"
#include "DisplayManager.h"
#include "Hardware.h"
#include "NixieGlyph.h"
#include "NixieTube.h"
#include "NixieGlyphCrossfader.h"
#include "RgbLed.h"
#include "SpiMaster.h"

#if HARDWARE_VERSION == 1
  #include "Hardware_v1.h"
#else
  #error HARDWARE_VERSION must be defined with a value of 1
#endif

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

// Hardware refresh interval
//
static uint8_t _driverRefreshInterval = 0;

// Pointer to SpiMaster, initialized by initialize()
//
SpiMaster *_spiMaster = nullptr;

// Slave ID assigned by SpiMaster
//
static uint8_t _slaveId = SpiMaster::cNoSlave;


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
  SpiMaster::SpiTransferReq *request = nullptr;
  SpiMaster::SpiSlave mySlave = {
    .gpioPort       = Hardware::cNssPort,       // gpio port on which CS line lives
    .gpioPin        = Hardware::cNssDisplayPin, // gpio pin on which CS line lives
    .strobeCs       = true,                     // CS line is strobed upon xfer completion if true
    .polarity       = true,                     // CS/CE polarity (true = active high)
    .misoPort       = Hardware::cSpi1AltPort,           // port on which slave inputs data
    .misoPin        = Hardware::cSpi1MisoDisplayPin,    // pin on which slave inputs data
    .br             = SPI_CR1_BAUDRATE_FPCLK_DIV_8,     // Baudrate
    .cpol           = SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,  // Clock polarity
    .cpha           = SPI_CR1_CPHA_CLK_TRANSITION_1,    // Clock Phase
    .lsbFirst       = SPI_CR1_LSBFIRST,     // Frame format -- lsb/msb first
    .dataSize       = SPI_CR2_DS_8BIT,      // Data size (4 to 16 bits, see RM)
    .memorySize     = DMA_CCR_MSIZE_8BIT,   // Memory word width (8, 16, 32 bit)
    .peripheralSize = DMA_CCR_PSIZE_8BIT    // Peripheral word width (8, 16, 32 bit)
  };

  // Initialize the display drivers' global brightness and dot control levels
  setMasterIntensity(NixieGlyph::cGlyph100Percent);

  _displayBufferOut[0] = 0;
  _displayBufferOut[1] = 0;
  _displayBufferOut[2] = 0;

  for (uint8_t i = 0; i < cPwmNumberOfDevices * cPwmChannelsPerDevice; i++)
  {
    _pwmValues[i] = 0;
  }

  _spiMaster = Hardware::getSpiMaster();

  _slaveId = _spiMaster->registerSlave(&mySlave);

  request = _spiMaster->getTransferRequestBuffer(_slaveId);

  // Trigger a write of the first block of data to the drivers to zero them out
  request->bufferIn = (uint8_t*)_displayBufferIn;
  request->bufferOut = (uint8_t*)_displayBufferOut;
  request->length = cSpiBytesToSend;
  request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;
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
  SpiMaster::SpiTransferReq *request = _spiMaster->getTransferRequestBuffer(_slaveId);
  uint32_t pwmBits = 0;

  if (request != nullptr)
  {
    for (uint8_t d = 0; d < cPwmNumberOfDevices; d++)
    {
      for (uint8_t bit = 0; bit < cPwmChannelsPerDevice; bit++)
      {
        if (_pwmValues[bit + (d * cPwmChannelsPerDevice)] > _pwmTickCounter)
        {
          pwmBits |= (1 << bit);
        }
      }
      _displayBufferOut[d] = pwmBits;
      pwmBits = 0;
    }

    // Write the data to the drivers
    // request->bufferIn = (uint8_t*)_displayBufferIn;
    // request->bufferOut = (uint8_t*)_displayBufferOut;
    // request->length = cSpiBytesToSend;
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;

    // trigger the transfer if nothing is in progress
    if (_spiMaster->busy() == false)
    {
      _spiMaster->processQueue();
    }

    // increment the tick counter and reset it if it's time
    if (++_pwmTickCounter > NixieGlyph::cGlyphMaximumIntensity)
    {
      _pwmTickCounter = 0;
    }
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
