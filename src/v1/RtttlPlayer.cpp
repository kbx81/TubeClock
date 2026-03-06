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
// Parses and plays RTTTL (Ring Tone Text Transfer Language) strings using
// the Hardware tone queue. Adapted from ESPHome's rtttl component.
//
// RTTTL format: "Name:d=<dur>,o=<oct>,b=<bpm>:<notes>"
//   - d: default note duration (1=whole, 2=half, 4=quarter, 8=eighth, 16, 32)
//   - o: default octave (4-7)
//   - b: BPM (beats per minute, quarter notes)
//   - notes: comma-separated, each is [duration][note][#][octave][.]
//     e.g. 8c#5. = dotted eighth C-sharp in octave 5
//
#include <cstdint>
#include <cstring>

#include "Hardware.h"
#include "RtttlPlayer.h"

namespace kbxTubeClock::RtttlPlayer {

// Note frequency table (Hz): index 0 = silence, 1-12 = C4-B4, 13-24 = C5-B5,
// 25-36 = C6-B6, 37-48 = C7-B7. Matches values from the ESPHome RTTTL component
// and the Arduino Tone library.
//
static const uint16_t cNotes[] = {
    1,  // silence (below cToneFrequencyMinimum; Hardware treats as timed silence)
    262,  277,  294,  311,  330,  349,  370,  392,  415,  440,  466,  494,   // C4-B4
    523,  554,  587,  622,  659,  698,  740,  784,  831,  880,  932,  988,   // C5-B5
    1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,  // C6-B6
    2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951   // C7-B7
};

static const uint8_t cNotesCount = sizeof(cNotes) / sizeof(cNotes[0]);

// Maximum RTTTL string length that can be buffered for playback.
// Sized to consume the full SerialRemote payload (254 bytes max) minus the
// 3-byte "CBP" prefix, leaving 251 bytes for the RTTTL string itself.
//
static const uint8_t cRtttlBufferSize = 251;

// Playback buffer
//
static char _rtttl[cRtttlBufferSize + 1];
static uint8_t _length = 0;
static uint8_t _position = 0;  // parse position in _rtttl[]

// Header defaults (parsed from RTTTL header section)
//
static uint8_t _defaultDuration = 4;
static uint8_t _defaultOctave = 6;
static uint16_t _wholenote = 0;  // ms per whole note = 60000*4/bpm

// Next note pre-parsed and ready to feed into the Hardware queue
//
static uint16_t _nextFreq = 0;
static uint16_t _nextDur = 0;
static bool _hasNextNote = false;

// Playback state flags
//
static bool _playing = false;
static bool _finished = false;  // self-clearing, signals one completion event

// --- Read consecutive ASCII digits from _rtttl[_position], return as integer ---
//
static uint8_t _getInteger() {
  uint8_t ret = 0;
  while (_position < _length && _rtttl[_position] >= '0' && _rtttl[_position] <= '9') {
    ret = static_cast<uint8_t>(ret * 10 + (_rtttl[_position++] - '0'));
  }
  return ret;
}

// --- Map an RTTTL note character to a base cNotes[] index ---
// Returns 0 for silence (pause), 1-12 for C-B within one octave.
//
static uint8_t _noteIndex(char c) {
  switch (c) {
    case 'c':
      return 1;  // 'c#' is index 2
    case 'd':
      return 3;  // 'd#' is index 4
    case 'e':
      return 5;
    case 'f':
      return 6;  // 'f#' is index 7
    case 'g':
      return 8;  // 'g#' is index 9
    case 'a':
      return 10;  // 'a#' is index 11
    case 'b':
    case 'h':
      return 12;  // German 'h' = English 'b' natural
    case 'p':
    default:
      return 0;  // pause / silence
  }
}

// --- Advance _position to just past the next occurrence of character c ---
// Returns true if found; leaves _position at end of string if not found.
//
static bool _findChar(char c) {
  while (_position < _length) {
    if (_rtttl[_position++] == c) {
      return true;
    }
  }
  return false;
}

// --- Advance _position to just past the next occurrence of the two-char sequence c0,c1 ---
//
static bool _findStr2(char c0, char c1) {
  while (_position + 1 < _length) {
    if (_rtttl[_position] == c0 && _rtttl[_position + 1] == c1) {
      _position += 2;
      return true;
    }
    _position++;
  }
  return false;
}

// --- Parse the next note from _rtttl[_position], store result in freq/dur ---
// Returns true if a note was successfully parsed; false if end of string.
//
static bool _parseNote(uint16_t &freq, uint16_t &dur) {
  // Skip comma/space separators
  while (_position < _length && (_rtttl[_position] == ',' || _rtttl[_position] == ' ')) {
    _position++;
  }

  if (_position >= _length) {
    return false;
  }

  // Optional duration prefix (e.g. "8" in "8c5")
  uint8_t num = _getInteger();
  uint16_t noteDuration = (num > 0) ? (_wholenote / num) : (_wholenote / _defaultDuration);

  // Note character
  if (_position >= _length) {
    return false;
  }
  uint8_t note = _noteIndex(_rtttl[_position++]);

  // Optional sharp ('#')
  if (_position < _length && _rtttl[_position] == '#') {
    note++;
    _position++;
  }

  // Optional octave digit
  uint8_t scale = _getInteger();
  if (scale == 0) {
    scale = _defaultOctave;
  }

  // Optional dotted note ('.'): extends duration by 50%
  if (_position < _length && _rtttl[_position] == '.') {
    noteDuration += noteDuration / 2;
    _position++;
  }

  // Map note + octave to frequency
  if (note == 0) {
    // Pause: use NOTE_REST value (1 Hz, below cToneFrequencyMinimum).
    // Hardware::tone() clears the PWM output and runs the timer for 'dur' ms.
    freq = cNotes[0];
  } else {
    // Clamp octave to supported range (4-7)
    if (scale < 4) {
      scale = 4;
    }
    if (scale > 7) {
      scale = 7;
    }

    uint8_t idx = static_cast<uint8_t>((scale - 4) * 12 + note);
    freq = (idx < cNotesCount) ? cNotes[idx] : cNotes[0];
  }

  dur = noteDuration;
  return true;
}

void play(const char *rtttl, uint8_t length) {
  if (rtttl == nullptr || length == 0) {
    return;
  }

  // Copy (and truncate) the RTTTL string into the playback buffer
  if (length > cRtttlBufferSize) {
    length = cRtttlBufferSize;
  }
  memcpy(_rtttl, rtttl, length);
  _rtttl[length] = '\0';
  _length = length;
  _position = 0;

  // Defaults (may be overridden by RTTTL header)
  _defaultDuration = 4;
  _defaultOctave = 6;
  int bpm = 63;

  // Skip song name: find first ':'
  if (!_findChar(':')) {
    return;
  }

  // Parse "d=<n>" (default duration)
  if (!_findStr2('d', '=')) {
    return;
  }
  uint8_t num = _getInteger();
  if (num > 0) {
    _defaultDuration = num;
  }

  // Parse "o=<n>" (default octave)
  if (!_findStr2('o', '=')) {
    return;
  }
  num = _getInteger();
  if (num >= 3 && num <= 7) {
    _defaultOctave = num;
  }

  // Parse "b=<n>" (BPM)
  if (!_findStr2('b', '=')) {
    return;
  }
  num = _getInteger();
  if (num > 0) {
    bpm = num;
  }

  // Find second ':' to reach the note sequence
  if (!_findChar(':')) {
    return;
  }

  // BPM is expressed in quarter notes per minute; scale to whole-note duration
  _wholenote = static_cast<uint16_t>(60000UL * 4 / static_cast<uint32_t>(bpm));

  // Pre-parse the first note so loop() can feed it immediately
  _hasNextNote = _parseNote(_nextFreq, _nextDur);
  _playing = _hasNextNote;
  _finished = false;
}

void stop() {
  _playing = false;
  _hasNextNote = false;
}

void loop() {
  if (!_playing) {
    return;
  }

  if (!_hasNextNote) {
    // All notes have been accepted into the Hardware queue; signal completion.
    // The last queued note(s) will continue to play out in hardware.
    _playing = false;
    _finished = true;
    return;
  }

  // Attempt to feed the pre-parsed note into the Hardware tone queue
  Hardware::HwReqAck result = Hardware::tone(_nextFreq, _nextDur);

  if (result == Hardware::HwReqAckError) {
    // Both queue slots are full; try again on the next loop() call
    return;
  }

  // Note was accepted (started immediately or queued) -- advance the parser
  _hasNextNote = _parseNote(_nextFreq, _nextDur);
}

bool isPlaying() { return _playing; }

bool playingFinished() {
  if (_finished) {
    _finished = false;
    return true;
  }
  return false;
}

}  // namespace kbxTubeClock::RtttlPlayer
