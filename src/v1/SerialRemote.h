//
// kbx81's tube clock serial remote control interface
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

namespace SerialRemote
{
  /// @brief Initialize the serial remote control interface
  ///
  void initialize();

  /// @brief Called from USART RX interrupt to process incoming bytes
  /// @param usart The USART peripheral base address that triggered the interrupt
  ///
  void rxIsr(uint32_t usart);

  /// @brief Called from USART TX interrupt to send next byte
  /// @param usart The USART peripheral base address that triggered the interrupt
  ///
  void txIsr(uint32_t usart);

  /// @brief Check if a complete command has been received and is ready to process
  /// @return true if a command is ready
  ///
  bool hasCommand();

  /// @brief Process a pending command (called from main loop)
  ///
  void process();

  /// @brief Notify the remote of a display mode change
  /// @param mode The new operating mode
  /// @param viewMode The new view sub-mode
  ///
  void notifyModeChange(uint8_t mode, uint8_t viewMode);

  /// @brief Notify the remote of a key event
  /// @param keyMask Bitmask of the key
  /// @param pressed true if pressed, false if released
  ///
  void notifyKeyEvent(uint8_t keyMask, bool pressed);
}


}
