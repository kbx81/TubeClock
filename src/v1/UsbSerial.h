//
// kbx81's tube clock USB CDC-ACM serial interface
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


namespace kbxTubeClock {

namespace UsbSerial
{
  /// @brief Initialize the USB CDC-ACM interface (clocks, GPIO, USB device)
  ///
  void initialize();

  /// @brief Service pending USB events — call from usb_isr()
  ///
  void poll();

  /// @brief Send data to the USB host
  /// @param data Pointer to data bytes
  /// @param len  Number of bytes (must be <= 64; silently dropped if busy)
  ///
  void write(const uint8_t* data, uint16_t len);

  /// @brief True when the host has the port open (DTR asserted)
  ///
  bool isConnected();
}


}
