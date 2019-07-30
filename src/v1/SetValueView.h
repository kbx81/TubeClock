//
// kbx81's tube clock set an arbitrary value view class
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

class SetValueView : public View {

// The view which displays the UI for setting times and dates
//
public: // Implement the SetValue class
  SetValueView();
  virtual void enter() override;
  virtual bool keyHandler(Keys::Key key) override;
  virtual void loop() override;

private:
  // values the user will set
  //
  uint16_t _setValue;

  // maximum allowed values for each field
  //
  uint16_t _maxValue;

  // the main application's mode
  //
  Application::OperatingMode _mode;

  // Settings to be used by the view
  //
  Settings _settings;

};

}
