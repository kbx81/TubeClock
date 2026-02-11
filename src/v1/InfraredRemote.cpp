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

// value at which the repeat countdown starts
//
static const uint8_t cRepeatCountdownStart = 12;

// Key codes from the remote (address = 0xBF00 for all):
//  VOLUMEDOWN  command = 0xFF00
//  PLAY/PAUSE  command = 0xFE01
//  VOLUMEUP    command = 0xFD02
//  SETUP       command = 0xFB04
//  UP          command = 0xFA05
//  STOP/MODE   command = 0xF906
//  LEFT        command = 0xF708
//  ENTER/SAVE  command = 0xF609
//  RIGHT       command = 0xF50A
//  ZERO/TEN    command = 0xF30C
//  DOWN        command = 0xF20D
//  BACK        command = 0xF10E
//  ONE         command = 0xEF10
//  TWO         command = 0xEE11
//  THREE       command = 0xED12
//  FOUR        command = 0xEB14
//  FIVE        command = 0xEA15
//  SIX         command = 0xE916
//  SEVEN       command = 0xE718
//  EIGHT       command = 0xE619
//  NINE        command = 0xE51A

// runtime-modifiable key code table (initialized with defaults)
// TODO: allow runtime user-defined values and initialize from flash/preferences
//
static IrKeyCode _keyCodeTable[IrKey::Count] = {
  { 0xBF00, 0xF609 },  // A (remote: Enter/Save)
  { 0xBF00, 0xF50A },  // B (remote: Right)
  { 0xBF00, 0xF708 },  // C (remote: Left)
  { 0xBF00, 0xF906 },  // E (remote: Stop/Mode)
  { 0xBF00, 0xF20D },  // D (remote: Down)
  { 0xBF00, 0xFA05 },  // U (remote: Up)
};


// the bit we're currently receiving
//
volatile static uint8_t _incomingBit = 0;

// the packet we're currently receiving
//
volatile static uint32_t _incomingPacket = 0;

// true if a key press is available
//
volatile static bool _keypressAvailable = 0;

// the last-received address (full 16-bit NEC address word)
//
volatile static uint16_t _lastRxAddress = 0;

// the last-received command (full 16-bit NEC command word)
//
volatile static uint16_t _lastRxCommand = 0;

// counts down to zero at which time repeating is considered to have stopped
//
volatile static uint8_t _repeatStopCountdown = 0;

// tracks the receiver state machine's state
//
volatile static IrRxState _rxState = IrRxState::IrRxIdle;


/// @brief Rolls a bit into the accumulator
/// @return True if receive is complete (32 bits)
///
bool _rxBit(const uint8_t bit)
{
  _incomingPacket |= ((uint32_t)bit << _incomingBit);
  return (++_incomingBit >= 32);
}


/// @brief Sets the receiver state machine to idle/default
///
void _setRxIdle()
{
  _rxState = IrRxIdle;
  _incomingBit = 0;
  _incomingPacket = 0;
}


/// @brief Stores the received packet as 16-bit address and command words
///
void _storeRx()
{
  _lastRxAddress = (uint16_t)_incomingPacket;
  _lastRxCommand = (uint16_t)(_incomingPacket >> 16);
}


void initialize()
{
  _setRxIdle();
}


bool hasKeyPress()
{
  return _keypressAvailable;
}


IrKey getKeyPress()
{
  uint16_t addr = _lastRxAddress;
  uint16_t cmd = _lastRxCommand;

  for (uint8_t i = 0; i < IrKey::Count; ++i)
  {
    if (_keyCodeTable[i].address == addr && _keyCodeTable[i].command == cmd)
    {
      return static_cast<IrKey>(i);
    }
  }
  return IrKey::None;
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
      [[fallthrough]];
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
    // if we got the correct bit-leading pulse, determine the space length to decode the bit value
    case IrRxBitHead:
      if (pinState == false)
      {
        uint8_t bit;
        if ((timerValue > cIrPulseTimeLower[IrRxBitHigh]) && (timerValue < cIrPulseTimeUpper[IrRxBitHigh]))
        {
          bit = 1;
        }
        else if ((timerValue > cIrPulseTimeLower[IrRxBitLow]) && (timerValue < cIrPulseTimeUpper[IrRxBitLow]))
        {
          bit = 0;
        }
        else
        {
          _setRxIdle();
          break;
        }
        _rxState = _rxBit(bit) ? IrRxEndSpace : IrRxNextBit;
      }
      else
      {
        _setRxIdle();
      }
      break;

    case IrRxEndSpace:
      if ((timerValue > cIrPulseTimeLower[IrRxBitLow]) && (timerValue < cIrPulseTimeUpper[IrRxBitLow]) && (pinState == true))
      {
        _storeRx();
        _keypressAvailable = true;
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

  if (_rxState != IrRxIdle)
  {
    _setRxIdle();
  }
}


}

}
