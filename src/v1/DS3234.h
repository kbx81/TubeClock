//
// kbx81's tube clock DS3234 RTC class
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
#include "DateTime.h"
#include "Hardware.h"
#include "SpiMaster.h"


namespace kbxTubeClock {

namespace DS3234 {

// Library usage:
// 0. Call initialize() to trigger slave registration with SpiMaster.
// 1. Set base year if required. Default is 2000.
// 2. Call isConnected() to determine if IC is available for use.
// 3. Call isValid() to determine validity of RTC data.
// 4. Call setDateTime() if required to set date & time.
// 5. Call refresh() to refresh register states from IC.
// 6. Call getDateTime() as required, typically after calling refresh().

/// @brief Initialize the DS3234 module
///
void initialize();

/// @brief Check if the RTC is connected; returns true if so
/// @return true if DS3234 is connected
///
bool isConnected();

/// @brief Check if the RTC is running; returns true if so
/// @return true if DS3234 is running
///
bool isRunning();

/// @brief Check if the RTC is valid; returns true if so
/// @return true if DS3234 is time is valid
///
bool isValid();

/// @brief Get the current date/time
///
DateTime getDateTime();

/// @brief Get the raw temperature register value
/// @return temperature register's raw value as uint16_t
///
uint16_t getTemperatureRegister();

/// @brief Get the whole-number part of the temperature register (in degrees celsius)
/// @return the temperature register's whole-number part as a int16_t
///
int16_t getTemperatureWholePart();

/// @brief Get the fractional part of the temperature register (in degrees celsius)
/// @return the temperature register's fractional part as a uint16_t
///
uint16_t getTemperatureFractionalPart();

/// Configure the real time clock driver with a new base year
///
/// @param yearBase The year base which is used for the RTC.
///    The RTC stores the year only with two digits, plus one
///    additional bit for the next century. If you set the
///    year base to 2000, the RTC will hold the correct time
///    for 200 years, starting from 2000-01-01 00:00:00.
///
void setBaseYear(uint16_t yearBase = 2000);

/// @brief Set the date/time
/// @param dateTime The date and time to set in the RTC
///
void setDateTime(const DateTime &dateTime);

/// @brief Get a register's raw value
/// @return SpiMaster::SpiReqAck status indicating the state of the request
///
SpiMaster::SpiReqAck getRegister(const uint8_t registerAddress, uint8_t* const registerDataBuffer, const uint8_t numberOfBytes);

/// @brief Set a register's raw value
/// @return SpiMaster::SpiReqAck status indicating the state of the request
///
SpiMaster::SpiReqAck setRegister(const uint8_t registerAddress, uint8_t* const registerDataBuffer, const uint8_t numberOfBytes, const bool block);

/// @brief Read one or more bytes from the DS3234's SRAM
/// @return SpiMaster::SpiReqAck status indicating the state of the request
///
SpiMaster::SpiReqAck readSram(const uint8_t sramStartAddress, uint8_t* const data, const uint8_t numberOfBytes);

/// @brief Write one or more bytes to the DS3234's SRAM
/// @return SpiMaster::SpiReqAck status indicating the state of the request
///
SpiMaster::SpiReqAck writeSram(const uint8_t sramStartAddress, uint8_t* const data, const uint8_t numberOfBytes, const bool block);

/// @brief Read all registers from the DS3234
/// @param block wait for SPI cycle to complete if true
/// @return SpiMaster::SpiReqAck status indicating the state of the request
///
SpiMaster::SpiReqAck refresh(const bool block = false);

}

}
