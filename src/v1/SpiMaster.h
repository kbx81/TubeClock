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


namespace kbxTubeClock {

class SpiMaster {

public:

  /// @brief Request return codes
  ///
  enum SpiReqAck : uint8_t {
    SpiReqAckOk,
    SpiReqAckBusy,
    SpiReqAckError,
    SpiReqAckQueued
  };

  /// @brief Structure defining SPI object initialization
  ///
  struct SpiMasterParams {
    uint32_t spi;             // SPI peripheral identifier
    uint32_t dmaController;   // DMA controller base address
    uint8_t  channelRx;       // Recieve DMA channel number: 1-7 for DMA1
    uint8_t  channelTx;       // Transmit DMA channel number: 1-7 for DMA1
  };

  /// @brief Structure defining SPI transfer requests
  ///
  struct SpiSlave {
    uint32_t gpioPort;        // gpio port on which CS line lives
    uint16_t gpioPin;         // gpio pin on which CS line lives
    bool     polarity;        // CS/CE polarity (active high = true)
    uint32_t misoPort;        // port on which slave inputs data
    uint16_t misoPin;         // pin on which slave inputs data
    uint32_t br;              // Baudrate
    uint32_t cpol;            // Clock polarity
    uint32_t cpha;            // Clock Phase
    uint32_t lsbFirst;        // Frame format -- lsb/msb first
    uint32_t dataSize;        // Data size (4 to 16 bits, see RM)
    uint32_t memorySize;      // Memory word width (8, 16, 32 bit)
    uint32_t peripheralSize;  // Peripheral word width (8, 16, 32 bit)
    // uint32_t Mode;
    // uint32_t Direction;
    // uint32_t DataSize;
    // uint32_t CLKPolarity;
    // uint32_t CLKPhase;
    // uint32_t NSS;
    // uint32_t BaudRatePrescaler;
    // uint32_t FirstBit;
    // uint32_t TIMode;
    // uint32_t CRCCalculation;
    // uint32_t CRCPolynomial;
    // uint32_t CRCLength;
    // uint32_t NSSPMode;
  };

  /// @brief Structure defining SPI transfer requests
  ///
  struct SpiTransferReq {
    uint8_t slave;
    uint8_t *bufferIn;
    uint8_t *bufferOut;
    uint16_t length;
    volatile SpiReqAck state;
  };

public:

  /// @brief maximum number of slaves supported
  ///
  static const uint8_t cMaxSlaves = 4;

  /// @brief maximum number of requests that may be queued
  ///
  static const uint8_t cQueueSize = 4;


  /// @brief Default constructor
  ///
  SpiMaster();


  /// @brief Initialize SPI
  /// @param SpiMasterParams structure containing desired configuration
  /// @return HwReqAck result of initialization request
  void initialize(const SpiMasterParams* spiInit);

  /// @brief Queues up a transfer via SPI via DMA
  /// @return ID of slave as registered
  uint8_t registerSlave(SpiSlave* slave);

  /// @brief Queues up a transfer via SPI via DMA
  /// @return HwReqAck state of transfer request
  SpiReqAck queueTransfer(SpiTransferReq* request);

  /// @brief Transfers data in/out through the SPI via DMA
  /// @return HwReqAck state of transfer request
  SpiReqAck transfer(SpiTransferReq* request);

  /// @brief Called from DMA complete interrupt
  ///
  void transferComplete();

  /// @brief Permits checking the status of the SPI; returns true if busy
  ///
  bool busy();


private:

  SpiReqAck _configureMaster(const uint8_t slave);

  void _activateCsCe(const uint8_t slave);
  void _deactivateCsCe(const uint8_t slave);

  void _configureMiso(const uint8_t slave);
  void _deconfigureMiso(const uint8_t slave);

  void _releaseAllCs();
  void _selectPeripheral(const uint8_t slave);

  /// @brief value used to indicate no slave is active/selected
  ///
  static const uint8_t cNoSelectedSlave;

  /// @brief the active/currently selected slave
  ///
  uint8_t _selectedSlave;

  /// @brief the next slave to be registered
  ///
  uint8_t _nextSlave;

  /// @brief our configuration
  ///
  SpiMasterParams _params;

  /// @brief slave configurations
  ///
  SpiSlave _slave[cMaxSlaves];

  /// @brief the next transfer the SPI needs to do
  ///
  SpiTransferReq* _transferQueue[cQueueSize];

};

}
