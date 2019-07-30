//
// kbx81's tube clock system status view class
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
#include "DateTime.h"


namespace kbxTubeClock {

class SystemStatusView : public View {

  enum DisplayItem : uint8_t
  {
    TubeLifetime      = ViewMode::ViewMode0,
    PeripheralStatus  = ViewMode::ViewMode1,
    GpsStatus         = ViewMode::ViewMode2,
    VoltageVddA       = ViewMode::ViewMode3,
    VoltageBattery    = ViewMode::ViewMode4
  };


/// @brief The view which displays the UI for setting times and dates
///
public: // Implement the SystemStatusView class
  SystemStatusView();
  virtual void enter() override;
  virtual bool keyHandler(Keys::Key key) override;
  virtual void loop() override;

private:
  /// @brief the selected byte
  ///
  int8_t _selectedView;

};

}
