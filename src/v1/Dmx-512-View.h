//
// kbx81's tube clock DMX-512 View class
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

#include "Application.h"
#include "Settings.h"
#include "View.h"


namespace kbxTubeClock {

class Dmx512View : public View {

// The view which displays either the date, time, temperature, or a rotation
//
public: // Implement the Dmx512View class
  Dmx512View();
  virtual void enter() override;
  virtual bool keyHandler(Keys::Key key) override;
  virtual void loop() override;

private:
  // Channel multiplier - used to increase range of intensities and strobe rates
  //
  static const uint8_t cChannelMultiplier = 4;

  // the main application's mode
  //
  Application::OperatingMode _mode;

  // views for each DMX-512 mode
  //
  void _dmxExtendedModeView(Dmx512Packet* packet, uint16_t address);
  void _dmxStandardModeView(Dmx512Packet* packet, uint16_t address);
};

}
