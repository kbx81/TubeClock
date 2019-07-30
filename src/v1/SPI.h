//
// kbx81's tube clock SPI class
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

#include "Hardware.h"


namespace kbxTubeClock {

class Spi {

public:

  /// @brief SPI1 peripherals
  ///
  enum SpiPeripheral : uint8_t {
    NotInitialized,
    HvDrivers,
    Rtc,
    TempSensor
  };

  /// @brief Structure defining SPI transfer requests
  ///
  struct SpiTransferReq {
    SpiPeripheral peripheral;
    uint8_t *bufferIn;
    uint8_t *bufferOut;
    uint16_t length;
    bool use16BitXfers;
    volatile Hardware::HwReqAck state;
  };

public:

  /// @brief some constant
  ///
  static const uint32_t cSpiConstant;


  /// @brief Default constructor
  ///
  Spi();


  /// @brief Initialize SPI
  ///
  void initialize();

  /// @brief Queues up a transfer via SPI via DMA
  /// @return HwReqAck state of transfer request
  Hardware::HwReqAck spiTransferRequest(SpiTransferReq* request);

  /// @brief Transfers data in/out through the SPI via DMA
  /// @return HwReqAck state of transfer request
  Hardware::HwReqAck spiTransfer(const SpiPeripheral peripheral, uint8_t *bufferIn, uint8_t *bufferOut, const uint16_t length, const bool use16BitXfers);

  /// @brief Permits checking the status of the SPI; returns true if busy
  ///
  bool     spiIsBusy();


private:

  /// @brief some variable
  ///
  uint16_t _variable;

};

}
