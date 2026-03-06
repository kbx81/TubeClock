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
#include <cstdint>

#include "NixieGlyph.h"
#include "NixieGlyphCrossfader.h"

namespace kbxTubeClock {

NixieGlyph NixieGlyphCrossfader::getActive() const { return _active; }

void NixieGlyphCrossfader::startNewFade(const NixieGlyph &newTarget) {
  if (newTarget.getIntensity() == _target.getIntensity()) {
    // Intensity unchanged. If duration=0 is requested and a fade is still in
    // progress, snap to target immediately — this allows a mode switch to
    // interrupt an in-progress fade even when the intensity already matches.
    if (newTarget.getDuration() == 0 && _active.getDuration() < _target.getDuration()) {
      _active = _target;
      _active.setDuration(_target.getDuration() + 1);
    }
    return;
  }

  // Save current position and reset tick counter BEFORE updating _target.
  // This prevents a race with tick() (ISR): if tick() fires after _target is
  // set but before the counter is reset, it sees currentTick >= totalTicks
  // with the new target and snaps _active immediately (skipping the fade).
  // By resetting the counter first, tick() sees currentTick=0 < totalTicks
  // and interpolates from _start (staying at the current position).
  _start = _active;
  _active.setDuration(0);
  _target = newTarget;

  // Duration 0 means instant transition — snap immediately to avoid
  // stale _active values being read by refresh() before the next tick()
  if (_target.getDuration() == 0) {
    _active = _target;
    _start = _target;
  }
}

void NixieGlyphCrossfader::tick() {
  // the _active NixieGlyph object's duration value is the current number of ticks into the fade
  uint32_t currentTick = _active.getDuration();
  uint32_t totalTicks = _target.getDuration();

  if (currentTick < totalTicks) {
    // set _active to the interpolated value between _start and _target
    _active.setFromLinearInterpolation(currentTick, totalTicks, _start, _target);
    _active.setDuration(currentTick + 1);
  } else if (currentTick == totalTicks) {
    // Snap to target exactly once; push duration past totalTicks so subsequent
    // ticks fall through to the implicit else (do nothing), avoiding repeated
    // writes to _active and the concurrent-update hazard with main-loop callers
    _active = _target;
    _active.setDuration(totalTicks + 1);
  }
  // else: currentTick > totalTicks — fade already complete, nothing to do
}

};  // namespace kbxTubeClock
