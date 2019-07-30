//
// kbx81's tube clock SPI class
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
#include "Hardware.h"
#include "SPI.h"


namespace kbxTubeClock {

  // some constant
  const uint32_t Spi::cSpiConstant = 0;


  Spi::Spi()
  : _variable(0)
  {
    initialize();
  }


  void Spi::initialize()
  {
  }


}
