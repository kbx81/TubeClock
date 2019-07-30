//
// kbx81's tube clock keys class
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

// This class provides access to the touch keys on the device.
//
// This implements a simple key handling mechanism for single key
// presses. If two keys are pressed simultaneously, always the key with the
// higher priority (lower scan code) is used.
//
// Pressed keys are added to a queue, so the code can get all key presses
// even after a short delay.
//
// There is a repeat function enabled, so after ~1s, the pressed key
// is automatically repeated to allow simple interfaces to adjust time/date
// and similar things.
//
#pragma once

#include <cstdint>
#include <string.h>
#include "Hardware.h"


namespace kbxTubeClock {

namespace Keys {


// key scan codes
//
enum Key : uint8_t {
  None = 0,
  A = (1 << 5),
  B = (1 << 4),
  C = (1 << 3),
  E = (1 << 2),
  D = (1 << 1),
  U = (1 << 0)
};


// Check the keys
//
void scanKeys();

// Should be called at consistent intervals to insert key repeats
//
void repeatHandler();

// Check if there is a key press in the queue
//
bool hasKeyPress();

// Get the next key from the queue
//
Key getKeyPress();


}

}
