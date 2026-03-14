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

#include "AlarmHandler.h"
#include "Application.h"
#include "BuildInfo.h"
#include "DateTime.h"
#include "DisplayManager.h"
#include "GpsReceiver.h"
#include "Hardware.h"
#include "RgbLed.h"
#include "RtttlPlayer.h"
#include "SerialRemote.h"
#include "Settings.h"
#include "UsbSerial.h"
#include "TimerCounterView.h"

#if HARDWARE_VERSION == 1
#include "Hardware_v1.h"
#else
#error HARDWARE_VERSION must be defined with a value of 1
#endif

namespace kbxTubeClock::SerialRemote {

// RX parser state machine
//
enum RxState : uint8_t {
  RxIdle,
  RxGettingHeader,
  RxGettingPayload,
  RxGettingChecksum1,
  RxGettingChecksum2,
};

// Maximum payload length (between "$TC" header and "*" checksum marker).
// 255 bytes accommodates more RTTTL strings (see 'B' command handler).
//
static const uint8_t cMaxPayloadLength = 255;

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

// TX notification/response queue capacity
//
static const uint8_t cQueueSize = 8;

// Maximum valid OperatingMode value (OperatingModeTestDisplay)
//
static const uint8_t cMaxOperatingMode = 41;

// --- RX state ---

// Double-buffered: ISR fills one, main loop processes the other
static char _rxPayload[2][cMaxPayloadLength];
static uint8_t _rxPayloadLength[2] = {0, 0};
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
static volatile uint8_t _txIndex[cTxUsartCount] = {0, 0};
static volatile bool _txActive[cTxUsartCount] = {false, false};

// Map USART peripheral to TX index: USART1 -> 0, USART4 -> 1
static uint8_t _txUsartIndex(uint32_t usart) { return (usart == Hardware::cGpsUsart) ? 0 : 1; }

// Cached Usart pointers for TX (USART1, USART4) and RX (USART3)
static Usart *_txUsartPtrs[cTxUsartCount] = {nullptr, nullptr};
static Usart *_rxUsart3Ptr = nullptr;

// Suppresses unsolicited notifications during command processing so the
// command handler can send a single authoritative response with final state
static bool _suppressNotifications = false;

// Set when HBOOT command is received; reset is deferred until TX is idle
static bool _bootloaderResetPending = false;

// --- TX notification/response queue ---
//
// Each entry stores a message type and any data needed to format the message at
// dequeue time. _txBuffer is only written when a message is actually ready to
// transmit, so the queue never races with an in-flight transfer.
//
// A queue entry is popped only when !_txBusy() -- i.e. after BOTH USARTs have
// finished transmitting the previous message.
//
// Unsolicited notification types are coalesced (at most one per type in the
// queue at a time). Command response types are always enqueued as-is.
//
enum class NotificationType : uint8_t {
  None = 0,  // Placeholder / cancelled entry; skipped on dequeue

  // Unsolicited notifications (coalesced)
  Mode,       // Current operating mode + view mode
  Intensity,  // Current display intensity + auto-adjust flag
  Led,        // Current status LED color
  Temp,       // All temperature sensor values
  Adc,        // ADC: light level, VddA, VBatt
  RtttlDone,  // RTTTL playback completed
  Boot,       // Sent once at startup before any other notifications

  // Key events (not coalesced; each press/release is a distinct event)
  KeyEvent,

  // Command responses (formatted with live state at dequeue time)
  CmdKeys,             // Current raw button bitmask (K command)
  CmdHvState,          // HV power on/off state (H command)
  CmdConnected,        // Connected-peripherals bitmask (H/CON command)
  CmdTime,             // Current date/time (T command)
  CmdTempSource,       // Active temperature sensor type (M/S command)
  CmdTempSourceError,  // Temperature sensor type change error (M/S command)
  CmdSettingValue,     // Single setting number + value (S command)
  CmdSettingSaved,     // Flash save result (S/W command)
  CmdSettingsErased,   // Flash erase result (S/ERASE command)
  CmdBuzzerStatus,     // RTTTL play/stop state (B command)
  CmdBootloader,       // HBOOT0/1/2 command acknowledgement
  // --- Timer/alarm ---
  CmdTimerStatus,  // R command response: timer state + value
  CmdAlarmClear,   // RA command response: alarm clear acknowledgement
  AlarmActive,     // Unsolicited: any alarm went active (rising edge)

  // Diagnostics command responses (D command)
  CmdDiagFirmware,      // Firmware version + build number
  CmdDiagOnTime,        // HV on-time counter (tube lifetime)
  CmdDiagOnTimeReset,   // Reset HV on-time counter (acknowledgement)
  CmdDiagRtc,           // Active RTC type + startup result
  CmdDiagSettingsLoad,  // Settings load source at boot
  CmdDiagGps,           // GPS connection/fix status + satellite count
};

struct QueueEntry {
  NotificationType type;
  union {
    uint8_t keyMask;     // KeyEvent
    uint8_t settingNum;  // CmdSettingValue
    bool saveOk;         // CmdSettingSaved
    uint8_t bootSubCmd;  // CmdBootloader (0/1/2)
    bool wasActive;      // CmdAlarmClear
  } data;
};

static QueueEntry _txQueue[cQueueSize];
static uint8_t _txQueueHead = 0;
static uint8_t _txQueueTail = 0;

// --- Change-detection state for unsolicited notifications ---

static uint8_t _prevIntensity = 0;

static int32_t _prevTempValues[Hardware::cTempSensorCount] = {Hardware::cTempSentinel, Hardware::cTempSentinel,
                                                              Hardware::cTempSentinel, Hardware::cTempSentinel,
                                                              Hardware::cTempSentinel};

static uint16_t _prevAdcLight = 0;
static uint16_t _prevAdcVddA = 0;
static uint16_t _prevAdcVbatt = 0;
static RgbLed _prevLed;
static bool _prevLedAutoAdjust = false;
static bool _prevAlarmActive = false;
static bool _prevGpsConnected = false;
static bool _prevGpsValid = false;
static uint8_t _prevGpsSats = 0;

// --- Helper: convert nibble to hex char ---
static char _toHex(uint8_t nibble) { return (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10); }

// --- Helper: convert hex char to nibble ---
static uint8_t _fromHex(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  return 0xFF;  // invalid
}

// --- Helper: append a single char to the TX buffer ---
static void _txAppendChar(char c) {
  if (_txLength < cTxBufferSize - 1) {
    _txBuffer[_txLength++] = c;
  }
}

// --- Helper: append a null-terminated string to the TX buffer ---
static void _txAppendString(const char *str) {
  while (*str) {
    _txAppendChar(*str++);
  }
}

// --- Helper: append a decimal number to the TX buffer ---
static void _txAppendDecimal(int32_t value, uint8_t minDigits) {
  if (value < 0) {
    _txAppendChar('-');
    value = -value;
  }

  char digits[11];
  uint8_t count = 0;
  do {
    digits[count++] = '0' + (value % 10);
    value /= 10;
  } while (value > 0);

  while (count < minDigits) {
    digits[count++] = '0';
  }

  for (uint8_t i = count; i > 0; i--) {
    _txAppendChar(digits[i - 1]);
  }
}

// --- Start building a response in the TX buffer with category prefix ---
static void _txBeginResponse(const char *prefix) {
  _txLength = 4;
  _txBuffer[0] = '$';
  _txBuffer[1] = 'T';
  _txBuffer[2] = 'C';
  _txBuffer[3] = 'S';
  _txAppendString(prefix);
}

// --- Finalize and send the response (compute checksum, start TX) ---
static void _txSendResponse() {
  // Compute XOR checksum of everything between '$' (exclusive) and '*' (exclusive)
  uint8_t checksum = 0;
  for (uint8_t i = 1; i < _txLength; i++) {
    checksum ^= _txBuffer[i];
  }

  _txAppendChar('*');
  _txAppendChar(_toHex((checksum >> 4) & 0x0F));
  _txAppendChar(_toHex(checksum & 0x0F));
  _txAppendChar('\n');

  // Start TX on all output USARTs
  for (uint8_t i = 0; i < cTxUsartCount; i++) {
    _txIndex[i] = 0;
    _txActive[i] = true;
    // Write first byte and enable TXE interrupt
    _txUsartPtrs[i]->writeByte(_txBuffer[0]);
    _txIndex[i] = 1;
    _txUsartPtrs[i]->enableTxInterrupt();
  }

  // Also send via USB if the host is connected
  UsbSerial::write(reinterpret_cast<const uint8_t *>(_txBuffer), _txLength);
}

// --- Check if TX is still in progress on any USART ---
static bool _txBusy() {
  for (uint8_t i = 0; i < cTxUsartCount; i++) {
    if (_txActive[i]) {
      return true;
    }
  }
  return false;
}

// --- Helper: send current page/mode status ---
static void _txSendPageStatus() {
  _txBeginResponse("P");
  _txAppendDecimal(static_cast<uint8_t>(Application::getOperatingMode()), 1);
  auto vm = static_cast<uint8_t>(Application::getViewMode());
  if (vm > 0) {
    _txAppendChar('P');
    _txAppendDecimal(vm, 1);
  }
  _txSendResponse();
}

// --- Helper: send current ADC values ---
static void _txSendAdcStatus() {
  _txBeginResponse("HADC");
  _txAppendDecimal(Hardware::lightLevelRaw(), 1);
  _txAppendChar(',');
  _txAppendDecimal(Hardware::voltageVddARaw(), 1);
  _txAppendChar(',');
  _txAppendDecimal(Hardware::voltageBattRaw(), 1);
  _txSendResponse();
}

// --- Helper: send all temperature sensor values ---
static void _txSendTempStatus() {
  _txBeginResponse("M");
  for (uint8_t i = 0; i < Hardware::cTempSensorCount; i++) {
    if (i > 0) {
      _txAppendChar(',');
    }
    _txAppendDecimal(Hardware::temperatureCx10(static_cast<Hardware::TempSensorType>(i)), 1);
  }
  _txSendResponse();
}

// --- Helper: send current status LED values ---
static void _txSendLedStatus() {
  RgbLed led = DisplayManager::getStatusLed();
  _txBeginResponse("L");
  _txAppendDecimal(led.getIntensity(), 1);
  _txAppendChar(',');
  _txAppendDecimal(led.getRed() / 16, 1);
  _txAppendChar(',');
  _txAppendDecimal(led.getGreen() / 16, 1);
  _txAppendChar(',');
  _txAppendDecimal(led.getBlue() / 16, 1);
  _txAppendChar(',');
  _txAppendChar(Application::getLedIntensityAutoAdjust() ? '1' : '0');
  _txSendResponse();
}

// --- Diagnostics (D category) response helpers ---

static void _txSendDiagFirmware() {
  _txBeginResponse("DF");
  _txAppendString(kFirmwareVersion);
  _txAppendChar(',');
  _txAppendDecimal(kFirmwareBuild, 1);
  _txSendResponse();
}

static void _txSendDiagOnTime() {
  _txBeginResponse("DOT");
  _txAppendDecimal(Hardware::onTimeSeconds(), 1);
  _txSendResponse();
}

static void _txSendDiagOnTimeReset() {
  Hardware::onTimeSecondsReset();
  _txBeginResponse("DOTR");
  _txSendResponse();
}

static void _txSendDiagRtc() {
  _txBeginResponse("DRTC");
  _txAppendDecimal(static_cast<uint8_t>(Hardware::getRTCType()), 1);
  _txAppendChar(',');
  _txAppendDecimal(static_cast<uint8_t>(Hardware::getRTCStartupResult()), 1);
  _txSendResponse();
}

static void _txSendDiagSettingsLoad() {
  _txBeginResponse("DS");
  _txAppendDecimal(Application::getStartupSettingsLoadResult(), 1);
  _txSendResponse();
}

static void _txSendDiagGps() {
  _txBeginResponse("DGPS");
  _txAppendDecimal(GpsReceiver::isConnected() ? 1 : 0, 1);
  _txAppendChar(',');
  _txAppendDecimal(GpsReceiver::isValid() ? 1 : 0, 1);
  _txAppendChar(',');
  _txAppendDecimal(GpsReceiver::getSatellitesInView(), 1);
  _txSendResponse();
}

// --- Helper: append formatted date/time to TX buffer ---
static void _txAppendDateTime(const DateTime &dt) {
  _txAppendDecimal(dt.hour(), 2);
  _txAppendDecimal(dt.minute(), 2);
  _txAppendDecimal(dt.second(), 2);
  _txAppendDecimal(dt.year(), 4);
  _txAppendDecimal(dt.month(), 2);
  _txAppendDecimal(dt.day(), 2);
}

// --- Parse a decimal number from the payload, advancing the index ---
static int32_t _parseDecimal(const char *str, uint8_t &index, uint8_t maxLen) {
  int32_t value = 0;
  bool negative = false;

  if (index < maxLen && str[index] == '-') {
    negative = true;
    index++;
  }

  while (index < maxLen && str[index] >= '0' && str[index] <= '9') {
    value = value * 10 + (str[index] - '0');
    index++;
  }

  return negative ? -value : value;
}

// --- Queue helpers ---

static bool _queueEmpty() { return _txQueueHead == _txQueueTail; }

static bool _queueFull() { return ((_txQueueTail + 1) % cQueueSize) == _txQueueHead; }

// Returns true if a non-None entry of the given type is already in the queue
static bool _queueContains(NotificationType type) {
  uint8_t i = _txQueueHead;
  while (i != _txQueueTail) {
    if (_txQueue[i].type == type) {
      return true;
    }
    i = (i + 1) % cQueueSize;
  }
  return false;
}

// Appends an entry; drops the new entry (no-op) if the queue is full
static bool _queueEnqueue(const QueueEntry &entry) {
  if (_queueFull()) {
    return false;
  }
  _txQueue[_txQueueTail] = entry;
  _txQueueTail = (_txQueueTail + 1) % cQueueSize;
  return true;
}

// Enqueues a coalescing notification: skipped if type is already queued or
// notifications are currently suppressed
static void _enqueueNotification(NotificationType type) {
  if (_suppressNotifications) {
    return;
  }
  if (!_queueContains(type)) {
    QueueEntry e;
    e.type = type;
    _queueEnqueue(e);
  }
}

// Marks all queued entries of the given type as None (effectively cancels them)
static void _queueCancelType(NotificationType type) {
  uint8_t i = _txQueueHead;
  while (i != _txQueueTail) {
    if (_txQueue[i].type == type) {
      _txQueue[i].type = NotificationType::None;
    }
    i = (i + 1) % cQueueSize;
  }
}

// Sends the next queued message if TX is idle. None entries at the head are
// skipped (advanced past) without transmitting. Called from process().
static void _txProcessQueue() {
  // Skip any None (cancelled) entries at the head
  while (!_queueEmpty() && _txQueue[_txQueueHead].type == NotificationType::None) {
    _txQueueHead = (_txQueueHead + 1) % cQueueSize;
  }

  if (_queueEmpty() || _txBusy()) {
    return;
  }

  // Pop the head entry
  QueueEntry entry = _txQueue[_txQueueHead];
  _txQueueHead = (_txQueueHead + 1) % cQueueSize;

  switch (entry.type) {
    case NotificationType::Mode:
      _txSendPageStatus();
      break;

    case NotificationType::KeyEvent:
      _txBeginResponse("K");
      _txAppendDecimal(entry.data.keyMask, 1);
      _txSendResponse();
      break;

    case NotificationType::Intensity:
      _txBeginResponse("I");
      _txAppendDecimal(Application::getIntensity(), 1);
      _txAppendChar(',');
      _txAppendChar(Application::getIntensityAutoAdjust() ? '1' : '0');
      _txSendResponse();
      break;

    case NotificationType::Led:
      _txSendLedStatus();
      break;

    case NotificationType::Temp:
      _txSendTempStatus();
      break;

    case NotificationType::Adc:
      _txSendAdcStatus();
      break;

    case NotificationType::RtttlDone:
      _txBeginResponse("BOK");
      _txSendResponse();
      break;

    case NotificationType::Boot:
      _txBeginResponse("BOOT");
      _txAppendString(kFirmwareVersion);
      _txAppendChar(',');
      _txAppendDecimal(kFirmwareBuild, 1);
      _txSendResponse();
      break;

    case NotificationType::CmdKeys:
      _txBeginResponse("K");
      _txAppendDecimal(Hardware::buttons(), 1);
      _txSendResponse();
      break;

    case NotificationType::CmdHvState:
      _txBeginResponse("HV");
      _txAppendChar(Hardware::getHvState() ? '1' : '0');
      _txSendResponse();
      break;

    case NotificationType::CmdConnected: {
      _txBeginResponse("HCON");
      uint8_t connected = 0;
      if (Hardware::getTempSensorType() == Hardware::TempSensorType::DS3234) {
        connected |= (1 << 0);
      }
      if (Hardware::getTempSensorType() == Hardware::TempSensorType::DS1722) {
        connected |= (1 << 1);
      }
      if (Hardware::getTempSensorType() == Hardware::TempSensorType::LM74) {
        connected |= (1 << 2);
      }
      if (GpsReceiver::isConnected()) {
        connected |= (1 << 3);
      }
      if (GpsReceiver::isValid()) {
        connected |= (1 << 4);
      }
      _txAppendDecimal(connected, 1);
      _txSendResponse();
      break;
    }

    case NotificationType::CmdTime: {
      DateTime dt = Application::dateTime();
      _txBeginResponse("T");
      _txAppendDateTime(dt);
      _txSendResponse();
      break;
    }

    case NotificationType::CmdTempSource:
      _txBeginResponse("MS");
      _txAppendDecimal(Hardware::getTempSensorType(), 1);
      _txSendResponse();
      break;

    case NotificationType::CmdTempSourceError:
      _txBeginResponse("MSE");
      _txSendResponse();
      break;

    case NotificationType::CmdSettingValue:
      _txBeginResponse("S");
      _txAppendDecimal(entry.data.settingNum, 1);
      _txAppendChar(',');
      _txAppendDecimal(Application::getSettingsPtr()->getRawSetting(entry.data.settingNum), 1);
      _txSendResponse();
      break;

    case NotificationType::CmdSettingSaved:
      _txBeginResponse("SW");
      _txAppendChar(entry.data.saveOk ? '1' : '0');
      _txSendResponse();
      break;

    case NotificationType::CmdSettingsErased:
      _txBeginResponse("SERASE");
      _txAppendChar(entry.data.saveOk ? '1' : '0');
      _txSendResponse();
      break;

    case NotificationType::CmdBuzzerStatus:
      _txBeginResponse("B");
      _txAppendChar(RtttlPlayer::isPlaying() ? 'P' : 'S');
      _txSendResponse();
      break;

    case NotificationType::CmdBootloader:
      _txBeginResponse("HBOOT");
      _txAppendChar(static_cast<char>('0' + entry.data.bootSubCmd));
      _txSendResponse();
      break;

    case NotificationType::CmdTimerStatus: {
      uint8_t vm = static_cast<uint8_t>(Application::getViewMode());
      char state = 'S';
      if (Application::getOperatingMode() == Application::OperatingMode::OperatingModeTimerCounter) {
        if (vm == 1) {
          state = 'U';
        } else if (vm == 2) {
          state = 'D';
        } else if (vm == 3) {
          state = 'R';
        }
      }
      _txBeginResponse("R");
      _txAppendChar(state);
      _txAppendChar(',');
      _txAppendDecimal(TimerCounterView::getTimerValue(), 1);
      _txSendResponse();
      break;
    }

    case NotificationType::CmdAlarmClear:
      _txBeginResponse("RA");
      _txAppendChar(entry.data.wasActive ? '1' : '0');
      _txSendResponse();
      break;

    case NotificationType::AlarmActive:
      _txBeginResponse("RALM");
      _txSendResponse();
      break;

    case NotificationType::CmdDiagFirmware:
      _txSendDiagFirmware();
      break;
    case NotificationType::CmdDiagOnTime:
      _txSendDiagOnTime();
      break;
    case NotificationType::CmdDiagOnTimeReset:
      _txSendDiagOnTimeReset();
      break;
    case NotificationType::CmdDiagRtc:
      _txSendDiagRtc();
      break;
    case NotificationType::CmdDiagSettingsLoad:
      _txSendDiagSettingsLoad();
      break;
    case NotificationType::CmdDiagGps:
      _txSendDiagGps();
      break;

    default:
      break;
  }
}

// --- Forward declaration for command handler ---
static void _handleCommand(const char *payload, uint8_t length);

// --- RX state machine: process a single received byte ---
static void _rxProcessByte(char rxChar) {
  switch (_rxState) {
    case RxState::RxGettingHeader:
      if (rxChar == cHeader[_rxHeaderIndex]) {
        _rxChecksum ^= rxChar;
        if (++_rxHeaderIndex >= cHeaderLength) {
          // "$TC" matched -- start collecting payload
          _rxState = RxState::RxGettingPayload;
          _rxPayloadIndex = 0;
        }
      } else {
        // Mismatch -- back to idle
        _rxState = RxState::RxIdle;
        _rxHeaderIndex = 0;
      }
      break;

    case RxState::RxGettingPayload:
      if (rxChar == '*') {
        // Checksum marker -- start reading checksum
        _rxState = RxState::RxGettingChecksum1;
      } else if (rxChar == '\n' || rxChar == '\r') {
        // End of sentence without checksum -- accept anyway for simplicity
        _rxPayload[_rxFillBuffer][_rxPayloadIndex] = '\0';
        _rxPayloadLength[_rxFillBuffer] = _rxPayloadIndex;

        if (!_rxCommandReady) {
          _rxFillBuffer ^= 1;  // swap buffers
          _rxCommandReady = true;
        }
        _rxState = RxState::RxIdle;
      } else if (_rxPayloadIndex < cMaxPayloadLength - 1) {
        _rxPayload[_rxFillBuffer][_rxPayloadIndex++] = rxChar;
        _rxChecksum ^= rxChar;
      } else {
        // Payload too long -- discard
        _rxState = RxState::RxIdle;
      }
      break;

    case RxState::RxGettingChecksum1: {
      uint8_t nibble = _fromHex(rxChar);
      if (nibble != 0xFF) {
        _rxReceivedChecksum = nibble << 4;
        _rxState = RxState::RxGettingChecksum2;
      } else {
        _rxState = RxState::RxIdle;
      }
      break;
    }

    case RxState::RxGettingChecksum2: {
      uint8_t nibble = _fromHex(rxChar);
      if (nibble != 0xFF) {
        _rxReceivedChecksum |= nibble;
        if (_rxReceivedChecksum == _rxChecksum) {
          // Valid checksum -- command is ready
          _rxPayload[_rxFillBuffer][_rxPayloadIndex] = '\0';
          _rxPayloadLength[_rxFillBuffer] = _rxPayloadIndex;

          if (!_rxCommandReady) {
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
      if (rxChar == '$') {
        // Start of a new sentence
        _rxHeaderIndex = 1;  // we've already matched '$'
        _rxChecksum = 0;
        _rxReceivedChecksum = 0;
        _rxState = RxState::RxGettingHeader;
      }
      break;
  }
}

void initialize() {
  _txUsartPtrs[0] = Hardware::getUsart(0);  // USART1
  _txUsartPtrs[1] = Hardware::getUsart(3);  // USART4
  _rxUsart3Ptr = Hardware::getUsart(2);     // USART3

  // Enable USART3 RX interrupt now that the ISR handler is ready.
  // (Must not be enabled earlier -- before _rxUsart3Ptr is set, rxIsr()
  //  would return without reading the byte, leaving RXNE asserted and
  //  locking the CPU in an infinite ISR tail-chain on Cortex-M0.)
  _rxUsart3Ptr->enableRxInterrupt();

  // Enqueue the boot notification as the very first queue entry so it is
  // transmitted before any other unsolicited notifications.
  QueueEntry bootEntry;
  bootEntry.type = NotificationType::Boot;
  _queueEnqueue(bootEntry);
}

void rxIsr(uint32_t usart) {
  // Look up the Usart instance for this peripheral
  Usart *u = (usart == Hardware::cGpsUsart) ? _txUsartPtrs[0] : _rxUsart3Ptr;
  if (u == nullptr) {
    return;
  }

  // Check if the RXNE flag is set for this USART
  if (!u->rxReady()) {
    return;
  }

  // Read the received byte (clears RXNE flag) and run the state machine
  _rxProcessByte(u->readByte());
}

void rxByte(char c) { _rxProcessByte(c); }

void txIsr(uint32_t usart) {
  uint8_t idx = _txUsartIndex(usart);
  Usart *u = _txUsartPtrs[idx];
  if (u == nullptr) {
    return;
  }

  // Check if the TXE flag is set for this USART
  if (!u->txReady()) {
    return;
  }

  if (_txActive[idx] && _txIndex[idx] < _txLength) {
    u->writeByte(_txBuffer[_txIndex[idx]++]);
  } else {
    // Done transmitting -- disable TXE interrupt
    u->disableTxInterrupt();
    _txActive[idx] = false;
  }
}

bool hasCommand() { return _rxCommandReady; }

void process() {
  // Check for intensity changes (auto-brightness)
  if (Application::getIntensityAutoAdjust()) {
    uint8_t intensity = Application::getIntensity();
    if (intensity != _prevIntensity) {
      _prevIntensity = intensity;
      _enqueueNotification(NotificationType::Intensity);
    }
  }

  // Check for temperature sensor updates
  if (Hardware::temperatureUpdated()) {
    bool changed = false;
    for (uint8_t i = 0; i < Hardware::cTempSensorCount; i++) {
      int32_t current = Hardware::temperatureCx10(static_cast<Hardware::TempSensorType>(i));
      if (current != _prevTempValues[i]) {
        _prevTempValues[i] = current;
        changed = true;
      }
    }
    if (changed) {
      _enqueueNotification(NotificationType::Temp);
    }
  }

  // Check for LED color/intensity changes or LED auto-adjust state changes
  {
    RgbLed currentLed = DisplayManager::getStatusLed();
    bool ledAutoAdjust = Application::getLedIntensityAutoAdjust();
    if (currentLed != _prevLed || ledAutoAdjust != _prevLedAutoAdjust) {
      _prevLed = currentLed;
      _prevLedAutoAdjust = ledAutoAdjust;
      _enqueueNotification(NotificationType::Led);
    }
  }

  // Check for ADC value changes
  if (Hardware::adcValuesUpdated()) {
    uint16_t light = Hardware::lightLevelRaw();
    uint16_t vdda = Hardware::voltageVddARaw();
    uint16_t vbatt = Hardware::voltageBattRaw();

    if (light != _prevAdcLight || vdda != _prevAdcVddA || vbatt != _prevAdcVbatt) {
      _prevAdcLight = light;
      _prevAdcVddA = vdda;
      _prevAdcVbatt = vbatt;
      _enqueueNotification(NotificationType::Adc);
    }
  }

  // Check for RTTTL playback completion
  if (RtttlPlayer::playingFinished()) {
    _enqueueNotification(NotificationType::RtttlDone);
  }

  // Check for GPS status changes
  {
    bool connected = GpsReceiver::isConnected();
    bool valid = GpsReceiver::isValid();
    uint8_t sats = GpsReceiver::getSatellitesInView();
    if (connected != _prevGpsConnected || valid != _prevGpsValid || sats != _prevGpsSats) {
      _prevGpsConnected = connected;
      _prevGpsValid = valid;
      _prevGpsSats = sats;
      _enqueueNotification(NotificationType::CmdDiagGps);
    }
  }

  // Check for alarm activation (rising edge only)
  {
    bool alarmActive = AlarmHandler::isAlarmActive();
    if (alarmActive && !_prevAlarmActive) {
      _enqueueNotification(NotificationType::AlarmActive);
    }
    _prevAlarmActive = alarmActive;
  }

  // Send next queued notification/response when TX is idle (both USARTs done)
  _txProcessQueue();

  // If a bootloader reset is pending, wait until the ACK has been fully
  // transmitted (queue empty + both USARTs idle) then trigger the system reset.
  if (_bootloaderResetPending && !_txBusy() && _queueEmpty()) {
    Hardware::systemReset();  // noreturn; flag already set by BOOT2 handler
  }

  if (!_rxCommandReady) {
    return;
  }

  // The completed command is in the non-fill buffer
  uint8_t processBuffer = _rxFillBuffer ^ 1;
  _handleCommand(_rxPayload[processBuffer], _rxPayloadLength[processBuffer]);

  _rxCommandReady = false;
}

void notifyModeChange(uint8_t mode, uint8_t viewMode) {
  (void) mode;
  (void) viewMode;
  _enqueueNotification(NotificationType::Mode);
}

void notifyKeyEvent(uint8_t keyMask) {
  if (_suppressNotifications) {
    return;
  }
  // KeyEvents are not coalesced -- each state change is a distinct event
  if (!_queueFull()) {
    QueueEntry e;
    e.type = NotificationType::KeyEvent;
    e.data.keyMask = keyMask;
    _queueEnqueue(e);
  }
}

void notifyHvStateChanged() { _enqueueNotification(NotificationType::CmdHvState); }

void notifySettingChanged(uint8_t settingNum) {
  // Reuse the CmdSettingValue path; not coalesced so each changed
  // setting gets its own notification even when multiple change at once.
  if (!_queueFull()) {
    QueueEntry e;
    e.type = NotificationType::CmdSettingValue;
    e.data.settingNum = settingNum;
    _queueEnqueue(e);
  }
}

// --- Command handler dispatch ---
static void _handleCommand(const char *payload, uint8_t length) {
  if (length < 2) {
    return;
  }

  // First char after "$TC" is C (command) -- we only handle commands
  if (payload[0] != 'C') {
    return;
  }

  switch (payload[1]) {
    // --- Page/Mode ---
    case 'P': {
      if (length > 2) {
        // "$TCCP<nn>[P<m>]" -- set page (and optionally sub-page)
        uint8_t idx = 2;
        int32_t mode = _parseDecimal(payload, idx, length);

        uint8_t viewMode = 0;
        bool hasViewMode = false;
        if (idx < length && payload[idx] == 'P') {
          idx++;
          viewMode = _parseDecimal(payload, idx, length);
          hasViewMode = true;
        }

        if (mode >= 0 && mode <= cMaxOperatingMode) {
          _suppressNotifications = true;
          Application::setOperatingMode(static_cast<Application::OperatingMode>(mode));
          if (hasViewMode) {
            Application::setViewMode(static_cast<ViewMode>(viewMode));
          }
          _suppressNotifications = false;
        }
      }
      // Always respond with current page status (handles both get and set).
      // Uses the coalescing path: if a Mode notification is already queued,
      // it will carry the authoritative post-set state when dequeued.
      _enqueueNotification(NotificationType::Mode);
      break;
    }

    // --- Keys ---
    case 'K': {
      QueueEntry e;
      e.type = NotificationType::CmdKeys;
      _queueEnqueue(e);
      break;
    }

    // --- Hardware ---
    case 'H': {
      // "$TCCHV" -- query HV state (length == 3, 'V' only after "CH")
      if (length == 3 && payload[2] == 'V') {
        QueueEntry e;
        e.type = NotificationType::CmdHvState;
        _queueEnqueue(e);
        break;
      }

      if (length < 5) {
        break;
      }

      char s0 = payload[2], s1 = payload[3], s2 = payload[4];

      if (s0 == 'A' && s1 == 'D' && s2 == 'C') {
        // "$TCCHADC" -- ADC data: same format as Adc notification
        _enqueueNotification(NotificationType::Adc);
      } else if (s0 == 'C' && s1 == 'O' && s2 == 'N') {
        // "$TCCHCON" -- connected components bitmask
        QueueEntry e;
        e.type = NotificationType::CmdConnected;
        _queueEnqueue(e);
      } else if (s0 == 'V' && s1 == 'O') {
        // "$TCCHVON" / "$TCCHVOF" -- HV power on/off
        Hardware::setHvState(s2 == 'N');
        QueueEntry e;
        e.type = NotificationType::CmdHvState;
        _queueEnqueue(e);
      } else if (s0 == 'B' && s1 == 'O' && s2 == 'O' && length >= 7 && payload[5] == 'T' &&
                 (payload[6] == '0' || payload[6] == '1' || payload[6] == '2')) {
        // "$TCCHBOOT0" -- disable bootloader on next boot (clear flag)
        // "$TCCHBOOT1" -- enable bootloader on next boot (set flag, no reset)
        // "$TCCHBOOT2" -- enter bootloader now (set flag, reset after ACK)
        uint8_t sub = static_cast<uint8_t>(payload[6] - '0');
        if (sub == 0) {
          Hardware::clearBootloaderFlag();
        } else {
          Hardware::setBootloaderFlag();
        }
        if (sub == 2) {
          _bootloaderResetPending = true;
        }
        QueueEntry e;
        e.type = NotificationType::CmdBootloader;
        e.data.bootSubCmd = sub;
        _queueEnqueue(e);
      }
      break;
    }

    // --- Time ---
    case 'T': {
      if (length >= 16) {
        // "$TCCT<HHMMSSYYYYMMDD>" -- set time (14 digits starting at index 2)
        uint8_t idx = 2;
        int32_t hour = _parseDecimal(payload, idx, idx + 2);
        int32_t minute = _parseDecimal(payload, idx, idx + 2);
        int32_t second = _parseDecimal(payload, idx, idx + 2);
        int32_t year = _parseDecimal(payload, idx, idx + 4);
        int32_t month = _parseDecimal(payload, idx, idx + 2);
        int32_t day = _parseDecimal(payload, idx, idx + 2);

        DateTime dt;
        dt.setTime(hour, minute, second);
        dt.setDate(year, month, day);
        Application::setDateTime(dt);
      }

      // Always respond with current time (handles both get and set)
      {
        QueueEntry e;
        e.type = NotificationType::CmdTime;
        _queueEnqueue(e);
      }
      break;
    }

    // --- Temperature ---
    case 'M': {
      if (length >= 3 && payload[2] == 'S') {
        // "$TCCMS" -- get/set temperature source
        if (length > 3) {
          uint8_t idx = 3;
          int32_t source = _parseDecimal(payload, idx, length);
          if (!Hardware::setTempSensorType(static_cast<Hardware::TempSensorType>(source))) {
            QueueEntry e;
            e.type = NotificationType::CmdTempSourceError;
            _queueEnqueue(e);
            break;
          }
        }
        QueueEntry e;
        e.type = NotificationType::CmdTempSource;
        _queueEnqueue(e);
      } else {
        // "$TCCM" / "$TCCM<value>" -- get/set temperature value
        if (length > 2) {
          uint8_t idx = 2;
          int32_t temp = _parseDecimal(payload, idx, length);
          Hardware::setTemperature(temp);
        }
        _enqueueNotification(NotificationType::Temp);
      }
      break;
    }

    // --- Intensity ---
    case 'I': {
      if (length >= 3 && payload[2] == 'A') {
        // "$TCCIA<0|1>" -- set auto-adjust
        if (length >= 4) {
          Application::setIntensityAutoAdjust(payload[3] == '1', false);
        }
      } else if (length > 2) {
        // "$TCCI<nnn>[,<0|1>]" -- set intensity, optionally set auto-adjust
        uint8_t idx = 2;
        int32_t intensity = _parseDecimal(payload, idx, length);
        if (intensity >= 0 && intensity <= 255) {
          Application::setIntensity(static_cast<uint8_t>(intensity));
        }
        if (idx < length && payload[idx] == ',') {
          idx++;
          if (idx < length) {
            Application::setIntensityAutoAdjust(payload[idx] == '1', false);
          }
        }
      }

      // Sync _prevIntensity so process() doesn't fire a redundant notification
      _prevIntensity = Application::getIntensity();
      // Always respond with current intensity state
      _enqueueNotification(NotificationType::Intensity);
      break;
    }

    // --- Settings ---
    case 'S': {
      if (length < 3) {
        break;
      }

      // "$TCCSW" -- save current settings to flash (skips write if unchanged)
      if (payload[2] == 'W') {
        bool ok = Application::saveSettingsToFlash();
        QueueEntry e;
        e.type = NotificationType::CmdSettingSaved;
        e.data.saveOk = ok;
        _queueEnqueue(e);
        break;
      }

      // "$TCCSERASE" -- factory reset: erase settings flash area
      if (length >= 7 && payload[2] == 'E' && payload[3] == 'R' && payload[4] == 'A' && payload[5] == 'S' &&
          payload[6] == 'E') {
        uint32_t result = Hardware::eraseFlash(Settings::cSettingsFlashAddress);
        Application::getSettingsPtr()->invalidateSram();
        QueueEntry e;
        e.type = NotificationType::CmdSettingsErased;
        e.data.saveOk = (result == 0);
        _queueEnqueue(e);
        break;
      }

      uint8_t idx = 2;
      int32_t settingNum = _parseDecimal(payload, idx, length);

      if (settingNum < 0 || settingNum > Settings::Setting::DmxAddress) {
        break;
      }

      if (idx < length && payload[idx] == ',') {
        // "$TCCS<nn>,<value>" -- set setting
        idx++;  // skip comma
        int32_t value = _parseDecimal(payload, idx, length);
        Application::getSettingsPtr()->setRawSetting(static_cast<uint8_t>(settingNum), static_cast<uint16_t>(value));
        Application::refreshSettings();
      }

      // Respond with current value (whether we just set it or not)
      {
        QueueEntry e;
        e.type = NotificationType::CmdSettingValue;
        e.data.settingNum = static_cast<uint8_t>(settingNum);
        _queueEnqueue(e);
      }
      break;
    }

    // --- Status LED ---
    case 'L': {
      if (length >= 3 && payload[2] == 'A') {
        // "$TCCLA<0|1>" -- set LED auto-adjust flag only
        if (length >= 4) {
          Application::setLedIntensityAutoAdjust(payload[3] == '1');
        }
      } else if (length > 2) {
        // "$TCCL<i>,<r>,<g>,<b>[,<0|1>]" -- set LED intensity and color, optionally set auto-adjust
        uint8_t idx = 2;
        int32_t i = _parseDecimal(payload, idx, length);
        if (idx < length && payload[idx] == ',') {
          idx++;
        }
        int32_t r = _parseDecimal(payload, idx, length);
        if (idx < length && payload[idx] == ',') {
          idx++;
        }
        int32_t g = _parseDecimal(payload, idx, length);
        if (idx < length && payload[idx] == ',') {
          idx++;
        }
        int32_t b = _parseDecimal(payload, idx, length);

        uint8_t ledIntensity = DisplayManager::getStatusLed().getIntensity();
        if (i >= 0 && i <= 255) {
          ledIntensity = static_cast<uint8_t>(i);
          Application::setLedIntensity(ledIntensity);
        }
        if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
          RgbLed newLed(static_cast<uint16_t>(r) * 16, static_cast<uint16_t>(g) * 16, static_cast<uint16_t>(b) * 16);
          newLed.setIntensity(ledIntensity);
          DisplayManager::writeStatusLed(newLed);
        }
        if (idx < length && payload[idx] == ',') {
          idx++;
          if (idx < length) {
            Application::setLedIntensityAutoAdjust(payload[idx] == '1');
          }
        }
      }

      // Sync _prevLed so process() doesn't fire a redundant notification
      _prevLed = DisplayManager::getStatusLed();

      // Always respond with current LED state
      _enqueueNotification(NotificationType::Led);
      break;
    }

    // --- Buzzer / RTTTL ---
    case 'B': {
      if (length >= 3) {
        char action = payload[2];

        if (action == 'P' && length > 3) {
          // "$TCCBP<rtttl>" -- play RTTTL string starting at payload[3]
          RtttlPlayer::play(payload + 3, length - 3);
          _queueCancelType(NotificationType::RtttlDone);  // cancel any stale done notification
        } else if (action == 'S') {
          // "$TCCBS" -- stop playback
          RtttlPlayer::stop();
          _queueCancelType(NotificationType::RtttlDone);
        } else if (action == 'C') {
          // "$TCCBC[hh]" -- play hourly chime; optional hour 0-23, else current hour
          uint8_t hour = 255;
          if (length > 3) {
            uint8_t idx = 3;
            int32_t n = _parseDecimal(payload, idx, length);
            if (n >= 0 && n <= 23) {
              hour = static_cast<uint8_t>(n);
            }
          }
          AlarmHandler::playChime(hour);
          _queueCancelType(NotificationType::RtttlDone);
        }
        // else: 'Q' or any other sub-command falls through to send status
      }

      // Always respond with current playback status
      {
        QueueEntry e;
        e.type = NotificationType::CmdBuzzerStatus;
        _queueEnqueue(e);
      }
      break;
    }

    // --- Timer/Counter ---
    case 'R': {
      if (length < 3) {
        break;
      }
      char action = payload[2];

      if (action == 'A') {
        // "$TCCRA" -- clear alarm (does not change mode)
        QueueEntry e;
        e.type = NotificationType::CmdAlarmClear;
        e.data.wasActive = AlarmHandler::isAlarmActive();
        AlarmHandler::clearAlarm();
        _prevAlarmActive = false;  // suppress redundant AlarmActive notification
        _queueEnqueue(e);
        break;
      }

      // All other R commands switch to timer mode
      _suppressNotifications = true;
      Application::setOperatingMode(Application::OperatingMode::OperatingModeTimerCounter);

      if (action == 'U') {
        TimerCounterView::setCountUp(true);
        Application::setViewMode(static_cast<ViewMode>(1));  // TimerRunUp
      } else if (action == 'D') {
        TimerCounterView::setCountUp(false);
        Application::setViewMode(static_cast<ViewMode>(2));  // TimerRunDown
      } else if (action == 'S') {
        Application::setViewMode(static_cast<ViewMode>(0));  // TimerStop
      } else if (action == 'R') {
        if (length > 3) {
          // "$TCCRR<seconds>" -- load arbitrary value, keep direction
          uint8_t idx = 3;
          int32_t val = _parseDecimal(payload, idx, length);
          if (val >= 0 && val <= static_cast<int32_t>(TimerCounterView::cMaxBcdValue)) {
            TimerCounterView::setTimerValue(static_cast<uint32_t>(val));
          }
          Application::setViewMode(static_cast<ViewMode>(0));  // TimerStop
        } else {
          // "$TCCRR" -- reset via standard mechanism (uses TimerResetValue setting)
          Application::setViewMode(static_cast<ViewMode>(3));  // TimerReset
        }
      }

      _suppressNotifications = false;

      {
        QueueEntry e;
        e.type = NotificationType::CmdTimerStatus;
        _queueEnqueue(e);
      }
      break;
    }

    // --- Diagnostics ---
    case 'D': {
      if (length < 3) {
        break;
      }
      char s0 = payload[2];

      if (s0 == 'F') {
        // "$TCCDF" -- firmware version + build number
        QueueEntry e;
        e.type = NotificationType::CmdDiagFirmware;
        _queueEnqueue(e);
      } else if (s0 == 'O' && length >= 4 && payload[3] == 'T') {
        if (length >= 5 && payload[4] == 'R') {
          // "$TCCDOTR" -- reset on-time counter
          QueueEntry e;
          e.type = NotificationType::CmdDiagOnTimeReset;
          _queueEnqueue(e);
        } else {
          // "$TCCDOT" -- query HV on-time (tube lifetime)
          QueueEntry e;
          e.type = NotificationType::CmdDiagOnTime;
          _queueEnqueue(e);
        }
      } else if (s0 == 'R' && length >= 5 && payload[3] == 'T' && payload[4] == 'C') {
        // "$TCCDRTC" -- active RTC type + startup result
        QueueEntry e;
        e.type = NotificationType::CmdDiagRtc;
        _queueEnqueue(e);
      } else if (s0 == 'S') {
        // "$TCCDS" -- settings load source at boot
        QueueEntry e;
        e.type = NotificationType::CmdDiagSettingsLoad;
        _queueEnqueue(e);
      } else if (s0 == 'G' && length >= 5 && payload[3] == 'P' && payload[4] == 'S') {
        // "$TCCDGPS" -- GPS connection/fix/satellite status
        QueueEntry e;
        e.type = NotificationType::CmdDiagGps;
        _queueEnqueue(e);
      }
      break;
    }

    default:
      break;
  }
}

}  // namespace kbxTubeClock::SerialRemote
