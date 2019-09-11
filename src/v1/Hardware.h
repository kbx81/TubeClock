//
// kbx81's tube clock Hardware class
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

#include <cstddef>
#include <cstdint>

#include <libopencm3/stm32/usart.h>

#include "DateTime.h"
#include "Display.h"
#include "RgbLed.h"
#include "SpiMaster.h"
#include "Usart.h"

#ifndef HARDWARE_VERSION
  #error HARDWARE_VERSION must be defined with a value of 1
#endif


namespace kbxTubeClock {

namespace Hardware {

  /// @brief Hardware request return codes
  ///
  enum HwReqAck : uint8_t {
    HwReqAckOk,
    HwReqAckBusy,
    HwReqAckError,
    HwReqAckQueued
  };

  /// @brief What triggers peripheral refreshes
  ///
  enum PeripheralRefreshTrigger : uint8_t {
    NoTrigger,
    PpsExti,
    SysTick
  };

  /// @brief Type of temperature sensor connected, if any
  ///
  enum RtcType : uint8_t {
    Stm32,
    DS323x
  };

  /// @brief Type of temperature sensor connected, if any
  ///
  enum TempSensorType : uint8_t {
    NoTempSensor,
    DS3231,
    DS3234,
    DS1722,
    LM74,
    LM75,
    MCP9808
  };


  // Number of USARTs to configure
  //
  static const uint8_t cNumberOfUsarts = 2;

  // USART default/initial baud rates
  //
  static const uint32_t cUsart1BaudRate = 115200;
  static const uint32_t cUsart2BaudRate = 250000;

  // USART1 data transnfer parameters
  //
  static const uint8_t cUsart1DataBits  = 8;                      // Number of data bits
  static const auto cUsart1StopBits     = USART_STOPBITS_1;       // Number of stop bits
  static const auto cUsart1Parity       = USART_PARITY_NONE;      // Parity
  static const auto cUsart1Mode         = USART_MODE_TX_RX;       // Mode (Tx, Rx, Tx+Rx)
  static const auto cUsart1FlowControl  = USART_FLOWCONTROL_NONE; // Flow control
  static const auto cUsart1AutoBaud     = USART_CR2_ABRMOD_FALL_EDGE | USART_CR2_ABREN; // Enable auto-baud rate detection
  static const auto cUsart1DriverEnable = false;                  // Enable hardware DE output (for RS-485)

  // USART1 data transnfer parameters
  //
  static const uint8_t cUsart2DataBits  = 8;                      // Number of data bits
  static const auto cUsart2StopBits     = USART_STOPBITS_2;       // Number of stop bits
  static const auto cUsart2Parity       = USART_PARITY_NONE;      // Parity
  static const auto cUsart2Mode         = USART_MODE_RX;          // Mode (Tx, Rx, Tx+Rx)
  static const auto cUsart2FlowControl  = USART_FLOWCONTROL_NONE; // Flow control
  static const auto cUsart2AutoBaud     = 0;                      // Enable auto-baud rate detection
  static const auto cUsart2DriverEnable = true;                   // Enable hardware DE output (for RS-485)

  /// @brief Hardware version to build for
  ///
  static const uint32_t cTargetHardwareVersion = HARDWARE_VERSION;


  /// @brief Initialize the hardware
  ///
  void     initialize();

  /// @brief Refreshes hardware external to the MCU
  ///
  void     refresh();

  /// @brief Generates a tick sound if no tone is active
  /// @return HwReqOk if tick was activated, HwReqBusy if tick was queued, else HwReqError
  HwReqAck tick();

  /// @brief Generates a tone of frequency for duration milliseconds
  /// @return HwReqOk if tone was activated, HwReqBusy if tone was queued, else HwReqError
  HwReqAck tone(const uint16_t frequency, const uint16_t duration);

  /// @brief Returns true if an external alarm input pin is active (they are active low)
  ///
  bool     alarmInput(const uint8_t alarmInputNumber);

  /// @brief Check a button
  ///
  bool     button(const uint8_t button);

  /// @brief Returns a bitmap indicating the state of all six buttons
  ///
  uint8_t  buttons();

  /// @brief Returns a bitmap indicating buttons that were pressed since the last check
  ///
  uint8_t  buttonsPressed();

  /// @brief Returns a bitmap indicating buttons that were released since the last check
  ///
  uint8_t  buttonsReleased();

  /// @brief Refreshes the button states from the TSC
  ///
  void     buttonsRefresh();

  /// @brief Switches on/off the boost converter. true = on, false = off
  ///
  void     setHvState(const bool hvEnabled);

  /// @brief Enable or disable the display blanking output pin
  ///
  void     setDisplayHardwareBlanking(const bool blankingState);

  /// @brief Returns the state of daylight savings time in hardware
  ///
  // bool     dstState();

  /// @brief Get the current date/time
  ///
  DateTime dateTime();

  /// @brief Set the date/time
  ///
  void     setDateTime(const DateTime &dateTime);

  /// @brief Returns the state of the RTC (true if year != 0)
  ///
  bool     rtcIsSet();

  /// @brief Returns the temperature based on hardware
  ///  (in fahrenheit if fahrenheit == true; in BCD if bcd == true)
  int32_t  temperature(const bool fahrenheit, const bool bcd = false);

  /// @brief Returns the level of light seen by the phototransistor (0 = min, 4095 = max)
  ///
  uint16_t lightLevel();

  /// @brief Returns the time HV has been on in seconds
  ///
  uint32_t onTimeSeconds();

  /// @brief Resets the HV time-on counter
  ///
  void     onTimeSecondsReset();

  /// @brief Returns the current battery voltage times 1000
  ///
  uint16_t voltageBatt();

  /// @brief Returns the current input voltage (VddA) times 1000
  ///
  uint16_t voltageVddA();

  /// @brief Adjusts the clock for daylight savings
  ///
  // void     setDstState(const bool enableDst, const bool adjustRtcHardware);

  /// @brief Sets the value used as a threshold for flicker reduction at low intensities
  ///
  void     setFlickerReduction(const uint16_t value);

  /// @brief Sets the calibration value used for the temperature calculation
  ///
  void     setTemperatureCalibration(const int8_t value);

  /// @brief Sets the volume of tones emitted from the buzzer
  ///  Valid range is 0 (muted/minimum) to 7 (maximum/default)
  void     setVolume(const uint8_t volumeLevel);

  /// @brief Returns a CRC for the given data; expects 32-bit words
  ///
  uint32_t getCRC(uint32_t *inputData, uint32_t numElements);

  /// @brief Returns the active peripheral refresh trigger
  ///
  PeripheralRefreshTrigger getPeripheralRefreshTrigger();

  /// @brief Returns the state of the PPS pin
  ///
  bool getPpsInputState();

  /// @brief Returns the active RTC type/source
  ///
  RtcType getRTCType();

  /// @brief Returns the active temperature sensor type/source
  ///
  TempSensorType getTempSensorType();

  /// @brief Erases the FLASH area used for app data
  /// @return 0xff if startAddress is out of range, flash_get_status_flags() if
  ///  erase request failed, 0x40 if erase verification failed
  uint32_t eraseFlash(uint32_t startAddress);

  /// @brief Reads the FLASH area used for app data
  ///
  void     readFlash(uint32_t startAddress, uint16_t numElements, uint8_t *outputData);

  /// @brief Writes the FLASH area used for app data
  /// @return 0xff if startAddress is out of range, flash_get_status_flags() if
  ///  erase request failed, 0x40 if erase verification failed,
  ///  (0x80 | flash_get_status_flags()) if programming failure
  uint32_t writeFlash(uint32_t startAddress, uint8_t *inputData, uint16_t numElements);

  /// @brief Reads and/or writes data to/from specified I2C interface via DMA
  ///
  HwReqAck i2cTransfer(const uint8_t addr, const uint8_t *bufferTx, size_t numberTx, uint8_t *bufferRx, size_t numberRx);

  /// @brief Reads and/or writes data to/from the I2C1 interface via DMA
  ///
  HwReqAck i2cReceive(const uint8_t addr, uint8_t *bufferRx, const size_t numberRx, const bool autoEndXfer);
  HwReqAck i2cTransmit(const uint8_t addr, const uint8_t *bufferTx, const size_t numberTx, const bool autoEndXfer);

  /// @brief Aborts a transfer initiated by i2cTransfer()
  ///
  void     i2cAbort();

  /// @brief Permits checking the status of the I2C; returns true if busy
  ///
  bool     i2cIsBusy();

  /// @brief Gets the pointer to the current SpiMaster
  /// @return pointer to current SpiMaster
  SpiMaster* getSpiMaster();

  /// @brief Gets a pointer to the specified USART interface
  /// @return pointer to requested USART interface
  Usart*   getUsart(const uint8_t usart);

  /// @brief Sets the given status LED to the given intensity/RgbLed
  ///
  void     setBlueLed(const uint32_t intensity);
  void     setGreenLed(const uint32_t intensity);
  void     setRedLed(const uint32_t intensity);
  void     setStatusLed(const RgbLed led);
  void     blinkStatusLed(const RgbLed led1, const RgbLed led2, uint32_t numberOfBlinks, const uint32_t delayLength);

  /// @brief Creates a delay of length based on systick (configured for milliseconds)
  ///
  void     delay(const uint32_t length);

  /// @brief Initiates a non-blocking delay of length based on systick (configured for milliseconds)
  ///
  void     delayNBStart();

  /// @brief Returns true if the non-blocking delay initiated by delayNB() is complete
  ///
  bool     delayNBComplete(const uint32_t length);

  /// @brief Converts to/from int/BCD
  /// @return encoded value
  ///
  uint32_t bcdToUint32(uint32_t bcdValue);
  uint32_t uint32ToBcd(uint32_t uint32Value); // beware of values > 99999999


  /// @brief Interrupt Service Routines
  ///
  void     dmaCh1Isr();
  void     dmaCh2to3Isr();
  void     dmaCh4to7Isr();
  void     exti415Isr();
  void     systickIsr();
  void     tim2Isr();
  void     tim7Isr();
  void     tim15Isr();
  void     tim16Isr();
  void     tscIsr();
  void     usart1Isr();
  void     usart2Isr();

}

}
