//
// kbx81's tube clock InfraredRemote Library
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

/// @brief kbx Tube Clock InfraredRemote
///

namespace InfraredRemote
{
  // key scan codes
  //
  enum InfraredRemoteKey : uint8_t {
    None        = 0xff,
    VOLUMEDOWN  = 0x00,
    PLAYPAUSE   = 0x01,
    VOLUMEUP    = 0x02,
    SETUP       = 0x04,
    UP          = 0x05,
    STOPMODE    = 0x06,
    LEFT        = 0x08,
    ENTER       = 0x09,
    RIGHT       = 0x0a,
    ZERO        = 0x0c,
    DOWN        = 0x0d,
    BACK        = 0x0e,
    ONE         = 0x10,
    TWO         = 0x11,
    THREE       = 0x12,
    FOUR        = 0x14,
    FIVE        = 0x15,
    SIX         = 0x16,
    SEVEN       = 0x18,
    EIGHT       = 0x19,
    NINE        = 0x1a
  };


  // timer period - 48 gives us one microsecond ticks on the STM32
  //
  static const uint16_t cTimerPeriod = 48;


  /// @brief Initialize InfraredRemote
  ///
  void initialize();

  /// @brief Check if there is a key press in the queue
  /// @return True if a key press is available
  ///
  bool hasKeyPress();

  /// @brief Get the next key from the queue
  /// @return Key pressed
  ///
  InfraredRemoteKey getKeyPress();

  /// @brief Check if the key pressed is being held down
  /// @return True if the key pressed is being held
  ///
  bool keyIsHeld();

  /// @brief Call from GPIO interrupt
  ///
  void tick();

  /// @brief Call from Timer interrupt
  ///
  void overflow();


}


}
