//
// kbx81's tube clock RTTTL player
// ---------------------------------------------------------------------------
// (c)2025 by kbx81. See LICENSE for details.
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

namespace kbxTubeClock::RtttlPlayer {

/// @brief Begin playing an RTTTL string (stops any current playback)
/// @param rtttl Pointer to the RTTTL string (not required to be null-terminated)
/// @param length Length of the RTTTL string in bytes
///
void play(const char *rtttl, uint8_t length);

/// @brief Stop playback immediately
///
void stop();

/// @brief Feed the next pending note into the Hardware tone queue (call from main loop)
///
void loop();

/// @brief Returns true if playback is currently active
///
bool isPlaying();

/// @brief Returns true once after playback completes, then clears itself
///
bool playingFinished();

}  // namespace kbxTubeClock::RtttlPlayer
