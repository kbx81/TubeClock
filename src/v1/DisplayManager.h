//
// kbx81's tube clock DisplayManager Library
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
#include "RgbLed.h"

/// @brief kbx Binary Clock DisplayManager
///
namespace kbxTubeClock::DisplayManager {

/// @brief Initialize DisplayManager
///
void initialize();

/// @brief Refreshes intensities of tubes that are changing intensity levels due to (an) active crossfade(s)
/// @note This function is now a no-op. Refresh is integrated into tick() for better performance.
///
void refresh();

/// @brief Triggers a refresh of tubes intensities next time refresh() is called
///
void tick();

/// @brief Triggers a refresh of the tube PWM states and refreshes the drivers
///
void tickPWM();

/// @brief Enables/Disables control of the status LED by refresh() (above)
/// @param autoRefreshEnabled New state for status LED auto-refreshing
///
void setStatusLedAutoRefreshing(const bool autoRefreshEnabled);

/// @brief Returns the current "master" intensity of the display
/// @return Current master intensity (0-255)
///
uint8_t getMasterIntensity();

/// @brief Sets the "master" intensity of the display (0-255)
/// @param intensity New master intensity for display. Display is updated at next tick()
void setMasterIntensity(const uint8_t intensity);

/// @brief Get the state of display blanking
/// @return true if display is blank/off
///
bool getDisplayBlanking();

/// @brief Returns true if a blink() animation is currently in progress
///
bool isBlinkActive();

/// @brief Blank the display
/// @param blank Display is blank (off) if true
///
void setDisplayBlanking(const bool blank);

/// @brief Blinks the display count times as a non-blocking animation.
///  The animation is driven by tick() and completes asynchronously (~100ms per blink).
/// @param count Number of blinks (default 2)
///
void blink(uint8_t count = 2);

/// @brief Writes the passed display into the display buffer. Tubes will fade
///  to the intensities in the new display at the specified rates.
/// @param display Display to be written
/// @param statusLed Status LED state
///
void writeDisplay(const Display &display);
void writeDisplay(const Display &display, const RgbLed &statusLed);

/// @brief Sets the status LED's state
/// @param statusLed New status LED state
///
void writeStatusLed(const RgbLed &statusLed);

/// @brief Returns the current status LED state as stored in the DisplayManager
/// @return Current RgbLed value
///
RgbLed getStatusLed();

}  // namespace kbxTubeClock::DisplayManager
