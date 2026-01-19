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
// writeDisplay()/writeStatusLed() (main loop) ->
//   _loadCrossfaders() (loads new target values into crossfaders)
//
// tick() (called from sys_tick_handler at 1000 Hz) ->
//   crossfader tick() updates fade state ->
//   atomic buffer swap (_activeBuffer flip) ->
//   sets flag for refresh()
//
// refresh() (main loop) ->
//   reads crossfader states, adjusts intensity ->
//   _setDisplayPwmValue() writes to INACTIVE _pwmValues buffer
//
// tickPWM() (called from tim2_isr at 12,800 Hz) ->
//   reads from ACTIVE _pwmValues buffer and generates software PWM ->
//     _displayBufferOut (HV562x shift register data) ->
//       SPI DMA to HV562x shift registers & latches
//
// Double buffering eliminates data races: ISR reads from one buffer while
// main loop writes to the other, then tick() atomically swaps them.

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

// SoftPWM tick counter - uint8_t allows natural rollover at 256
//
static uint8_t _pwmTickCounter = 0;

// Double-buffered SoftPWM values for race-free updates
// Buffer 0 and 1 swap roles: one is actively read by ISR, other is written by main loop
// Organized as [buffer][device][channel] for better cache locality
// Aligned to 4-byte boundary for optimal ARM memory access
//
static uint16_t _pwmValues[2][cPwmNumberOfDevices][cPwmChannelsPerDevice] __attribute__((aligned(4)));

// Active buffer index - read by tickPWM() ISR, swapped atomically by tick() ISR
// 0 or 1 indicating which buffer tickPWM() should read from
//
volatile static uint8_t _activeBuffer = 0;

// HV562x buffers - aligned for DMA transfers
//
static uint32_t _displayBufferIn[cPwmNumberOfDevices] __attribute__((aligned(4)));
static uint32_t _displayBufferOut[cPwmNumberOfDevices] __attribute__((aligned(4)));

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


// Writes to PWM buffer corresponding to a glyph
// Writes to the INACTIVE buffer (the one not being read by tickPWM ISR)
//
void _setDisplayPwmValue(const uint8_t glyphNumber, const uint16_t glyphValue)
{
  if (glyphNumber < cGlyphCount)
  {
    uint8_t linearIndex = (cPwmNumberOfDevices * cPwmChannelsPerDevice) - 1 - glyphNumber;
    // Optimized division and modulo for power-of-2 (32 = 2^5)
    // Division by 32 = right shift by 5, modulo 32 = mask with 0x1F
    uint8_t device = linearIndex >> 5;   // Fast divide by 32 (cPwmChannelsPerDevice)
    uint8_t channel = linearIndex & 0x1F; // Fast modulo 32

    // Write to inactive buffer (opposite of what tickPWM is reading)
    uint8_t inactiveBuffer = 1 - _activeBuffer;
    _pwmValues[inactiveBuffer][device][channel] = glyphValue;
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

  // Initialize both PWM buffers to zero
  for (uint8_t b = 0; b < 2; b++)
  {
    for (uint8_t d = 0; d < cPwmNumberOfDevices; d++)
    {
      for (uint8_t c = 0; c < cPwmChannelsPerDevice; c++)
      {
        _pwmValues[b][d][c] = 0;
      }
    }
  }

  // Initialize display output buffers
  for (uint8_t d = 0; d < cPwmNumberOfDevices; d++)
  {
    _displayBufferOut[d] = 0;
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

    // Read crossfader states (already ticked in ISR), adjust intensities, write to PWM buffer
    for (uint8_t glyphCrossfader = 0; glyphCrossfader < cGlyphCount; glyphCrossfader++)
    {
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
  // Tick all crossfaders here in the ISR for precise 1000 Hz timing
  // This ensures fade duration values accurately represent milliseconds
  for (uint8_t glyphCrossfader = 0; glyphCrossfader < cGlyphCount; glyphCrossfader++)
  {
    _crossfader[glyphCrossfader].tick();
  }

  // Atomically swap buffers - main loop's writes are now visible to tickPWM() ISR
  // This single-byte write is atomic on ARM Cortex-M, ensuring race-free buffer swap
  _activeBuffer = 1 - _activeBuffer;

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

  if (request != nullptr)
  {
    // Read from active buffer (set by tick() ISR via atomic buffer swap)
    uint8_t activeBuffer = _activeBuffer;

    // Process all 3 devices with 8-bit unrolling to improve performance
    // Total of 12 iterations (3 devices × 4 bytes) instead of 96
    for (uint8_t device = 0; device < cPwmNumberOfDevices; device++)
    {
      uint32_t pwmBits = 0;

      // Process 8 bits at a time (4 iterations per device: 32 bits / 8 = 4)
      for (uint8_t byteIdx = 0; byteIdx < 4; byteIdx++)
      {
        uint8_t offset = byteIdx << 3;  // byteIdx * 8

        // Unroll 8 bits manually for branchless performance
        pwmBits |= ((uint32_t)(_pwmValues[activeBuffer][device][offset + 0] > _pwmTickCounter)) << (offset + 0);
        pwmBits |= ((uint32_t)(_pwmValues[activeBuffer][device][offset + 1] > _pwmTickCounter)) << (offset + 1);
        pwmBits |= ((uint32_t)(_pwmValues[activeBuffer][device][offset + 2] > _pwmTickCounter)) << (offset + 2);
        pwmBits |= ((uint32_t)(_pwmValues[activeBuffer][device][offset + 3] > _pwmTickCounter)) << (offset + 3);
        pwmBits |= ((uint32_t)(_pwmValues[activeBuffer][device][offset + 4] > _pwmTickCounter)) << (offset + 4);
        pwmBits |= ((uint32_t)(_pwmValues[activeBuffer][device][offset + 5] > _pwmTickCounter)) << (offset + 5);
        pwmBits |= ((uint32_t)(_pwmValues[activeBuffer][device][offset + 6] > _pwmTickCounter)) << (offset + 6);
        pwmBits |= ((uint32_t)(_pwmValues[activeBuffer][device][offset + 7] > _pwmTickCounter)) << (offset + 7);
      }

      _displayBufferOut[device] = pwmBits;
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

    // increment the tick counter - uint8_t naturally wraps at 256 (no masking needed!)
    _pwmTickCounter++;
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
