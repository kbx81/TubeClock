//
// kbx81's tube clock display animator
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

#include "Display.h"
#include "Keys.h"
#include "Settings.h"


namespace kbxTubeClock {

namespace Animator {


  /// @brief Initialize the alarms
  ///
  void initialize();

  /// @brief Handles key presses (during an active alarm)
  /// @param key Key pressed
  ///
  void keyHandler(Keys::Key key);

  /// @brief Sets the delay between frames in ticks
  /// @param ticksBetweenFrames Number of ticks between frames
  ///
  void setDelayBetweenFrames(const uint16_t ticksBetweenFrames);

  /// @brief Sets the delay between frames in ticks
  /// @param duration Desired length of animation in ticks
  ///
  void setAnimationDuration(const uint16_t duration);

  /// @brief Sets the final display for the animation
  /// @param display Animation's final display
  ///
  void setFinalDisplay(const Display display);

  /// @brief Sets the initial display for the animation
  /// @param display Animation's initial display
  ///
  void setInitialDisplay(const Display display);

  /// @brief Check if an animation is running/active
  /// @return true if an animation is in progress
  ///
  bool isRunning();

  /// @brief Runs the specified animation
  /// @param animationId Animation to run. 0 = select a random animation
  ///
  void run(const uint8_t animationId = 0);

  /// @brief Called from the main application loop
  ///
  void loop();

  /// @brief Call at fixed intervals (via systick?) to advance animations
  ///
  void tick();
}

}
