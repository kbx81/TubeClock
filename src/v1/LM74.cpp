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
#include <cstdint>
#include "LM74.h"
#include "Hardware.h"


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
static uint8_t lm74Register[cNumberOfRegisters];

// A spare buffer for when we don't need to read/write data
//
static uint8_t spareBuffer[cNumberOfRegisters];

// SPI transfer requests
//
static Hardware::SpiTransferReq _request = {
    peripheral : Hardware::SpiPeripheral::TempSensor,
    bufferIn : lm74Register,
    bufferOut : spareBuffer,
    length : cNumberOfRegisters,
    use16BitXfers : false,
    state : Hardware::HwReqAck::HwReqAckError
};


bool isConnected()
{
  // Try to read the temperature registers
  while (Hardware::spiTransferRequest(&_request) != Hardware::HwReqAck::HwReqAckOk);
  while (_request.state != Hardware::HwReqAck::HwReqAckOk);

  // If we read back some possible expected values, we'll go with it
  if (((lm74Register[0] == 0xff) && ((lm74Register[1] & 0xfc) == 0)) ||
      ((lm74Register[1] & 0x04) == 0x04))
  {
    // Kick off a full register refresh
    while (refresh() != Hardware::HwReqAck::HwReqAckOk);

    return true;
  }

  return false;
}


uint16_t getTemperatureRegister()
{
  return (lm74Register[0] << 8) | lm74Register[1];
}


int16_t getTemperatureWholePart()
{
  int16_t temperature = ((lm74Register[0] & 0x80) << 8) |
                        (lm74Register[0] << 1) |
                        (lm74Register[1] >> 7);
  return temperature;
}


uint16_t getTemperatureFractionalPart()
{
  uint16_t temperature = (lm74Register[1] & 0x7f) >> 3;

  return temperature;
}


Hardware::HwReqAck refresh()
{
  // Try to read the temperature registers
  return Hardware::spiTransferRequest(&_request);
}


}

}
