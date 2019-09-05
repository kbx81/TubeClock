//
// kbx81's tube clock LM74 temperature sensor class
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

#include "LM74.h"
#include "Hardware.h"
#include "SpiMaster.h"

#if HARDWARE_VERSION == 1
  #include "Hardware_v1.h"
#else
  #error HARDWARE_VERSION must be defined with a value of 1
#endif


namespace kbxTubeClock {

namespace LM74 {


// The number of 8-bit registers in the chip
//
// static const uint8_t cNumberOfRegisters = 6;
static const uint8_t cNumberOfRegisters = 2;  // ...but we only use two

// Registers of interest
//
// static const uint8_t cConfigurationRegister = 0x00;
// static const uint8_t cTemperatureRegister = 0x02;
// static const uint8_t cDeviceIdRegister = 0x04;

// A copy of the LM74 registers, refreshed by calling refresh() functions
//
static uint8_t _lm74Register[cNumberOfRegisters];

// A spare buffer for when we don't need to read/write data
//
static uint8_t _spareBuffer[cNumberOfRegisters];

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
    .polarity       = false,                // CS/CE polarity (true = active high)
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


bool isConnected()
{
  volatile SpiMaster::SpiTransferReq* request = _spiMaster->getTransferRequestBuffer(_slaveId);

  if (request != nullptr)
  {
    request->bufferIn = _lm74Register;
    request->bufferOut = _spareBuffer;
    request->length = cNumberOfRegisters;
    // Try to read the temperature registers
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;
    // We must wait for the transfer to complete before we touch any buffers again
    while ((request->state != SpiMaster::SpiReqAck::SpiReqAckOk) || (_spiMaster->busy() == true));

    // If we read back some possible expected values, we'll go with it
    if (((_lm74Register[0] == 0xff) && ((_lm74Register[1] & 0xfc) == 0))
        || ((_lm74Register[1] & 0x04) == 0x04))
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
  return (_lm74Register[0] << 8) | _lm74Register[1];
}


int16_t getTemperatureWholePart()
{
  int16_t temperature = ((_lm74Register[0] & 0x80) << 8) |
                        (_lm74Register[0] << 1) |
                        (_lm74Register[1] >> 7);
  return temperature;
}


uint16_t getTemperatureFractionalPart()
{
  uint16_t temperature = (_lm74Register[1] & 0x7f) >> 3;

  return temperature;
}


SpiMaster::SpiReqAck refresh(const bool block)
{
  SpiMaster::SpiTransferReq* request = _spiMaster->getTransferRequestBuffer(_slaveId);

  if (request != nullptr)
  {
    // Trigger a read of the temperature registers
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;

    while (((request->state != SpiMaster::SpiReqAck::SpiReqAckOk) || (_spiMaster->busy() == true)) && (block == true));

    return SpiMaster::SpiReqAck::SpiReqAckOk;
  }
  // Try to read the temperature registers
  // return Hardware::spiTransferRequest(&_request);
  return SpiMaster::SpiReqAck::SpiReqAckError;
}


}

}
