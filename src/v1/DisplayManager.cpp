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

// The display workflow might be the most confusing aspect of this codebase.
// The following flow diagram should help clarify how it works:
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
// tickPWM() (called from tim2_isr at 15,360 Hz) ->
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

// Single PWM buffer - tick() writes directly, tickPWM() reads
// No double buffering needed as tick() writes complete values atomically
// Organized as [device][channel] for better cache locality
// Aligned to 4-byte boundary for optimal ARM memory access
//
static uint8_t _pwmValues[cPwmNumberOfDevices][cPwmChannelsPerDevice] __attribute__((aligned(4)));

// Master intensity lookup table (256 entries, uint8_t -> uint8_t)
// Maps input intensity (0-255) to output intensity (0-255) with master dimming applied
// Initialized by initialize() based on _masterIntensity setting
//
static uint8_t _intensityLUT[256];

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

// master display intensity (0-255)
static uint8_t _masterIntensity = 255;

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
// Applies master intensity via lookup table
//
void _setDisplayPwmValue(const uint8_t glyphNumber, const uint8_t glyphValue)
{
  if (glyphNumber < cGlyphCount)
  {
    uint8_t linearIndex = (cPwmNumberOfDevices * cPwmChannelsPerDevice) - 1 - glyphNumber;
    // Optimized division and modulo for power-of-2 (32 = 2^5)
    // Division by 32 = right shift by 5, modulo 32 = mask with 0x1F
    uint8_t device = linearIndex >> 5;   // Fast divide by 32 (cPwmChannelsPerDevice)
    uint8_t channel = linearIndex & 0x1F; // Fast modulo 32

    // Apply master intensity via lookup table and write to PWM buffer
    _pwmValues[device][channel] = _intensityLUT[glyphValue];
  }
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

  // Initialize master intensity to full brightness
  setMasterIntensity(255);

  // Initialize PWM buffer to zero
  for (uint8_t d = 0; d < cPwmNumberOfDevices; d++)
  {
    for (uint8_t c = 0; c < cPwmChannelsPerDevice; c++)
    {
      _pwmValues[d][c] = 0;
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
  // Update PWM buffer when tick() requests it
  // The crossfader tick() calls happen in ISR for precise timing,
  // but we read the results and update PWM here to minimize ISR time
  if (!_refreshIntensitiesNow) return;
  _refreshIntensitiesNow = false;

  // Read crossfader states and update PWM buffer with master intensity applied
  NixieGlyph activeGlyph;
  for (uint8_t glyphCrossfader = 0; glyphCrossfader < cGlyphCount; glyphCrossfader++)
  {
    activeGlyph = _crossfader[glyphCrossfader].getActive();
    _setDisplayPwmValue(glyphCrossfader, activeGlyph.getIntensity());
  }

  // Refresh the status LED to play nicely with strobing via DMX-512
  if (!_autoRefreshStatusLed) return;

  if (_displayBlank)
  {
    Hardware::setStatusLed(RgbLed());
  }
  else
  {
    RgbLed status = _statusLed;
    // Apply master intensity to status LED using LUT
    // Scale 12-bit RGB values (0-4095) to 8-bit (0-255) for LUT indexing
    // Then scale 8-bit LUT output (0-254) back to 12-bit (0-4095) for hardware
    status.setRed(((uint32_t)_intensityLUT[status.getRed() >> 4] * RgbLed::cLedMaxIntensity + 127) / 254);
    status.setGreen(((uint32_t)_intensityLUT[status.getGreen() >> 4] * RgbLed::cLedMaxIntensity + 127) / 254);
    status.setBlue(((uint32_t)_intensityLUT[status.getBlue() >> 4] * RgbLed::cLedMaxIntensity + 127) / 254);
    Hardware::setStatusLed(status);
  }
}


void tick()
{
  // Called from systick ISR at 1000 Hz
  // Tick all crossfaders here to maintain precise 1ms timing for fade durations
  // But defer the PWM value updates to refresh() to minimize ISR time
  for (uint8_t glyphCrossfader = 0; glyphCrossfader < cGlyphCount; glyphCrossfader++)
  {
    _crossfader[glyphCrossfader].tick();
  }

  // Set flag to request PWM buffer update in main loop
  _refreshIntensitiesNow = true;
}


void tickPWM()
{
  SpiMaster::SpiTransferReq *request = _spiMaster->getTransferRequestBuffer(_slaveId);

  if (request != nullptr)
  {
    // Read then increment the tick counter - uint8_t naturally wraps at 256 (no masking needed!)
    auto tickCount = _pwmTickCounter++;
    // Process all 3 devices with 8-bit unrolling to improve performance
    // Total of 12 iterations (3 devices × 4 bytes) instead of 96
    for (uint8_t device = 0; device < cPwmNumberOfDevices; device++)
    {
      // Fully unrolled 32-bit PWM comparison for maximum performance
      // PWM logic: output is ON when intensity > counter
      // This handles all cases correctly:
      //   intensity=0: always OFF (0 > counter is never true)
      //   intensity=1: ON when counter=0 (1/256 duty cycle)
      //   intensity=254: ON when counter=0-253 (254/256 = 99.2% duty cycle, perceptually full)
      // Note: LUT clamps output to max 254 to work with > comparison
      uint32_t pwmBits = 0;

      pwmBits |= ((uint32_t)(_pwmValues[device][0] > tickCount)) << 0;
      pwmBits |= ((uint32_t)(_pwmValues[device][1] > tickCount)) << 1;
      pwmBits |= ((uint32_t)(_pwmValues[device][2] > tickCount)) << 2;
      pwmBits |= ((uint32_t)(_pwmValues[device][3] > tickCount)) << 3;
      pwmBits |= ((uint32_t)(_pwmValues[device][4] > tickCount)) << 4;
      pwmBits |= ((uint32_t)(_pwmValues[device][5] > tickCount)) << 5;
      pwmBits |= ((uint32_t)(_pwmValues[device][6] > tickCount)) << 6;
      pwmBits |= ((uint32_t)(_pwmValues[device][7] > tickCount)) << 7;

      pwmBits |= ((uint32_t)(_pwmValues[device][8] > tickCount)) << 8;
      pwmBits |= ((uint32_t)(_pwmValues[device][9] > tickCount)) << 9;
      pwmBits |= ((uint32_t)(_pwmValues[device][10] > tickCount)) << 10;
      pwmBits |= ((uint32_t)(_pwmValues[device][11] > tickCount)) << 11;
      pwmBits |= ((uint32_t)(_pwmValues[device][12] > tickCount)) << 12;
      pwmBits |= ((uint32_t)(_pwmValues[device][13] > tickCount)) << 13;
      pwmBits |= ((uint32_t)(_pwmValues[device][14] > tickCount)) << 14;
      pwmBits |= ((uint32_t)(_pwmValues[device][15] > tickCount)) << 15;

      pwmBits |= ((uint32_t)(_pwmValues[device][16] > tickCount)) << 16;
      pwmBits |= ((uint32_t)(_pwmValues[device][17] > tickCount)) << 17;
      pwmBits |= ((uint32_t)(_pwmValues[device][18] > tickCount)) << 18;
      pwmBits |= ((uint32_t)(_pwmValues[device][19] > tickCount)) << 19;
      pwmBits |= ((uint32_t)(_pwmValues[device][20] > tickCount)) << 20;
      pwmBits |= ((uint32_t)(_pwmValues[device][21] > tickCount)) << 21;
      pwmBits |= ((uint32_t)(_pwmValues[device][22] > tickCount)) << 22;
      pwmBits |= ((uint32_t)(_pwmValues[device][23] > tickCount)) << 23;

      pwmBits |= ((uint32_t)(_pwmValues[device][24] > tickCount)) << 24;
      pwmBits |= ((uint32_t)(_pwmValues[device][25] > tickCount)) << 25;
      pwmBits |= ((uint32_t)(_pwmValues[device][26] > tickCount)) << 26;
      pwmBits |= ((uint32_t)(_pwmValues[device][27] > tickCount)) << 27;
      pwmBits |= ((uint32_t)(_pwmValues[device][28] > tickCount)) << 28;
      pwmBits |= ((uint32_t)(_pwmValues[device][29] > tickCount)) << 29;
      pwmBits |= ((uint32_t)(_pwmValues[device][30] > tickCount)) << 30;
      pwmBits |= ((uint32_t)(_pwmValues[device][31] > tickCount)) << 31;

      _displayBufferOut[device] = pwmBits;
    }

    // Queue the data for transfer to the drivers.
    // During normal operation, dmaComplete() chains each transfer to the next
    // via processQueue(). We only call processQueue() here as a bootstrap when
    // the SPI is completely idle (startup or after a period of no display activity).
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;

    if (_spiMaster->busy() == false)
    {
      _spiMaster->processQueue();
    }
  }
}


void setStatusLedAutoRefreshing(const bool autoRefreshEnabled)
{
  _autoRefreshStatusLed = autoRefreshEnabled;
}


uint8_t getMasterIntensity()
{
  return _masterIntensity;
}


void setMasterIntensity(const uint8_t intensity)
{
  _masterIntensity = intensity;

  // Build the master intensity lookup table
  // Maps input intensity (0-255) to output intensity (0-254) with master dimming applied
  // Note: Output is clamped to 254 max because PWM uses > comparison
  // With > comparison: value=254 gives 255/256 duty (99.6%), value=255 gives 255/256 duty
  // So we map everything to 0-254 range to avoid the ambiguity
  // Use rounding instead of truncation for better accuracy at low intensities
  for (uint16_t i = 0; i < 256; i++)
  {
    // Add 127 (half of 255) for rounding before division
    uint16_t scaled = ((uint32_t)i * _masterIntensity + 127) / 255;
    // Clamp to 254 maximum to work correctly with > comparison in PWM
    _intensityLUT[i] = (scaled >= 254) ? 254 : scaled;
  }
}


bool getDisplayBlanking()
{
  return _displayBlank;
}


void setDisplayBlanking(const bool blank)
{
  if (_displayBlank == blank) return;

  _displayBlank = blank;

  if (blank)
  {
    if (_autoRefreshStatusLed) Hardware::setStatusLed(RgbLed());
    Hardware::setDisplayHardwareBlanking(true);
  }
  else
  {
    if (_autoRefreshStatusLed) Hardware::setStatusLed(_statusLed);
    Hardware::setDisplayHardwareBlanking(false);
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
