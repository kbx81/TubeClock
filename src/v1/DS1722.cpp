//
// kbx81's tube clock DS1722 temperature sensor class
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
#include <libopencm3/stm32/spi.h>

#include <cstdint>

#include "DS1722.h"
#include "Hardware.h"
#include "SpiMaster.h"

#include "RgbLed.h"

#if HARDWARE_VERSION == 1
  #include "Hardware_v1.h"
#else
  #error HARDWARE_VERSION must be defined with a value of 1
#endif


namespace kbxTubeClock {

namespace DS1722 {


// The number of registers in the chip plus one (for the address byte we send)
//
static const uint8_t cNumberOfRegisters = 3 + 1;

// Registers of interest
//
static const uint8_t cConfigurationRegister  = 0x00;
static const uint8_t cTemperatureLSBRegister = 0x01;
static const uint8_t cTemperatureMSBRegister = 0x02;

static const uint8_t cAddressByte = 0x00;

// What we want in the config register
//
static const uint8_t cConfigByte = 0xee;

// Setting this bit in the address byte makes the operation a write
//
static const uint8_t cWriteBit = 0x80;

// SPI buffers
//
static uint8_t _spiWorkingBufferIn[cNumberOfRegisters];
static uint8_t _spiWorkingBufferOut[cNumberOfRegisters];

// A full copy of DS1722 registers, refreshed by calling refresh() function
// ...dirty black magic, but it works...
static uint8_t* _ds1722RegisterIn = _spiWorkingBufferIn + 1;
// static uint8_t* _ds1722RegisterOut = _spiWorkingBufferOut + 1;

// Pointer to SpiMaster, initialized by initialize()
//
static SpiMaster *_spiMaster = nullptr;

// Slave ID assigned by SpiMaster
//
static uint8_t _slaveId = SpiMaster::cNoSlave;


void initialize()
{
  SpiMaster::SpiSlave mySlave = {
    .gpioPort       = Hardware::cNssPort,           // gpio port on which CS line lives
    .gpioPin        = Hardware::cNssTemperaturePin, // gpio pin on which CS line lives
    .strobeCs       = false,                // CS line is strobed upon xfer completion if true
    .polarity       = true,                 // CS/CE polarity (true = active high)
    .misoPort       = Hardware::cSpi1Port,              // port on which slave inputs data
    .misoPin        = Hardware::cSpi1MisoPin,           // pin on which slave inputs data
    .br             = SPI_CR1_BAUDRATE_FPCLK_DIV_16,    // Baudrate
    .cpol           = SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,  // Clock polarity
    .cpha           = SPI_CR1_CPHA_CLK_TRANSITION_2,    // Clock Phase
    .lsbFirst       = SPI_CR1_MSBFIRST,     // Frame format -- lsb/msb first
    .dataSize       = SPI_CR2_DS_8BIT,      // Data size (4 to 16 bits, see RM)
    .memorySize     = DMA_CCR_MSIZE_8BIT,   // Memory word width (8, 16, 32 bit)
    .peripheralSize = DMA_CCR_PSIZE_8BIT    // Peripheral word width (8, 16, 32 bit)
  };

  _spiMaster = Hardware::getSpiMaster();

  _slaveId = _spiMaster->registerSlave(&mySlave);
}


void flash(const SpiMaster::SpiReqAck state)
{
  switch (state)
  {
    case SpiMaster::SpiReqAck::SpiReqAckQueued:
    Hardware::setStatusLed(RgbLed(0, 0, 1024));
    break;

    case SpiMaster::SpiReqAck::SpiReqAckError:
    Hardware::setStatusLed(RgbLed(1024, 0, 0));
    break;

    case SpiMaster::SpiReqAck::SpiReqAckBusy:
    Hardware::setStatusLed(RgbLed(0, 1024, 0));
    break;

    default:
    Hardware::setStatusLed(RgbLed(1024, 250, 250));
  }
  Hardware::delay(250);
  // while (1);
}


bool isConnected()
{
  SpiMaster::SpiTransferReq* volatile request = _spiMaster->getTransferRequestBuffer(_slaveId);

  if (request != nullptr)
  {
    // Start writing at the Configuration register
    _spiWorkingBufferOut[cAddressByte] = cConfigurationRegister | cWriteBit;
    // Set the config register to the value we want to use
    _spiWorkingBufferOut[cAddressByte + 1] = cConfigByte;

    request->bufferIn = _spiWorkingBufferIn;
    request->bufferOut = _spiWorkingBufferOut;
    request->length = 2;
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;
    // We must wait for the transfer to complete before we touch any buffers again
    while ((request->state != SpiMaster::SpiReqAck::SpiReqAckOk) || (_spiMaster->busy() == true));
    // flash(request->state);

    // This is the address we want to start reading from
    _spiWorkingBufferOut[cAddressByte] = cConfigurationRegister;
    // Try to read the byte (and other registers) back
    request->length = cNumberOfRegisters;
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;
    // We must wait for the transfer to complete before we touch any buffers again
    while ((request->state != SpiMaster::SpiReqAck::SpiReqAckOk) || (_spiMaster->busy() == true));

    // If we read back the byte we wrote, the IC is very likely connected
    if (_ds1722RegisterIn[cConfigurationRegister] == cConfigByte)
    {
      // Kick off a full register refresh
      while (refresh() != SpiMaster::SpiReqAck::SpiReqAckOk);

      return true;
    }
  }
  return false;
}


uint16_t getTemperatureRegister()
{
  return (_ds1722RegisterIn[cTemperatureMSBRegister] << 8) | _ds1722RegisterIn[cTemperatureLSBRegister];
}


int16_t getTemperatureWholePart()
{
  return ((int8_t)_ds1722RegisterIn[cTemperatureMSBRegister]);
}


uint16_t getTemperatureFractionalPart()
{
  return (_ds1722RegisterIn[cTemperatureLSBRegister] >> 4);
}


SpiMaster::SpiReqAck refresh(const bool block)
{
  SpiMaster::SpiTransferReq* request = _spiMaster->getTransferRequestBuffer(_slaveId);

  if (request != nullptr)
  {
    // This is the address we want to start reading from
    _spiWorkingBufferOut[cAddressByte] = cConfigurationRegister;
    // Try to read the byte (and other registers) back
    // return Hardware::spiTransferRequest(&_request);
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;

    while (((request->state != SpiMaster::SpiReqAck::SpiReqAckOk) || (_spiMaster->busy() == true)) && (block == true));

    return SpiMaster::SpiReqAck::SpiReqAckOk;
  }
  return SpiMaster::SpiReqAck::SpiReqAckError;
}


}

}
