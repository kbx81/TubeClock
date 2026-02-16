//
// kbx81's tube clock serial remote control interface
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

#include "Application.h"
#include "DateTime.h"
#include "GpsReceiver.h"
#include "Hardware.h"
#include "SerialRemote.h"
#include "Settings.h"

#if HARDWARE_VERSION == 1
  #include "Hardware_v1.h"
#else
  #error HARDWARE_VERSION must be defined with a value of 1
#endif


namespace kbxTubeClock {

namespace SerialRemote
{


// RX parser state machine
//
enum RxState : uint8_t {
  RxIdle,
  RxGettingHeader,
  RxGettingPayload,
  RxGettingChecksum1,
  RxGettingChecksum2
};


// Maximum payload length (between "$TC" header and "*" checksum marker)
//
static const uint8_t cMaxPayloadLength = 32;

// Protocol header: "$TC"
//
static const char cHeader[] = "$TC";
static const uint8_t cHeaderLength = 3;

// TX buffer size
//
static const uint8_t cTxBufferSize = 48;

// Number of TX USARTs we send responses on
//
static const uint8_t cTxUsartCount = 2;

// Maximum valid OperatingMode value (OperatingModeTestDisplay)
//
static const uint8_t cMaxOperatingMode = 41;


// --- RX state ---

// Double-buffered: ISR fills one, main loop processes the other
static char _rxPayload[2][cMaxPayloadLength];
static uint8_t _rxPayloadLength[2] = { 0, 0 };
// Index of the buffer the ISR is currently filling
static volatile uint8_t _rxFillBuffer = 0;
// Flag indicating a complete command is ready in the non-fill buffer
static volatile bool _rxCommandReady = false;

static RxState _rxState = RxState::RxIdle;
static uint8_t _rxHeaderIndex = 0;
static uint8_t _rxPayloadIndex = 0;
static uint8_t _rxChecksum = 0;
static uint8_t _rxReceivedChecksum = 0;


// --- TX state ---

static char _txBuffer[cTxBufferSize];
static volatile uint8_t _txLength = 0;

// Per-USART TX progress tracking
static volatile uint8_t _txIndex[cTxUsartCount] = { 0, 0 };
static volatile bool _txActive[cTxUsartCount] = { false, false };

// Map USART peripheral to TX index: USART1 -> 0, USART4 -> 1
static uint8_t _txUsartIndex(uint32_t usart)
{
  return (usart == Hardware::cGpsUsart) ? 0 : 1;
}

// Cached Usart pointers for TX (USART1, USART4) and RX (USART3)
static Usart* _txUsartPtrs[cTxUsartCount] = { nullptr, nullptr };
static Usart* _rxUsart3Ptr = nullptr;

// Suppresses unsolicited notifications during command processing so the
// command handler can send a single authoritative response with final state
static bool _suppressNotifications = false;

// Deferred mode change notification -- set when notifyModeChange() is called
// but TX is busy (e.g. key press notification in progress). Cleared and sent
// the next time process() runs and TX is idle.
static bool _pendingModeNotification = false;

// Deferred key event notification state
static uint8_t _pendingKeyMask = 0;
static bool _pendingKeyPressed = false;
static bool _pendingKeyNotification = false;

// Intensity change notification state
static uint8_t _prevIntensity = 0;
static bool _pendingIntensityNotification = false;

// Temperature notification state -- tracks previous Cx10 values to detect changes
static int32_t _prevTempValues[Hardware::cTempSensorCount] = {
  Hardware::cTempSentinel, Hardware::cTempSentinel, Hardware::cTempSentinel,
  Hardware::cTempSentinel, Hardware::cTempSentinel
};
static bool _pendingTempNotification = false;

// ADC change notification state -- tracks previous values to detect changes
static uint16_t _prevAdcLight = 0;
static uint16_t _prevAdcVddA = 0;
static uint16_t _prevAdcVbatt = 0;
static bool _pendingAdcNotification = false;


// --- Helper: convert nibble to hex char ---
static char _toHex(uint8_t nibble)
{
  return (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
}


// --- Helper: convert hex char to nibble ---
static uint8_t _fromHex(char c)
{
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0xFF;  // invalid
}


// --- Helper: append a single char to the TX buffer ---
static void _txAppendChar(char c)
{
  if (_txLength < cTxBufferSize - 1)
  {
    _txBuffer[_txLength++] = c;
  }
}


// --- Helper: append a null-terminated string to the TX buffer ---
static void _txAppendString(const char* str)
{
  while (*str)
  {
    _txAppendChar(*str++);
  }
}


// --- Helper: append a decimal number to the TX buffer ---
static void _txAppendDecimal(int32_t value, uint8_t minDigits)
{
  if (value < 0)
  {
    _txAppendChar('-');
    value = -value;
  }

  char digits[11];
  uint8_t count = 0;
  do {
    digits[count++] = '0' + (value % 10);
    value /= 10;
  } while (value > 0);

  while (count < minDigits)
  {
    digits[count++] = '0';
  }

  for (uint8_t i = count; i > 0; i--)
  {
    _txAppendChar(digits[i - 1]);
  }
}


// --- Start building a response in the TX buffer with category prefix ---
static void _txBeginResponse(const char* prefix)
{
  _txLength = 4;
  _txBuffer[0] = '$';
  _txBuffer[1] = 'T';
  _txBuffer[2] = 'C';
  _txBuffer[3] = 'S';
  _txAppendString(prefix);
}


// --- Finalize and send the response (compute checksum, start TX) ---
static void _txSendResponse()
{
  // Compute XOR checksum of everything between '$' (exclusive) and '*' (exclusive)
  uint8_t checksum = 0;
  for (uint8_t i = 1; i < _txLength; i++)
  {
    checksum ^= _txBuffer[i];
  }

  _txAppendChar('*');
  _txAppendChar(_toHex((checksum >> 4) & 0x0F));
  _txAppendChar(_toHex(checksum & 0x0F));
  _txAppendChar('\n');

  // Start TX on all output USARTs
  for (uint8_t i = 0; i < cTxUsartCount; i++)
  {
    _txIndex[i] = 0;
    _txActive[i] = true;
    // Write first byte and enable TXE interrupt
    _txUsartPtrs[i]->writeByte(_txBuffer[0]);
    _txIndex[i] = 1;
    _txUsartPtrs[i]->enableTxInterrupt();
  }
}


// --- Check if TX is still in progress on any USART ---
static bool _txBusy()
{
  for (uint8_t i = 0; i < cTxUsartCount; i++)
  {
    if (_txActive[i]) return true;
  }
  return false;
}


// --- Helper: send current page/mode status as response ---
static void _txSendPageStatus()
{
  _txBeginResponse("P");
  _txAppendDecimal(static_cast<uint8_t>(Application::getOperatingMode()), 1);
  auto vm = static_cast<uint8_t>(Application::getViewMode());
  if (vm > 0)
  {
    _txAppendChar('P');
    _txAppendDecimal(vm, 1);
  }
  _txSendResponse();
}


// --- Helper: send current ADC values as notification ---
static void _txSendAdcStatus()
{
  _txBeginResponse("HADC");
  _txAppendDecimal(_prevAdcLight, 1);
  _txAppendChar(',');
  _txAppendDecimal(_prevAdcVddA, 1);
  _txAppendChar(',');
  _txAppendDecimal(_prevAdcVbatt, 1);
  _txSendResponse();
}


// --- Helper: send all temperature sensor values as response/notification ---
static void _txSendTempStatus()
{
  _txBeginResponse("M");
  for (uint8_t i = 0; i < Hardware::cTempSensorCount; i++)
  {
    if (i > 0) _txAppendChar(',');
    _txAppendDecimal(Hardware::temperatureCx10(static_cast<Hardware::TempSensorType>(i)), 1);
  }
  _txSendResponse();
}


// --- Helper: append formatted date/time to TX buffer ---
static void _txAppendDateTime(const DateTime &dt)
{
  _txAppendDecimal(dt.hour(), 2);
  _txAppendDecimal(dt.minute(), 2);
  _txAppendDecimal(dt.second(), 2);
  _txAppendDecimal(dt.year(), 4);
  _txAppendDecimal(dt.month(), 2);
  _txAppendDecimal(dt.day(), 2);
}


// --- Parse a decimal number from the payload, advancing the index ---
static int32_t _parseDecimal(const char* str, uint8_t &index, uint8_t maxLen)
{
  int32_t value = 0;
  bool negative = false;

  if (index < maxLen && str[index] == '-')
  {
    negative = true;
    index++;
  }

  while (index < maxLen && str[index] >= '0' && str[index] <= '9')
  {
    value = value * 10 + (str[index] - '0');
    index++;
  }

  return negative ? -value : value;
}


// --- Forward declaration for command handler ---
static void _handleCommand(const char* payload, uint8_t length);


void initialize()
{
  _txUsartPtrs[0] = Hardware::getUsart(0);  // USART1
  _txUsartPtrs[1] = Hardware::getUsart(3);  // USART4
  _rxUsart3Ptr    = Hardware::getUsart(2);  // USART3
}


void rxIsr(uint32_t usart)
{
  // Look up the Usart instance for this peripheral
  Usart* u = (usart == Hardware::cGpsUsart) ? _txUsartPtrs[0] : _rxUsart3Ptr;
  if (u == nullptr) return;

  // Check if the RXNE flag is set for this USART
  if (!u->rxReady())
  {
    return;
  }

  // Read the received byte (clears RXNE flag)
  char rxChar = u->readByte();

  switch (_rxState)
  {
    case RxState::RxGettingHeader:
      if (rxChar == cHeader[_rxHeaderIndex])
      {
        _rxChecksum ^= rxChar;
        if (++_rxHeaderIndex >= cHeaderLength)
        {
          // "$TC" matched -- start collecting payload
          _rxState = RxState::RxGettingPayload;
          _rxPayloadIndex = 0;
        }
      }
      else
      {
        // Mismatch -- back to idle
        _rxState = RxState::RxIdle;
        _rxHeaderIndex = 0;
      }
      break;

    case RxState::RxGettingPayload:
      if (rxChar == '*')
      {
        // Checksum marker -- start reading checksum
        _rxState = RxState::RxGettingChecksum1;
      }
      else if (rxChar == '\n' || rxChar == '\r')
      {
        // End of sentence without checksum -- accept anyway for simplicity
        _rxPayload[_rxFillBuffer][_rxPayloadIndex] = '\0';
        _rxPayloadLength[_rxFillBuffer] = _rxPayloadIndex;

        if (!_rxCommandReady)
        {
          _rxFillBuffer ^= 1;  // swap buffers
          _rxCommandReady = true;
        }
        _rxState = RxState::RxIdle;
      }
      else if (_rxPayloadIndex < cMaxPayloadLength - 1)
      {
        _rxPayload[_rxFillBuffer][_rxPayloadIndex++] = rxChar;
        _rxChecksum ^= rxChar;
      }
      else
      {
        // Payload too long -- discard
        _rxState = RxState::RxIdle;
      }
      break;

    case RxState::RxGettingChecksum1:
    {
      uint8_t nibble = _fromHex(rxChar);
      if (nibble != 0xFF)
      {
        _rxReceivedChecksum = nibble << 4;
        _rxState = RxState::RxGettingChecksum2;
      }
      else
      {
        _rxState = RxState::RxIdle;
      }
      break;
    }

    case RxState::RxGettingChecksum2:
    {
      uint8_t nibble = _fromHex(rxChar);
      if (nibble != 0xFF)
      {
        _rxReceivedChecksum |= nibble;
        if (_rxReceivedChecksum == _rxChecksum)
        {
          // Valid checksum -- command is ready
          _rxPayload[_rxFillBuffer][_rxPayloadIndex] = '\0';
          _rxPayloadLength[_rxFillBuffer] = _rxPayloadIndex;

          if (!_rxCommandReady)
          {
            _rxFillBuffer ^= 1;
            _rxCommandReady = true;
          }
        }
        // else: checksum mismatch, silently discard
      }
      _rxState = RxState::RxIdle;
      break;
    }

    default:  // RxIdle
      if (rxChar == '$')
      {
        // Start of a new sentence
        _rxHeaderIndex = 1;  // we've already matched '$'
        _rxChecksum = 0;
        _rxReceivedChecksum = 0;
        _rxState = RxState::RxGettingHeader;
      }
      break;
  }
}


void txIsr(uint32_t usart)
{
  uint8_t idx = _txUsartIndex(usart);
  Usart* u = _txUsartPtrs[idx];
  if (u == nullptr) return;

  // Check if the TXE flag is set for this USART
  if (!u->txReady())
  {
    return;
  }

  if (_txActive[idx] && _txIndex[idx] < _txLength)
  {
    u->writeByte(_txBuffer[_txIndex[idx]++]);
  }
  else
  {
    // Done transmitting -- disable TXE interrupt
    u->disableTxInterrupt();
    _txActive[idx] = false;
  }
}


bool hasCommand()
{
  return _rxCommandReady;
}


void process()
{
  // Check for intensity changes (auto-brightness)
  if (Application::getIntensityAutoAdjust())
  {
    uint8_t intensity = Application::getIntensity();
    if (intensity != _prevIntensity)
    {
      _prevIntensity = intensity;
      _pendingIntensityNotification = true;
    }
  }

  // Check for temperature sensor updates
  if (Hardware::temperatureUpdated())
  {
    for (uint8_t i = 0; i < Hardware::cTempSensorCount; i++)
    {
      int32_t current = Hardware::temperatureCx10(static_cast<Hardware::TempSensorType>(i));
      if (current != _prevTempValues[i])
      {
        _prevTempValues[i] = current;
        _pendingTempNotification = true;
      }
    }
  }

  // Check for ADC value changes
  if (Hardware::adcValuesUpdated())
  {
    uint16_t light = Hardware::lightLevel();
    uint16_t vdda = Hardware::voltageVddA();
    uint16_t vbatt = Hardware::voltageBatt();

    if (light != _prevAdcLight || vdda != _prevAdcVddA || vbatt != _prevAdcVbatt)
    {
      _prevAdcLight = light;
      _prevAdcVddA = vdda;
      _prevAdcVbatt = vbatt;
      _pendingAdcNotification = true;
    }
  }

  // Send deferred notifications when TX is idle (mode > key > intensity > temp > ADC)
  if (!_txBusy())
  {
    if (_pendingModeNotification)
    {
      _pendingModeNotification = false;
      _txSendPageStatus();
    }
    else if (_pendingKeyNotification)
    {
      _pendingKeyNotification = false;
      _txBeginResponse("K");
      _txAppendDecimal(_pendingKeyMask, 1);
      _txAppendChar(',');
      _txAppendChar(_pendingKeyPressed ? '1' : '0');
      _txSendResponse();
    }
    else if (_pendingIntensityNotification)
    {
      _pendingIntensityNotification = false;
      _txBeginResponse("I");
      _txAppendDecimal(_prevIntensity, 1);
      _txAppendChar(',');
      _txAppendChar(Application::getIntensityAutoAdjust() ? '1' : '0');
      _txSendResponse();
    }
    else if (_pendingTempNotification)
    {
      _pendingTempNotification = false;
      _txSendTempStatus();
    }
    else if (_pendingAdcNotification)
    {
      _pendingAdcNotification = false;
      _txSendAdcStatus();
    }
  }

  if (!_rxCommandReady)
  {
    return;
  }

  // The completed command is in the non-fill buffer
  uint8_t processBuffer = _rxFillBuffer ^ 1;
  _handleCommand(_rxPayload[processBuffer], _rxPayloadLength[processBuffer]);

  _rxCommandReady = false;
}


void notifyModeChange(uint8_t mode, uint8_t viewMode)
{
  (void)mode;
  (void)viewMode;
  if (_suppressNotifications) return;
  if (_txBusy())
  {
    _pendingModeNotification = true;
    return;
  }
  _txSendPageStatus();
}


void notifyKeyEvent(uint8_t keyMask, bool pressed)
{
  if (_suppressNotifications) return;
  if (_txBusy())
  {
    _pendingKeyMask = keyMask;
    _pendingKeyPressed = pressed;
    _pendingKeyNotification = true;
    return;
  }

  _txBeginResponse("K");
  _txAppendDecimal(keyMask, 1);
  _txAppendChar(',');
  _txAppendChar(pressed ? '1' : '0');
  _txSendResponse();
}


// --- Command handler dispatch ---
static void _handleCommand(const char* payload, uint8_t length)
{
  if (length < 2) return;

  // First char after "$TC" is C (command) -- we only handle commands
  if (payload[0] != 'C') return;

  // Don't respond if TX is busy
  if (_txBusy()) return;

  switch (payload[1])
  {
    // --- Page/Mode ---
    case 'P':
    {
      if (length > 2)
      {
        // "$TCCP<nn>[P<m>]" -- set page (and optionally sub-page)
        uint8_t idx = 2;
        int32_t mode = _parseDecimal(payload, idx, length);

        uint8_t viewMode = 0;
        bool hasViewMode = false;
        if (idx < length && payload[idx] == 'P')
        {
          idx++;
          viewMode = _parseDecimal(payload, idx, length);
          hasViewMode = true;
        }

        if (mode >= 0 && mode <= cMaxOperatingMode)
        {
          _suppressNotifications = true;
          Application::setOperatingMode(static_cast<Application::OperatingMode>(mode));
          if (hasViewMode)
          {
            Application::setViewMode(static_cast<ViewMode>(viewMode));
          }
          _suppressNotifications = false;
        }
      }
      // Always respond with current page status (handles both get and set)
      _txSendPageStatus();
      break;
    }

    // --- Keys ---
    case 'K':
    {
      _txBeginResponse("K");
      _txAppendDecimal(Hardware::buttons(), 1);
      _txSendResponse();
      break;
    }

    // --- Hardware ---
    case 'H':
    {
      if (length < 5) break;

      char s0 = payload[2], s1 = payload[3], s2 = payload[4];

      if (s0 == 'A' && s1 == 'D' && s2 == 'C')
      {
        // "$TCCHADC" -- ADC data: light level, VddA, VBatt
        _txBeginResponse("HADC");
        _txAppendDecimal(Hardware::lightLevel(), 1);
        _txAppendChar(',');
        _txAppendDecimal(Hardware::voltageVddA(), 1);
        _txAppendChar(',');
        _txAppendDecimal(Hardware::voltageBatt(), 1);
        _txSendResponse();
      }
      else if (s0 == 'C' && s1 == 'O' && s2 == 'N')
      {
        // "$TCCHCON" -- connected components bitmask
        _txBeginResponse("H");
        uint8_t connected = 0;
        if (Hardware::getTempSensorType() == Hardware::TempSensorType::DS3234)  connected |= (1 << 0);
        if (Hardware::getTempSensorType() == Hardware::TempSensorType::DS1722)  connected |= (1 << 1);
        if (Hardware::getTempSensorType() == Hardware::TempSensorType::LM74)    connected |= (1 << 2);
        if (GpsReceiver::isConnected())                                         connected |= (1 << 3);
        if (GpsReceiver::isValid())                                             connected |= (1 << 4);
        _txAppendDecimal(connected, 1);
        _txSendResponse();
      }
      else if (s0 == 'V' && s1 == 'O')
      {
        // "$TCCHVON" / "$TCCHVOF" -- HV power on/off
        bool hvOn = (s2 == 'N');
        Hardware::setHvState(hvOn);
        _txBeginResponse("HV");
        _txAppendChar(hvOn ? '1' : '0');
        _txSendResponse();
      }
      break;
    }

    // --- Time ---
    case 'T':
    {
      if (length >= 16)
      {
        // "$TCCT<HHMMSSYYYYMMDD>" -- set time (14 digits starting at index 2)
        uint8_t idx = 2;
        int32_t hour   = _parseDecimal(payload, idx, idx + 2);
        int32_t minute = _parseDecimal(payload, idx, idx + 2);
        int32_t second = _parseDecimal(payload, idx, idx + 2);
        int32_t year   = _parseDecimal(payload, idx, idx + 4);
        int32_t month  = _parseDecimal(payload, idx, idx + 2);
        int32_t day    = _parseDecimal(payload, idx, idx + 2);

        DateTime dt;
        dt.setTime(hour, minute, second);
        dt.setDate(year, month, day);
        Application::setDateTime(dt);
      }

      // Always respond with current time (handles both get and set)
      {
        DateTime dt = Application::dateTime();
        _txBeginResponse("T");
        _txAppendDateTime(dt);
        _txSendResponse();
      }
      break;
    }

    // --- Temperature ---
    case 'M':
    {
      if (length >= 3 && payload[2] == 'S')
      {
        // "$TCCMS" -- get/set temperature source
        if (length > 3)
        {
          uint8_t idx = 3;
          int32_t source = _parseDecimal(payload, idx, length);
          if (!Hardware::setTempSensorType(static_cast<Hardware::TempSensorType>(source)))
          {
            _txBeginResponse("MSE");
            _txSendResponse();
            break;
          }
        }
        _txBeginResponse("MS");
        _txAppendDecimal(Hardware::getTempSensorType(), 1);
        _txSendResponse();
      }
      else
      {
        // "$TCCM" / "$TCCM<value>" -- get/set temperature value
        if (length > 2)
        {
          uint8_t idx = 2;
          int32_t temp = _parseDecimal(payload, idx, length);
          Hardware::setTemperature(temp);
        }
        _txSendTempStatus();
      }
      break;
    }

    // --- Intensity ---
    case 'I':
    {
      if (length >= 3 && payload[2] == 'A')
      {
        // "$TCCIA<0|1>" -- set auto-adjust
        if (length >= 4)
        {
          Application::setIntensityAutoAdjust(payload[3] == '1', false);
        }
      }
      else if (length > 2)
      {
        // "$TCCI<nnn>[,<0|1>]" -- set intensity, optionally set auto-adjust
        uint8_t idx = 2;
        int32_t intensity = _parseDecimal(payload, idx, length);
        if (intensity >= 0 && intensity <= 255)
        {
          Application::setIntensity(static_cast<uint8_t>(intensity));
        }
        if (idx < length && payload[idx] == ',')
        {
          idx++;
          if (idx < length)
          {
            Application::setIntensityAutoAdjust(payload[idx] == '1', false);
          }
        }
      }

      // Always respond with current intensity state
      _txBeginResponse("I");
      _txAppendDecimal(Application::getIntensity(), 1);
      _txAppendChar(',');
      _txAppendChar(Application::getIntensityAutoAdjust() ? '1' : '0');
      _txSendResponse();
      break;
    }

    // --- Settings ---
    case 'S':
    {
      if (length < 3) break;

      uint8_t idx = 2;
      int32_t settingNum = _parseDecimal(payload, idx, length);

      if (settingNum < 0 || settingNum > Settings::Setting::DmxAddress) break;

      if (idx < length && payload[idx] == ',')
      {
        // "$TCCS<nn>,<value>" -- set setting
        idx++;  // skip comma
        int32_t value = _parseDecimal(payload, idx, length);
        Application::getSettingsPtr()->setRawSetting(
          static_cast<uint8_t>(settingNum), static_cast<uint16_t>(value));
        Application::refreshSettings();
      }

      // Respond with current value (whether we just set it or not)
      _txBeginResponse("S");
      _txAppendDecimal(settingNum, 1);
      _txAppendChar(',');
      _txAppendDecimal(
        Application::getSettingsPtr()->getRawSetting(static_cast<uint8_t>(settingNum)), 1);
      _txSendResponse();
      break;
    }

    default:
      break;
  }
}


}

}
