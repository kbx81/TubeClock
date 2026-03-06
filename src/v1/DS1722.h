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
#pragma once

#include <cstdint>
#include "Hardware.h"
#include "SpiMaster.h"

namespace kbxTubeClock::DS1722 {

/// @brief Initialize the DS1722 module
///
void initialize();

/// @brief Probe the chip via SPI; returns true if connected (caches result for isConnected())
///
bool checkConnection();

/// @brief Returns cached connection state set by checkConnection()
///
bool isConnected();

/// @brief Get the raw temperature register value
///
uint16_t getTemperatureRegister();

/// @brief Get the temperature in tenths of degrees Celsius (Cx10), with offset applied
///
int32_t getTemperatureCx10();

/// @brief Set the desired temperature offset in tenths of degrees Celsius (e.g. 25 = +2.5C, -30 = -3.0C)
///
void setTemperatureOffset(int16_t offset);

/// @brief Read all registers from the DS1722
///
SpiMaster::SpiReqAck refresh(const bool block = false);

/// @brief Returns true if the last refresh() SPI transfer has completed
/// @return true if transfer is complete and temperature data will be fresh
///
bool transferComplete();

}  // namespace kbxTubeClock::DS1722
