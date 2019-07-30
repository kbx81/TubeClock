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

#include "Dmx-512-Packet.h"


namespace kbxTubeClock {


Dmx512Packet::Dmx512Packet()
: _bufferState(DmxBufferState::DmxBufferInvalid)
{
}


uint8_t Dmx512Packet::channel(const uint16_t channel) const
{
  if (channel < 512)
  {
    return _dmxPacket[channel + 1];
  }
  return _dmxPacket[1];
}


uint8_t Dmx512Packet::startCode() const
{
  return _dmxPacket[0];
}


uint8_t* Dmx512Packet::getBufferPtr()
{
  return _dmxPacket;
}


Dmx512Packet::DmxBufferState Dmx512Packet::getBufferState() const
{
  return _bufferState;
}


void Dmx512Packet::setBufferState(DmxBufferState newState)
{
  _bufferState = newState;
}


}
