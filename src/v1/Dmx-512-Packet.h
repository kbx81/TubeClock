//
// kbx81's tube clock DMX-512 Packet class
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

class Dmx512Packet {

public: // Implement the Dmx512Packet class
  // DMX-512 buffer states
  //
  enum DmxBufferState : uint8_t {
    DmxBufferInvalid,
    DmxBufferBeingWritten,
    DmxBufferValid
  };

public:
  // Constructor
  //
  Dmx512Packet();

  // Returns the level of the specified DMX-512 channel
  //  Valid channels are 0 to 511
  uint8_t channel(const uint16_t channel) const;

  // Returns the start code of the DMX-512 packet
  //
  uint8_t startCode() const;

  // Returns a pointer to the DMX-512 packet
  //
  uint8_t* getBufferPtr();

  // Returns the state of the buffer
  //
  Dmx512Packet::DmxBufferState getBufferState() const;

  // Sets the state of the buffer
  //
  void setBufferState(DmxBufferState newState);

private:
  // DMX-512 data buffer
  //
  uint8_t _dmxPacket[513];
  DmxBufferState _bufferState;

};

}
