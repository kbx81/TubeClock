//
// kbx81's tube clock set time/date view class
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

class SetTimeDateView : public View {

public:
  /// @brief The selected item on the display
  ///
  enum SelectedItem : int8_t
  {
    SecondDay = 0,
    MinuteMonth = 1,
    HourYear = 2
  };

  /// @brief Percentage of configured LED color intensities used for lowlight
  ///
  static const uint16_t cLowlightPercentage;

/// @brief The view which displays the UI for setting times and dates
///
public: // Implement the SetTimeDateView class
  SetTimeDateView();
  virtual void enter() override;
  virtual bool keyHandler(Keys::Key key) override;
  virtual void loop() override;

private:
  /// @brief the selected byte
  ///
  int8_t _selectedItem;

  /// @brief the DateTime object we'll adjust
  ///
  DateTime _workingDateTime;

  /// @brief the main application's mode
  ///
  Application::OperatingMode _mode;

  /// @brief Settings to be used by the view
  ///
  Settings _settings;

};

}
