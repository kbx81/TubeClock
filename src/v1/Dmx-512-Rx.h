//
// kbx81's tube clock DMX-512 receiver class
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
#include "Dmx-512-Packet.h"


namespace kbxTubeClock {

namespace Dmx512Rx {


// Initialize it all
//
void initialize();

// Returns true if a DMX-512 signal is being received
//
bool signalIsActive();

// Returns a pointer to the last DMX-512 packet received
//
Dmx512Packet* getLastPacket();

// Interrupt service routines
//
void rxIsr();
void rxCompleteIsr();
void timerUartIsr();
void timerSupervisorIsr();

}

}
