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
  enum SelectedItem : int8_t {
    SecondDay = 0,
    MinuteMonth = 1,
    HourYear = 2,
  };

  /// @brief Scale factor applied to non-selected digits and the status LED (see RgbLed::pct / NixieTube::pct)
  ///
  static const uint16_t cLowlightPercentage;

  /// @brief The view which displays the UI for setting times and dates
  ///
 public:  // Implement the SetTimeDateView class
  SetTimeDateView();
  virtual void enter(const Settings::SettingDescriptor* descriptor,
                     uint8_t relatedSetting, uint8_t numSettings = 1) override;
  virtual bool keyHandler(Keys::Key key) override;
  virtual void loop() override;

 private:
  /// @brief the selected byte
  ///
  int8_t _selectedItem;

  /// @brief the DateTime object we'll adjust
  ///
  DateTime _workingDateTime;

  /// @brief the main application's mode (drives clock/date/slot behavioral dispatch)
  ///
  Application::OperatingMode _mode;

  /// @brief the setting index passed at enter() time (used for slot save operations)
  ///
  uint8_t _relatedSetting;
};

}  // namespace kbxTubeClock
