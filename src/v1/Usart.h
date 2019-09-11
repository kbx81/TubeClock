//
// kbx81's tube clock USART class
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


namespace kbxTubeClock {

class Usart {

public:

  /// @brief Request return codes
  ///
  enum UsartReqAck : uint8_t {
    UsartReqAckOk,
    UsartReqAckBusy,
    UsartReqAckError,
    UsartReqAckQueued
  };

  /// @brief Structure defining USART object initialization
  ///
  struct UsartParams {
    uint32_t usart;             // USART peripheral identifier
    uint32_t dmaController;     // DMA controller base address
    uint8_t  channelRx;         // Recieve DMA channel number: 1-7 for DMA1
    uint8_t  channelTx;         // Transmit DMA channel number: 1-7 for DMA1
  };

  /// @brief Structure defining USART object initialization
  ///
  struct UsartTransferParams {
    uint32_t baudRate;          // Baud rate
    uint8_t  dataBits;          // Number of data bits
    uint32_t stopBits;          // Number of stop bits
    uint32_t parity;            // Parity
    uint32_t mode;              // Mode (Tx, Rx, Tx+Rx)
    uint32_t flowControl;       // Flow control
    uint32_t autoBaudMode;      // Enable auto-baud rate detection
    bool     driverEnableMode;  // Enable hardware DE output (for RS-485)
  };

  /// @brief Structure defining USART transfer requests
  ///
  struct UsartTransferReq {
    UsartReqAck state;
    uint8_t *buffer;
    uint16_t length;
  };

public:


  /// @brief Default constructor
  ///
  Usart();


  /// @brief Initialize USART
  /// @param usartInit structure containing desired master configuration
  void initialize(const UsartParams* usartInit);

  /// @brief Configure USART parameters
  /// @param usartTransferParams structure containing desired USART parameters
  void configure(const UsartTransferParams* usartTransferParams);

  /// @brief Recieves data in through the USART via DMA
  /// @param request Pointer to request structure used to receive
  /// @return UsartReqAck state of transfer request
  UsartReqAck receive(UsartTransferReq* request);

  /// @brief Transmist/sends data out through the USART via DMA
  /// @param request Pointer to request structure used to transmit
  /// @return UsartReqAck state of transfer request
  UsartReqAck transmit(UsartTransferReq* request);

  /// @brief Called from DMA complete interrupt
  ///
  void dmaComplete();

private:

  /// @brief our configuration
  ///
  UsartParams _params;

};

}
