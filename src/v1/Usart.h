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

#include <libopencm3/stm32/usart.h>

namespace kbxTubeClock {

class Usart {
 public:
  /// @brief Request return codes
  ///
  enum UsartReqAck : uint8_t {
    UsartReqAckOk,
    UsartReqAckBusy,
    UsartReqAckError,
    UsartReqAckQueued,
  };

  /// @brief Structure defining USART object initialization
  ///
  struct UsartParams {
    uint32_t usart;          // USART peripheral identifier
    uint32_t dmaController;  // DMA controller base address (0 if no DMA)
    uint8_t channelRx;       // Receive DMA channel number: 1-7 for DMA1 (0 if no DMA)
    uint8_t channelTx;       // Transmit DMA channel number: 1-7 for DMA1 (0 if no DMA)
  };

  /// @brief Structure defining USART data transfer parameters
  ///
  struct UsartTransferParams {
    uint32_t baudRate;      // Baud rate
    uint32_t stopBits;      // Number of stop bits
    uint32_t parity;        // Parity
    uint32_t mode;          // Mode (Tx, Rx, Tx+Rx)
    uint32_t flowControl;   // Flow control
    uint32_t autoBaudMode;  // Enable auto-baud rate detection
    uint8_t dataBits;       // Number of data bits
    bool driverEnableMode;  // Enable hardware DE output (for RS-485)
    bool swapTxRx;          // Enable TX/RX pin swap (USART_CR2_SWAP)
  };

  /// @brief Structure defining USART transfer requests
  ///
  struct UsartTransferReq {
    uint8_t *buffer;
    uint16_t length;
    UsartReqAck state;
  };

 public:
  /// @brief Default constructor
  ///
  Usart();

  /// @brief Initialize USART
  /// @param usartInit structure containing desired master configuration
  ///
  void initialize(const UsartParams *usartInit);

  /// @brief Configure USART parameters
  /// @param usartTransferParams structure containing desired USART parameters
  ///
  void configure(const UsartTransferParams *usartTransferParams);

  /// @brief Recieves data in through the USART via DMA
  /// @param request Pointer to request structure used to receive
  /// @return UsartReqAck state of transfer request
  ///
  UsartReqAck receiveDma(UsartTransferReq *request);

  /// @brief Transmits/sends data out through the USART via DMA
  /// @param request Pointer to request structure used to transmit
  /// @return UsartReqAck state of transfer request
  ///
  UsartReqAck transmitDma(UsartTransferReq *request);

  /// @brief Called from DMA complete interrupt
  ///
  void dmaComplete();

  /// @brief Enable the USART peripheral
  ///
  void enable() const;

  /// @brief Disable the USART peripheral
  ///
  void disable() const;

  /// @brief Get the USART peripheral base address
  ///
  inline uint32_t peripheral() const { return _params.usart; }

  /// @brief Check if a received byte is ready to read
  ///
  inline bool rxReady() const { return (USART_ISR(_params.usart) & USART_ISR_RXNE) != 0; }

  /// @brief Read a received byte (clears RXNE flag)
  ///
  inline uint8_t readByte() const { return static_cast<uint8_t>(USART_RDR(_params.usart)); }

  /// @brief Check if the transmit data register is empty
  ///
  inline bool txReady() const { return (USART_ISR(_params.usart) & USART_ISR_TXE) != 0; }

  /// @brief Write a byte to the transmit data register
  ///
  inline void writeByte(uint8_t byte) const { USART_TDR(_params.usart) = byte; }

  /// @brief Enable/disable RX and TX interrupts
  ///
  inline void enableRxInterrupt() const { usart_enable_rx_interrupt(_params.usart); }
  inline void disableRxInterrupt() const { usart_disable_rx_interrupt(_params.usart); }
  inline void enableTxInterrupt() const { usart_enable_tx_interrupt(_params.usart); }
  inline void disableTxInterrupt() const { usart_disable_tx_interrupt(_params.usart); }

  /// @brief Check for and clear USART error flags (FE, NF, ORE)
  ///
  inline bool hasErrors() const {
    return (USART_ISR(_params.usart) & (USART_ISR_FE | USART_ISR_NF | USART_ISR_ORE)) != 0;
  }
  inline void clearErrors() const { USART_ICR(_params.usart) = USART_ISR_FE | USART_ISR_NF | USART_ISR_ORE; }

 private:
  /// @brief our configuration
  ///
  UsartParams _params;
};

}  // namespace kbxTubeClock
