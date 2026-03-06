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

/// @brief kbx Tube Clock InfraredRemote
///

namespace kbxTubeClock::InfraredRemote {
// NEC IR code: full 16-bit address and command words
//
struct IrKeyCode {
  uint16_t address;
  uint16_t command;
};

// Logical IR key indices — identifies which key was pressed.
//  The order here determines the mapping to physical keys in Keys.cpp.
//
enum IrKey : uint8_t {
  A = 0,
  B = 1,
  C = 2,
  E = 3,
  D = 4,
  U = 5,
  Power = 6,
  Count = 7,
  None = 0xff,
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
IrKey getKeyPress();

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

}  // namespace kbxTubeClock::InfraredRemote
