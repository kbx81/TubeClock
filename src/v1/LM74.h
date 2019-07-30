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
#pragma once

#include <cstdint>
#include "Hardware.h"


namespace kbxTubeClock {

namespace LM74 {


// Check if the chip is connected
//
bool isConnected();

// Get the raw temperature register value
//
uint16_t getTemperatureRegister();

// Get the whole-number part of the temperature register (in degrees celsius)
//
int16_t getTemperatureWholePart();

// Get the fractional part of the temperature register (in degrees celsius)
//
uint16_t getTemperatureFractionalPart();

// Read temperature registers from the LM74
//  Returns result of read from Hardware::spiTransfer()
Hardware::HwReqAck refresh();

}

}
