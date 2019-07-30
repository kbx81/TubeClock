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


#include <cstdint>

#include "NixieGlyph.h"


namespace kbxTubeClock {

/// @brief kbx Tube Clock NixieGlyphCrossfader class
///

class NixieGlyphCrossfader
{
public:
  /// @brief Default constructor
  ///
  NixieGlyphCrossfader();

  /// @brief Returns true if a crossfade is in progress
  ///
  /// @return true if a crosfade is currently in progress/not complete
  ///
  bool fadeIsActive();

  /// @brief Gets the fader's active/current value
  ///
  /// @return fader's current values with duration set to number of ticks elapsed
  ///
  NixieGlyph getActive() const;

  /// @brief Gets the fader's target values
  ///
  /// @return fader's target values, fade duration included
  ///
  NixieGlyph getTargetWithDuration() const;

  /// @brief Restarts fade to target from active values with existing duration
  ///
  void resetFade();

  /// @brief Starts a new fade with (a) new target value(s)
  ///
  /// @param newTarget New RGB target values for fader, including duration
  ///
  void startNewFade(const NixieGlyph &newTarget);

  /// @brief Starts a new fade with (a) new target value(s) if target values are different from existing values
  ///
  /// @param newTarget New RGB target values for fader, including duration
  ///
  void startNewFadeIfDifferent(const NixieGlyph &newTarget);

  /// @brief Advances all crossfades currently in progress (call at fixed intervals)
  ///
  void tick();

  /// @brief Updates fade duration only
  ///
  /// @param newDuration New duration for fader (RgbLed RGB values ignored)
  ///
  void updateFadeDuration(const uint32_t newDuration);
  void updateFadeDuration(const NixieGlyph &newDuration);

  /// @brief Updates fade target value(s) only (resumes fade if it was completed)
  ///
  /// @param newTarget New target value(s) for fader
  /// @param startNewFade New fade begins if true
  ///
  void updateFadeTarget(const NixieGlyph &newTarget, const bool startNewFade = false);

  /// @brief Updates fade target value(s) and duration (resumes fade if it was completed)
  ///
  /// @param newTarget New target value(s) for fader including duration
  /// @param startNewFade New fade begins if true
  ///
  void updateFadeTargetAndDuration(const NixieGlyph &newTarget, const bool startNewFade = false);


private:
  NixieGlyph _active;   ///< current values of fader, duration value = tick counter
  NixieGlyph _start;    ///< values at which fade started, duration value unused
  NixieGlyph _target;   ///< target values for fader, duration value = duration of crossfade
};


}
