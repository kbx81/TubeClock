//
// kbx81's tube clock InfraredRemote Library
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
//
// IR timing info (times in microseconds)
// --------------------------------------
// Name	        Desired  +20%   -20%
// LeadPulse	    9000	10800	  7200
// Space	        4500	 5400	  3600
// RepeatSpace	  2250   2700	  1800
// BitHead	     562.5    675	   450
// BitHigh	    1687.5	 2025	  1350
// BitLow	       562.5	  675	   450
// EndSpace	     562.5	  675	   450

#include <libopencm3/stm32/timer.h>

#include "Hardware.h"
#include "InfraredRemote.h"

#if HARDWARE_VERSION == 1
  #include "Hardware_v1.h"
#else
  #error HARDWARE_VERSION must be defined with a value of 1
#endif


namespace kbxTubeClock {

namespace InfraredRemote
{

  /// @brief Infrared remote state machine states
  ///
  enum IrRxState : uint8_t {
    IrRxLeadPulse   = 0,
    IrRxSpace       = 1,
    IrRxRepeatSpace = 2,
    IrRxBitHead     = 3,
    IrRxBitHigh     = 4,
    IrRxBitLow      = 5,
    IrRxEndSpace    = 6,
    IrRxNextBit     = 7,  // does not correspond to the arrays below!
    IrRxIdle        = 8   // does not correspond to the arrays below!
  };


// infrared pulse timings (in microseconds)
//
static const uint16_t cIrPulseTimeUpper[] = { 10800, 5400, 2700, 675, 2025, 675, 675 };
static const uint16_t cIrPulseTimeLower[] = {  7200, 3600, 1800, 450, 1350, 450, 450 };

// data packet size in bytes
//
static const uint8_t cPacketSize = 4;

// value at which the repeat countdown starts
//
static const uint8_t cRepeatCountdownStart = 12;


// the bit we're currently receving
//
volatile static uint8_t _incomingBit = 0;

// the packet we're currently receving or just received
//
volatile static uint8_t _incomingPacket[cPacketSize];

// true if a key press is available
//
volatile static bool _keypressAvailable = 0;

// the last-received address
//
volatile static uint8_t _lastRxAddress = 0;

// the last-received command
//
volatile static uint8_t _lastRxCommand = 0;

// counts down to zero at which time repeating is considered to have stopped
//
volatile static uint8_t _repeatStopCountdown = 0;

// the byte in the packet we're receving now
//
volatile static uint8_t _rxPacketByte = 0;

// tracks the receiver state machine's state
//
volatile static IrRxState _rxState = IrRxState::IrRxIdle;


/// @brief Rolls a bit into the buffers
/// @return True if receive is complete
///
bool _rxBit(const uint8_t bit)
{
  if (_incomingBit >= 8)
  {
    _incomingBit = 0;

    if (++_rxPacketByte >= cPacketSize)
    {
      return true;
    }
  }

  _incomingPacket[_rxPacketByte] |= (bit << _incomingBit++);

  if ((_incomingBit >= 7) && (_rxPacketByte >= cPacketSize - 1))
  {
    return true;
  }
  return false;
}


/// @brief Sets the receiver state machine to idle/default
///
void _setRxIdle()
{
  _rxState = IrRxIdle;
  _incomingBit = 0;
  _rxPacketByte = 0;
  _incomingPacket[0] = 0;
  _incomingPacket[1] = 0;
  _incomingPacket[2] = 0;
  _incomingPacket[3] = 0;
}


/// @brief Checks the received packet for validity
/// @return True if valid
///
bool _verifyRx()
{
  uint8_t inverted = (~_incomingPacket[3]) & 0x7f;

  return (_incomingPacket[2] == inverted);

  // per the NEC spec, we should be doing this but the Adafruit remote is...weird...
  // return ((_incomingPacket[0] == ~(_incomingPacket[1])) && (_incomingPacket[2] == ~(_incomingPacket[3])));
}


/// @brief Sets the receiver state machine to idle/default
///
void _storeRx()
{
  _lastRxAddress = _incomingPacket[0];
  _lastRxCommand = _incomingPacket[2];
}


void initialize()
{
  _setRxIdle();
}


bool hasKeyPress()
{
  return _keypressAvailable;
}


InfraredRemoteKey getKeyPress()
{
  return (InfraredRemoteKey)_lastRxCommand;
}


bool keyIsHeld()
{
  return (_repeatStopCountdown > 0);
}


void tick()
{
  auto timerValue = timer_get_counter(Hardware::cIrTimer);
  bool pinState = (gpio_get(Hardware::cIrPort, Hardware::cIrPin) != 0);

  // reset the counter to begin measuring the now-in-progress pulse
  timer_set_counter(Hardware::cIrTimer, 1);

  switch (_rxState)
  {
    // after we've received the lead pulse, we should see either a space or a repeat-space
    case IrRxLeadPulse:
      if ((timerValue > cIrPulseTimeLower[IrRxSpace]) && (timerValue < cIrPulseTimeUpper[IrRxSpace]) && (pinState == false))
      {
        _rxState = IrRxSpace;         // advance to the next state
      }
      else if ((timerValue > cIrPulseTimeLower[IrRxRepeatSpace]) && (timerValue < cIrPulseTimeUpper[IrRxRepeatSpace]) && (pinState == false))
      {
        _rxState = IrRxRepeatSpace;   // advance to the next state
      }
      else
      {
        _setRxIdle();                 // nothing matched so reset the state machine
      }
      break;
    // we got the space. now it's time to start receiving bits! we expect a pulse of the IrRxBitHead length
    case IrRxSpace:
      _repeatStopCountdown = cRepeatCountdownStart;
    case IrRxNextBit:
      if ((timerValue > cIrPulseTimeLower[IrRxBitHead]) && (timerValue < cIrPulseTimeUpper[IrRxBitHead]) && (pinState == true))
      {
        _rxState = IrRxBitHead;       // advance to the next state
      }
      else
      {
        _setRxIdle();                 // nothing matched so reset the state machine
      }
      break;
    // we just expect a pulse of the IrRxBitHead length to confirm the repeat signal
    case IrRxRepeatSpace:
      if ((timerValue > cIrPulseTimeLower[IrRxBitLow]) && (timerValue < cIrPulseTimeUpper[IrRxBitLow]) && (pinState == true))
      {
        _repeatStopCountdown = cRepeatCountdownStart;

        // _keypressAvailable = true;    // can cause unexpected key presses :(

        _setRxIdle();                 // nothing matched so reset the state machine
      }
      break;
    // if we got the correct bit-leading pulse, we'll determine the length of the following pulse to determine whether it was a 0 bit or a 1 bit
    case IrRxBitHead:
      if ((timerValue > cIrPulseTimeLower[IrRxBitHigh]) && (timerValue < cIrPulseTimeUpper[IrRxBitHigh]) && (pinState == false))
      {
        // it was a '1' bit
        if (_rxBit(1) == false)
        {
          _rxState = IrRxNextBit;     // advance to the next state
        }
        else
        {
          _rxState = IrRxEndSpace;    // advance to the next state
        }
      }
      else if ((timerValue > cIrPulseTimeLower[IrRxBitLow]) && (timerValue < cIrPulseTimeUpper[IrRxBitLow]) && (pinState == false))
      {
        // it was a '0' bit
        if (_rxBit(0) == false)
        {
          _rxState = IrRxNextBit;     // advance to the next state
        }
        else
        {
          _rxState = IrRxEndSpace;    // advance to the next state
        }
      }
      else
      {
        // it was invalid
        _setRxIdle();                 // nothing matched so reset the state machine
      }
      break;

    case IrRxEndSpace:
      if ((timerValue > cIrPulseTimeLower[IrRxBitLow]) && (timerValue < cIrPulseTimeUpper[IrRxBitLow]) && (pinState == true))
      {
        if (_verifyRx() == true)
        {
          _storeRx();
          _keypressAvailable = true;
        }
        _setRxIdle();                 // we are done! reset the state machine
      }
      break;
    // state was idle or undefined. we start from scratch here.
    default:
      if ((timerValue > cIrPulseTimeLower[IrRxLeadPulse]) && (timerValue < cIrPulseTimeUpper[IrRxLeadPulse]) && (pinState == true))
      {
        _rxState = IrRxLeadPulse;   // advance to the next state
      }
      else
      {
        timer_enable_counter(Hardware::cIrTimer);
      }
  }
}


void overflow()
{
  if (_repeatStopCountdown == 0)
  {
    timer_disable_counter(Hardware::cIrTimer);

    _keypressAvailable = false;
  }
  else
  {
    _repeatStopCountdown--;
  }

  _setRxIdle();
}


}

}
