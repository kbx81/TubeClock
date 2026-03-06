//
// kbx81's Nixie Glyph Crossfader Library
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

#include "NixieGlyph.h"

namespace kbxTubeClock {

/// @brief kbx Tube Clock NixieGlyphCrossfader class
///

class NixieGlyphCrossfader {
 public:
  NixieGlyphCrossfader() : _active(), _start(), _target() {}

  /// @brief Gets the fader's active/current value
  ///
  /// @return fader's current values with duration set to number of ticks elapsed
  ///
  NixieGlyph getActive() const;

  /// @brief Starts a new fade with (a) new target value(s)
  /// No-op if the target intensity is unchanged, EXCEPT: if duration=0 is
  /// requested and a fade is still in progress, the fader snaps to the current
  /// target immediately so a mode switch can cleanly interrupt a running fade.
  ///
  /// @param newTarget New target values for fader, including duration
  ///
  void startNewFade(const NixieGlyph &newTarget);

  /// @brief Advances all crossfades currently in progress (call at fixed intervals)
  ///
  void tick();

 private:
  NixieGlyph _active;  ///< current values of fader, duration value = tick counter
  NixieGlyph _start;   ///< values at which fade started, duration value unused
  NixieGlyph _target;  ///< target values for fader, duration value = duration of crossfade
};

}  // namespace kbxTubeClock
