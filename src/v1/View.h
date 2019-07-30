//
// kbx81's tube clock View abstract class
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

#include "Keys.h"
#include "Settings.h"


/// @brief View operating modes
///
enum ViewMode : uint8_t
{
  ViewMode0,
  ViewMode1,
  ViewMode2,
  ViewMode3,
  ViewMode4,
  ViewMode5,
  ViewMode6,
  ViewMode7
};


// This is the abstract base class for all views in this application
//
class View {

  public:
    /// The method which is called before this view is displayed
    ///
    virtual void enter();

    /// Called before loop for any pressed key
    ///
    virtual bool keyHandler(kbxTubeClock::Keys::Key key);

    /// The loop which is called periodically while the view is active
    ///
    virtual void loop();

    /// Destructor
    ///
    virtual ~View();

};
