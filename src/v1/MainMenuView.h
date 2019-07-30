//
// kbx81's tube clock MainMenuView class
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

#include "Settings.h"
#include "View.h"


namespace kbxTubeClock {

class MainMenuView : public View {

// The view which displays either the date, time, temperature, or a rotation
//
public: // Implement the MainMainView class
  MainMenuView();
  virtual void enter() override;
  virtual bool keyHandler(Keys::Key key) override;
  virtual void loop() override;

private:
  // the currently selected & displayed mode
  //
  uint8_t _selectedMode;

  // the mode that was last selected
  //
  uint8_t _previousMode;

};

}
