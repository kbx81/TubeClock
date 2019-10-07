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
// To use:
// 1. Hardware starts -- SpiMaster object gets initialized via initialize()
// 2. Slaves register:
//     a. Slave must get pointer to SpiMaster object (via Hardware::getSpiMaster())
//     b. Slave calls SpiMaster::registerSlave() with its slave structure
//     c. Slave stores returned slave ID
// 3. Slave(s) initiate(s) transfers by calling SpiMaster::transfer() with slave ID
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
    bool     strobeCs;        // CS line is strobed upon xfer completion if true
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
  };

  /// @brief Structure defining SPI transfer requests
  ///
  struct SpiTransferReq {
    SpiReqAck state;
    uint8_t slave;
    uint8_t *bufferIn;
    uint8_t *bufferOut;
    uint16_t length;
  };

public:

  /// @brief maximum number of slaves supported
  ///
  static const uint8_t cMaxSlaves = 4;

  /// @brief value indicating no slave registration occured
  ///
  static const uint8_t cNoSlave = 255;

  /// @brief maximum number of requests that may be queued
  ///
  static const uint8_t cQueueSize = cMaxSlaves;


  /// @brief Default constructor
  ///
  SpiMaster();


  /// @brief Initialize SPI
  /// @param spiInit structure containing desired master configuration
  void initialize(const SpiMasterParams* spiInit);

  /// @brief Queues up a transfer via SPI via DMA
  /// @param slave structure containing desired slave configuration
  /// @return ID of slave as registered or cNoSlave if failure
  uint8_t registerSlave(SpiSlave* slave);

  /// @brief Gets a pointer to the slave's transfer request buffer
  /// @param slave ID number to perform transfer for
  /// @return SpiTransferReq pointer to the slave's transfer request buffer
  SpiTransferReq* getTransferRequestBuffer(const uint8_t slave);

  /// @brief Queues up a transfer via SPI via DMA
  /// @param slave ID number to perform transfer for
  /// @return HwReqAck state of transfer request
  SpiReqAck queueTransfer(const uint8_t slave, SpiTransferReq* request);

  /// @brief Transfers data in/out through the SPI via DMA
  /// @param slave ID number to perform transfer for
  /// @return HwReqAck state of transfer request
  SpiReqAck transfer(const uint8_t slave, SpiTransferReq* request);

  /// @brief Indicates if a given slave's transfer has completed
  /// @return true if transfer is complete
  ///
  bool transferComplete(const uint8_t slave);

  /// @brief looks for the next transfer to initiate
  /// @return true if another transfer was initiated
  ///
  bool processQueue();

  /// @brief Called from DMA complete interrupt
  ///
  void dmaComplete();

  /// @brief Permits checking the status of the SPI; returns true if busy
  ///
  bool busy();

  /// @brief Permits checking the status of the queues; returns true if no transfers are queued
  ///
  bool queuesEmpty();

private:

  SpiReqAck _configureMaster(const uint8_t slave);

  void _activateCsCe(const uint8_t slave);
  void _deactivateCsCe(const uint8_t slave);

  void _deactivateAllCsCe();

  void _configureMiso(const uint8_t slave);
  void _deconfigureMiso(const uint8_t slave);

  SpiReqAck _selectPeripheral(const uint8_t slave);

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

  /// @brief transfer requests from slaves that the SPI needs to do
  ///
  SpiTransferReq _transferRequest[cMaxSlaves];

};

}
