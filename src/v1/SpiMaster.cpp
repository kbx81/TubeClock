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
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/spi.h>

#include "Hardware.h"
#include "SpiMaster.h"

#if HARDWARE_VERSION == 1
  #include "Hardware_v1.h"
#else
  #error HARDWARE_VERSION must be defined with a value of 1
#endif


namespace kbxTubeClock {


SpiMaster::SpiMaster()
: _selectedSlave(cNoSlave),
  _nextSlave(0),
  _params{0, 0, 0, 0}
{
  for (uint8_t i = 0; i < cMaxSlaves; i++)
  {
    _transferRequest[i].state = SpiReqAck::SpiReqAckOk;
  }
}


void SpiMaster::initialize(const SpiMasterParams* spiInit)
{
  _params.spi             = spiInit->spi;
  _params.dmaController   = spiInit->dmaController;
  _params.channelRx       = spiInit->channelRx;
  _params.channelTx       = spiInit->channelTx;
}


uint8_t SpiMaster::registerSlave(SpiSlave* slave)
{
  if (_nextSlave < cMaxSlaves)
  {
    _slave[_nextSlave] = *slave;

    return _nextSlave++;
  }

  return cNoSlave;
}


SpiMaster::SpiReqAck SpiMaster::_configureMaster(const uint8_t slave)
{
  SpiReqAck result = SpiReqAck::SpiReqAckError;

  if (slave < _nextSlave)
  {
    // Reset SPI, SPI_CR1 register cleared, SPI is disabled
    spi_reset(_params.spi);

    // Set up SPI in Master mode as requested
    if (spi_init_master(_params.spi, _slave[slave].br, _slave[slave].cpol, _slave[slave].cpha, _slave[slave].lsbFirst) == 0)
    {
      result = SpiReqAck::SpiReqAckOk;
    }

    spi_fifo_reception_threshold_8bit(_params.spi);
    spi_set_data_size(_params.spi, _slave[slave].dataSize);
    /*
     * Set NSS management to software.
     *
     * Note:
     * Setting nss high is very important, even if we are controlling the GPIO
     * ourselves this bit needs to be at least set to 1, otherwise the SPI
     * peripheral will not send any data out.
     */
    spi_enable_software_slave_management(_params.spi);
    spi_set_nss_high(_params.spi);
  }

  return result;
}


void SpiMaster::_activateCsCe(const uint8_t slave)
{
  if (slave < _nextSlave)
  {
    if (_slave[slave].polarity == true)
    {
      gpio_set(_slave[slave].gpioPort, _slave[slave].gpioPin);
    }
    else
    {
      gpio_clear(_slave[slave].gpioPort, _slave[slave].gpioPin);
    }
  }
}


void SpiMaster::_deactivateCsCe(const uint8_t slave)
{
  if (slave < _nextSlave)
  {
    if (_slave[slave].polarity == true)
    {
      gpio_clear(_slave[slave].gpioPort, _slave[slave].gpioPin);
    }
    else
    {
      gpio_set(_slave[slave].gpioPort, _slave[slave].gpioPin);
    }
  }
}


void SpiMaster::_deactivateAllCsCe()
{
  for (uint8_t slave = 0; slave < _nextSlave; slave++)
  {
    _deactivateCsCe(slave);
  }
}


void SpiMaster::_configureMiso(const uint8_t slave)
{
  if (slave < _nextSlave)
  {
    // Configure peripheral MISO pin as alternate function
    gpio_mode_setup(_slave[slave].misoPort, GPIO_MODE_AF, GPIO_PUPD_NONE, _slave[slave].misoPin);
  }
}


void SpiMaster::_deconfigureMiso(const uint8_t slave)
{
  if (slave < _nextSlave)
  {
    // Configure peripheral MISO pin as input
    gpio_mode_setup(_slave[slave].misoPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, _slave[slave].misoPin);
  }
}


// Selects the active peripheral and configures the SPI for it
//  Activates the appropriate CS/CE line based on slave's registration
// ***** Make sure SPI is not busy/in use before switching things around! *****
SpiMaster::SpiReqAck SpiMaster::_selectPeripheral(const uint8_t slave)
{
  SpiReqAck result = SpiReqAck::SpiReqAckOk;

  // We won't waste time switching things around if we don't need to
  if (_selectedSlave != slave)
  {
    _deconfigureMiso(_selectedSlave);
    // if we are selecting a slave...and not just deactivating CS/CE lines...
    if (slave != cNoSlave)
    {
      _configureMiso(slave);

      _deactivateCsCe(_selectedSlave);
      // Reset SPI, SPI_CR1 register cleared, SPI is disabled
      spi_disable(_params.spi);
      // Configure the SPI as master for the slave being selected
      result = _configureMaster(slave);
      // (Re)Enable SPI1 peripheral
      spi_enable(_params.spi);

      if (_slave[slave].strobeCs == false)
      {
        _activateCsCe(slave);
      }
    }
    else
    {
      _deactivateCsCe(_selectedSlave);
    }
    _selectedSlave = slave;    // save for later!
  }

  return result;
}


SpiMaster::SpiTransferReq* SpiMaster::getTransferRequestBuffer(const uint8_t slave)
{
  if (slave < _nextSlave)
  {
    return &_transferRequest[slave];
  }
  return nullptr;
}


SpiMaster::SpiReqAck SpiMaster::queueTransfer(const uint8_t slave, SpiTransferReq* request)
{
  SpiReqAck status = SpiReqAck::SpiReqAckBusy;

  if (slave < _nextSlave)
  {
    if (_transferRequest[slave].state == SpiReqAck::SpiReqAckOk)
    {
      _transferRequest[slave] = *request;

      _transferRequest[slave].state = SpiReqAck::SpiReqAckQueued;

      status = SpiReqAck::SpiReqAckOk;
    }
  }
  else
  {
    status = SpiReqAck::SpiReqAckError;
  }

  return status;
}


// Hardware::HwReqAck SpiMaster::transfer(const Hardware::SpiPeripheral peripheral, uint8_t *bufferIn, uint8_t *bufferOut, const uint16_t length)
SpiMaster::SpiReqAck SpiMaster::transfer(const uint8_t slave, SpiTransferReq* request)
{
  uint32_t dmaEnable = 0;
  volatile uint8_t temp_data __attribute__ ((unused));

  if (_selectedSlave != cNoSlave)
  {
    // Let the caller know it was busy if so
    return SpiReqAck::SpiReqAckBusy;
  }

	// Reset SPI data and status registers
	// First, ensure the SPI is not busy...
  while (SPI_SR(_params.spi) & (SPI_SR_BSY));
  // ...now we purge the FIFO to ensure it's empty for the new inbound bits.
	while (SPI_SR(_params.spi) & (SPI_SR_RXNE | SPI_SR_OVR))
  {
		temp_data = SPI_DR(_params.spi);
	}

  // Configure SPI1 for use with the appropriate peripheral
  if (_selectPeripheral(slave) != SpiReqAck::SpiReqAckOk)
  {
    return SpiReqAck::SpiReqAckError;
  }
  // indicate busy
  request->state = SpiReqAck::SpiReqAckBusy;

  if (_params.dmaController != 0)
  {
    if (_params.channelRx != 0)
    {
      // Reset DMA channel
      dma_channel_reset(_params.dmaController, _params.channelRx);
      // Set up Rx DMA, note it has higher priority to avoid overrun
      dma_set_peripheral_address(_params.dmaController, _params.channelRx, (uint32_t)&SPI1_DR);
      dma_set_memory_address(_params.dmaController, _params.channelRx, (uint32_t)request->bufferIn);
      dma_set_number_of_data(_params.dmaController, _params.channelRx, request->length);
      dma_set_read_from_peripheral(_params.dmaController, _params.channelRx);
      dma_enable_memory_increment_mode(_params.dmaController, _params.channelRx);
      dma_set_peripheral_size(_params.dmaController, _params.channelRx, _slave[request->slave].peripheralSize);
      dma_set_memory_size(_params.dmaController, _params.channelRx, _slave[request->slave].memorySize);
      dma_set_priority(_params.dmaController, _params.channelRx, DMA_CCR_PL_VERY_HIGH);
      // Enable DMA transfer complete interrupt
    	dma_enable_transfer_complete_interrupt(_params.dmaController, _params.channelRx);
      // Activate DMA channel
    	dma_enable_channel(_params.dmaController, _params.channelRx);
      dmaEnable |= SPI_CR2_RXDMAEN;
    }

    if (_params.channelTx != 0)
    {
      // Reset DMA channel
      dma_channel_reset(_params.dmaController, _params.channelTx);
      // Set up tx DMA
      dma_set_peripheral_address(_params.dmaController, _params.channelTx, (uint32_t)&SPI1_DR);
      dma_set_memory_address(_params.dmaController, _params.channelTx, (uint32_t)request->bufferOut);
      dma_set_number_of_data(_params.dmaController, _params.channelTx, request->length);
      dma_set_read_from_memory(_params.dmaController, _params.channelTx);
      dma_enable_memory_increment_mode(_params.dmaController, _params.channelTx);
      dma_set_peripheral_size(_params.dmaController, _params.channelTx, _slave[request->slave].peripheralSize);
      dma_set_memory_size(_params.dmaController, _params.channelTx, _slave[request->slave].memorySize);
      dma_set_priority(_params.dmaController, _params.channelTx, DMA_CCR_PL_HIGH);
      // Enable DMA transfer complete interrupt
    	dma_enable_transfer_complete_interrupt(_params.dmaController, _params.channelTx);
      // Activate DMA channel
    	dma_enable_channel(_params.dmaController, _params.channelTx);
      dmaEnable |= SPI_CR2_TXDMAEN;
    }
  }

	/* Enable the SPI transfer via DMA
	 * This will immediately start the transmission, after which when the receive
   * is complete, the receive DMA will activate
	 */
	// spi_enable_rx_dma(spiParams.spi);
	// spi_enable_tx_dma(spiParams.spi);
  SPI_CR2(_params.spi) |= dmaEnable;

  return SpiReqAck::SpiReqAckOk;
}


bool SpiMaster::transferComplete(const uint8_t slave)
{
  if (slave < _nextSlave)
  {
    return (_transferRequest[slave].state == SpiReqAck::SpiReqAckOk);
  }
  return false;
}


bool SpiMaster::processQueue()
{
  for (uint8_t slave = 0; slave < _nextSlave; slave++)
  {
    if (_transferRequest[slave].state == SpiReqAck::SpiReqAckQueued)
    {
      transfer(slave, &_transferRequest[slave]);
      // return indicating a transfer was initiated
      return true;
    }
  }
  // return indicating no transfer was initiated
  return false;
}


void SpiMaster::dmaComplete()
{
  if (_selectedSlave < _nextSlave)
  {
    // tx & rx should always finish at the same time; wait for any remaining bits to roll out
    while ((SPI_SR(SPI1) & SPI_SR_BSY) != 0);
    // strobe the CS/CE pin if slave is configured for it
    if (_slave[_selectedSlave].strobeCs == true)
    {
      _activateCsCe(_selectedSlave);
    }
    // indicate completion
    _transferRequest[_selectedSlave].state = SpiReqAck::SpiReqAckOk;

    // release the appropriate CS line (also deasserts the CS/CE line if strobed)
    _selectPeripheral(cNoSlave);
    // fire off the next queued item (maintains more consistent timing for PWMing)
    processQueue();
  }
}


bool SpiMaster::busy()
{
  return (_selectedSlave != cNoSlave);
}


bool SpiMaster::queuesEmpty()
{
  for (uint8_t slave = 0; slave < _nextSlave; slave++)
  {
    if (_transferRequest[slave].state == SpiReqAck::SpiReqAckQueued)
    {
      return false;
    }
  }
  return true;
}


}
