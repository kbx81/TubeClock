//
// kbx81's tube clock keys class
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
#include "InfraredRemote.h"
#include "Keys.h"
#include "SerialRemote.h"


namespace kbxTubeClock {

namespace Keys {


  // The number of defined keys
  //
  static const uint8_t cKeyCount = 6;

  // The key masks for all tested keys
  //
  static const Key cKeyMasks[cKeyCount] = { A, B, C, E, D, U };

  // The size of the key queue
  //
  static const uint8_t cKeyQueueSize = 8;

  // The initial delay until repeat starts
  //
  static const uint16_t cRepeatDelay = 1000;

  // The speed of the repeat
  //
  static const uint8_t cRepeatSpeed = 75;


  // The last bitmask of the key states
  //
  static Key _currentPressedKey = None;

  // The number of checks since a key was pressed down
  //
  static uint16_t _keyPressedTimeCount = 0;

  // Flag if we are already in repeat mode
  //
  static bool _repeatMode = false;

  // Key queue for received keys (circular buffer)
  //
  static uint8_t _keyQueue[cKeyQueueSize];

  // Circular buffer head (read) and tail (write) indices
  //
  static uint8_t _queueHead = 0;
  static uint8_t _queueTail = 0;


  // Get the current pressed key
  //
  Key currentlyPressedKey()
  {
    Hardware::buttonsRefresh();
    const uint8_t mask = Hardware::buttons();
    uint8_t i = 0;

    for (i = 0; i < cKeyCount; ++i)
    {
      if ((mask & cKeyMasks[i]) != 0)
      {
        return cKeyMasks[i];
      }
    }

    if (InfraredRemote::hasKeyPress())
    {
      auto irKey = InfraredRemote::getKeyPress();
      if (irKey < InfraredRemote::IrKey::Count)
      {
        return cKeyMasks[irKey];
      }
    }

    return None;
  }


  // Add a key press to the queue.
  //
  void addKeyPress(Key key)
  {
    uint8_t nextTail = (_queueTail + 1) & (cKeyQueueSize - 1);
    if (nextTail != _queueHead)
    {
      _keyQueue[_queueTail] = key;
      _queueTail = nextTail;
    }
  }


  void scanKeys()
  {
    auto key = currentlyPressedKey();
    if (_currentPressedKey != key)
    {
      if (_currentPressedKey != None)
      {
        SerialRemote::notifyKeyEvent(_currentPressedKey, false);
      }
      _currentPressedKey = key;
      _keyPressedTimeCount = 0;
      _repeatMode = false;
      if (key != None)
      {
        addKeyPress(key);
        SerialRemote::notifyKeyEvent(key, true);
      }
    }
    // key repeating could be handled here, but we'll do it seperately (below)
    //  so that the timing of the repeats is more consistent
  }


  void repeatHandler()
  {
    if (_currentPressedKey != None)
    {
      ++_keyPressedTimeCount;
      if (_repeatMode && _keyPressedTimeCount > cRepeatSpeed)
      {
        _keyPressedTimeCount = 0;
        addKeyPress(_currentPressedKey);
      }
      else if (_keyPressedTimeCount > cRepeatDelay)
      {
        _keyPressedTimeCount = 0;
        addKeyPress(_currentPressedKey);
        _repeatMode = true;
      }
    }
  }


  bool hasKeyPress()
  {
    return (_queueHead != _queueTail);
  }


  Key getKeyPress()
  {
    if (_queueHead == _queueTail) return None;

    auto key = _keyQueue[_queueHead];
    _queueHead = (_queueHead + 1) & (cKeyQueueSize - 1);
    return (Key)key;
  }


}

}
