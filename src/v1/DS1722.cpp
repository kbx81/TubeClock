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
#include <cstdint>
#include "DS1722.h"
#include "Hardware.h"


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

// SPI transfer requests
//
static Hardware::SpiTransferReq _request = {
    peripheral : Hardware::SpiPeripheral::TempSensor,
    bufferIn : _spiWorkingBufferIn,
    bufferOut : _spiWorkingBufferOut,
    length : cNumberOfRegisters,
    use16BitXfers : false,
    state : Hardware::HwReqAck::HwReqAckError
};


bool isConnected()
{
  // Start writing at the Configuration register
  _spiWorkingBufferOut[cAddressByte] = cConfigurationRegister | cWriteBit;
  // Set the config register to the value we want to use
  _spiWorkingBufferOut[cAddressByte + 1] = cConfigByte;

  _request.length = 2;
  // Write to the regsiter
  while (Hardware::spiTransferRequest(&_request) != Hardware::HwReqAck::HwReqAckOk);
  while (_request.state != Hardware::HwReqAck::HwReqAckOk);

  _request.length = cNumberOfRegisters;
  // This is the address we want to start reading from
  _spiWorkingBufferOut[cAddressByte] = cConfigurationRegister;
  // Try to read the byte (and other registers) back
  while (Hardware::spiTransferRequest(&_request) != Hardware::HwReqAck::HwReqAckOk);
  while (_request.state != Hardware::HwReqAck::HwReqAckOk);

  // If we read back the byte we wrote, the IC is very likely connected
  if (_ds1722RegisterIn[cConfigurationRegister] == cConfigByte)
  {
    // Kick off a full register refresh
    while (refresh() != Hardware::HwReqAck::HwReqAckOk);

    return true;
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


Hardware::HwReqAck refresh()
{
  // This is the address we want to start reading from
  _spiWorkingBufferOut[cAddressByte] = cConfigurationRegister;
  // Try to read the byte (and other registers) back
  return Hardware::spiTransferRequest(&_request);
}


}

}
