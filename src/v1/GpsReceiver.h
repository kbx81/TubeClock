//
// kbx81's tube clock GpsReceiver Library
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


namespace kbxTubeClock {

/// @brief kbx Tube Clock GpsReceiver
///

namespace GpsReceiver
{
  /// @brief Check if the GPS module has sent anything meaningful at all
  /// @return True if connected
  ///
  bool isConnected();

  /// @brief Check if the GPS has received a valid date and time
  /// @return True if date & time is valid
  ///
  bool isValid();

  /// @brief Get the current date & time based on the GPS receiver
  /// @return DateTime object with current date & time
  ///
  DateTime getDateTime();

  /// @brief Get the adjusted date & time based on the GPS receiver & time zone
  /// @return DateTime object with local date & time
  ///
  DateTime getLocalDateTime();

  /// @brief Get the current number of satellites the GPS receiver sees
  /// @return Number of satellites in view as reported by the GPS receiver
  ///
  uint8_t getSatellitesInView();

  /// @brief Sets the offset used to compute the local time
  ///
  void setTimeZone(int16_t offsetInMinutes);

  /// @brief Call from USART recieve interrupt
  ///
  void rxIsr();


}


}
