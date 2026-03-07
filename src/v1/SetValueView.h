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

#include "Settings.h"
#include "View.h"

namespace kbxTubeClock {

class SetValueView : public View {
 public:
  static const uint8_t cMaxGroupSize = 4;

  SetValueView();
  virtual void enter(const Settings::SettingDescriptor* descriptor,
                     uint8_t relatedSetting, uint8_t numSettings = 1) override;
  virtual bool keyHandler(Keys::Key key) override;
  virtual void loop() override;

 private:
  Settings::SettingDescriptor _descriptor;  // copy of descriptor for active setting
  uint16_t _value[cMaxGroupSize];           // raw setting values (as stored in Settings)
  uint8_t  _relatedSetting;
  uint8_t  _numSettings;
  int8_t   _selectedItem;                   // current item index; always 0 when _numSettings == 1
};

}  // namespace kbxTubeClock
