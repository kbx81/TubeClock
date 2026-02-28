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

// Temperature offset in tenths of degrees Celsius
//
static int16_t _temperatureOffset = 0;

// Cached connection state, set by checkConnection()
//
static bool _connected = false;


void initialize()
{
  SpiMaster::SpiSlave mySlave = {
    .gpioPort       = Hardware::cNssPort,               // gpio port on which CS line lives
    .misoPort       = Hardware::cSpi1Port,              // port on which slave inputs data
    .br             = SPI_CR1_BAUDRATE_FPCLK_DIV_16,    // Baudrate
    .dataSize       = SPI_CR2_DS_8BIT,                  // Data size (4 to 16 bits, see RM)
    .memorySize     = DMA_CCR_MSIZE_8BIT,               // Memory word width (8, 16, 32 bit)
    .peripheralSize = DMA_CCR_PSIZE_8BIT,               // Peripheral word width (8, 16, 32 bit)
    .gpioPin        = Hardware::cNssTemperaturePin,     // gpio pin on which CS line lives
    .misoPin        = Hardware::cSpi1MisoPin,           // pin on which slave inputs data
    .strobeCs       = false,                            // CS line is strobed upon xfer completion if true
    .polarity       = false,                            // CS/CE polarity (true = active high)
    .cpol           = true,                             // Clock polarity (idle high)
    .cpha           = true,                             // Clock phase (transition 2)
    .lsbFirst       = false,                            // MSB first
  };

  _spiMaster = Hardware::getSpiMaster();

  _slaveId = _spiMaster->registerSlave(&mySlave);
}


bool checkConnection()
{
  SpiMaster::SpiTransferReq* request = _spiMaster->getTransferRequestBuffer(_slaveId);

  if (request != nullptr)
  {
    // First read
    request->bufferIn = _lm74Register;
    request->bufferOut = _spareBuffer;
    request->length = cNumberOfRegisters;
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;
    while (_spiMaster->transferComplete(_slaveId) == false);

    uint8_t read1_msb = _lm74Register[0];
    uint8_t read1_lsb = _lm74Register[1];

    // Second read
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;
    while (_spiMaster->transferComplete(_slaveId) == false);

    uint8_t read2_msb = _lm74Register[0];
    uint8_t read2_lsb = _lm74Register[1];

    // POR case: both reads must show MSB=0xFF, LSB bits[7:2]=0x00
    bool porMatch = (read1_msb == 0xff) && ((read1_lsb & 0xfc) == 0)
                 && (read2_msb == 0xff) && ((read2_lsb & 0xfc) == 0);

    // Post-conversion case: D2 (conversion complete) set in both reads,
    //  MSB != 0xFF (reject floating-high / ambiguous POR overlap),
    //  and temperature values match within +/-2 LSBs
    bool postConv = false;
    if (((read1_lsb & 0x04) == 0x04) && ((read2_lsb & 0x04) == 0x04)
        && (read1_msb != 0xff) && (read2_msb != 0xff))
    {
      // Compare temperature using D15:D3 (mask off D2:D0)
      uint16_t raw1 = (read1_msb << 8) | (read1_lsb & 0xf8);
      uint16_t raw2 = (read2_msb << 8) | (read2_lsb & 0xf8);
      uint16_t diff = (raw1 > raw2) ? (raw1 - raw2) : (raw2 - raw1);
      // Allow +/-2 temperature LSBs (each LSB is at bit 3, so 2 LSBs = 16)
      postConv = (diff <= 16);
    }

    if (porMatch || postConv)
    {
      // Kick off a full register refresh
      while (refresh(true) != SpiMaster::SpiReqAck::SpiReqAckOk);

      _connected = true;
      return true;
    }
  }
  return false;
}


bool isConnected()
{
  return _connected;
}


uint16_t getTemperatureRegister()
{
  return (_lm74Register[0] << 8) | _lm74Register[1];
}


int32_t getTemperatureCx10()
{
  int16_t temp_whole = ((_lm74Register[0] & 0x80) << 8) |
                        (_lm74Register[0] << 1) |
                        (_lm74Register[1] >> 7);
  uint16_t temp_frac = (_lm74Register[1] & 0x7f) >> 3;
  // (whole * 1000 + (frac >> 1) * 125) / 100 = (whole * 8 + (frac >> 1)) * 5 / 4
  // Replaces expensive / 100 (software divide) with / 4 (shift)
  int32_t v = (temp_whole * 8 + (temp_frac >> 1)) * 5;
  return v / 4 + _temperatureOffset;
}


void setTemperatureOffset(int16_t offset)
{
  _temperatureOffset = offset;
}


SpiMaster::SpiReqAck refresh(const bool block)
{
  SpiMaster::SpiTransferReq* request = _spiMaster->getTransferRequestBuffer(_slaveId);

  if (request != nullptr)
  {
    // Trigger a read of the temperature registers
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;

    while ((_spiMaster->transferComplete(_slaveId) == false) && (block == true));

    return SpiMaster::SpiReqAck::SpiReqAckOk;
  }
  // Try to read the temperature registers
  return SpiMaster::SpiReqAck::SpiReqAckError;
}


bool transferComplete()
{
  return _spiMaster->transferComplete(_slaveId);
}


}

}
