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

// some constant
const uint8_t SpiMaster::cNoSelectedSlave = 255;


SpiMaster::SpiMaster()
: _selectedSlave(cNoSelectedSlave),
  _nextSlave(0),
  _params{0, 0, 0, 0}
//   _transferQueue[0](nullptr),
//   _transferQueue[1](nullptr),
//   _transferQueue[2](nullptr),
//   _transferQueue[3](nullptr)
{
  // initialize();
}


void SpiMaster::initialize(const SpiMasterParams* spiInit)
{
  _params.spi             = spiInit->spi;
  // _params.br              = spiInit->br;
  // _params.cpol            = spiInit->cpol;
  // _params.cpha            = spiInit->cpha;
  // _params.lsbFirst        = spiInit->lsbFirst;
  _params.dmaController   = spiInit->dmaController;
  _params.channelRx       = spiInit->channelRx;
  _params.channelTx       = spiInit->channelTx;
  // _params.memorySize      = spiInit->memorySize;
  // _params.peripheralSize  = spiInit->peripheralSize;

  // Reset SPI, SPI_CR1 register cleared, SPI is disabled
  // spi_reset(_params.spi);

  // Set up SPI in Master mode as requested
  // if (spi_init_master(_params.spi, _params.br, _params.cpol, _params.cpha, _params.lsbFirst) != 0)
  // {
  //   result = Hardware::HwReqAck::HwReqAckError;
  // }

  // spi_fifo_reception_threshold_8bit(SPI1);
  // spi_set_data_size(SPI1, SPI_CR2_DS_8BIT);
  /*
   * Set NSS management to software.
   *
   * Note:
   * Setting nss high is very important, even if we are controlling the GPIO
   * ourselves this bit needs to be at least set to 1, otherwise the SPI
   * peripheral will not send any data out.
   */
  // spi_enable_software_slave_management(SPI1);
  // spi_set_nss_high(SPI1);
}


uint8_t SpiMaster::registerSlave(SpiSlave* slave)
{
  if (_nextSlave < cMaxSlaves)
  {
    _slave[_nextSlave] = *slave;

    return _nextSlave++;
  }

  return cNoSelectedSlave;
}


SpiReqAck SpiMaster::_configureMaster(const uint8_t slave)
{
  SpiReqAck result = SpiReqAck::SpiReqAckOk;

  if (slave < cMaxSlaves)
  {
    // Reset SPI, SPI_CR1 register cleared, SPI is disabled
    spi_reset(_params.spi);

    // Set up SPI in Master mode as requested
    if (spi_init_master(_params.spi, _slave[slave].br, _slave[slave].cpol, _slave[slave].cpha, _slave[slave].lsbFirst) != 0)
    {
      result = Hardware::HwReqAck::HwReqAckError;
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
  else
  {
    result = Hardware::HwReqAck::HwReqAckError;
  }

  return result;
}


void SpiMaster::_activateCsCe(const uint8_t slave)
{
  if (slave < cMaxSlaves)
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
  if (slave < cMaxSlaves)
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


void SpiMaster::_configureMiso(const uint8_t slave)
{
  if (slave < cMaxSlaves)
  {
    // Configure peripheral MISO pin as alternate function
    gpio_mode_setup(_slave[slave].misoPort, GPIO_MODE_AF, GPIO_PUPD_NONE, _slave[slave].misoPin);
  }
}


void SpiMaster::_deconfigureMiso(const uint8_t slave)
{
  if (slave < cMaxSlaves)
  {
    // Configure peripheral MISO pin as input
    gpio_mode_setup(_slave[slave].misoPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, _slave[slave].misoPin);
  }
}


// Deactivates all SPI peripheral CS/CE lines
//
void SpiMaster::_releaseAllCs()
{
  for (uint8_t slave = 0; slave < _nextSlave; slave++)
  {
    _deactivateCsCe(slave);
  }
}

// Selects the active peripheral and configures the SPI for it
//  Activates the appropriate CS/CE line based on slave's registration
// ***** Make sure SPI is not busy/in use before switching things around! *****
void SpiMaster::_selectPeripheral(const uint8_t slave)
{
  // We won't waste time switching things around if we don't need to
  if (_selectedSlave != slave)
  {
    // if we are selecting a slave...and not just deactivating CS/CE lines...
    if (slave != cNoSelectedSlave)
    {
      _deactivateCsCe(_selectedSlave);
      // Reset SPI, SPI_CR1 register cleared, SPI is disabled
      spi_disable(_params.spi);

      _deconfigureMiso(_selectedSlave);

      _selectedSlave = slave;    // save for later!

      _configureMiso(_selectedSlave);
      _configureMaster(_selectedSlave);
      _activateCsCe(_selectedSlave);

      // Enable SPI1 peripheral
      spi_enable(_params.spi);
    }
    else
    {
      _deactivateCsCe(_selectedSlave);

      _selectedSlave = slave;    // save for later!
    }
  }
}


SpiReqAck SpiMaster::queueTransfer(SpiTransferReq* request)
{
  SpiReqAck status = SpiReqAck::SpiReqAckBusy;

  // nvic_disable_irq(cDmaIrqSpi1);
  // nvic_disable_irq(cTubePwmTimerIrq);

  if (_transferQueue[1] == nullptr)
  {
    request->state = SpiReqAck::SpiReqAckQueued;
    _transferQueue[1] = request;
    status = SpiReqAck::SpiReqAckOk;
  }

  // nvic_enable_irq(cTubePwmTimerIrq);
  // nvic_enable_irq(cDmaIrqSpi1);

  return status;
}


// Hardware::HwReqAck SpiMaster::transfer(const Hardware::SpiPeripheral peripheral, uint8_t *bufferIn, uint8_t *bufferOut, const uint16_t length)
SpiReqAck SpiMaster::transfer(SpiTransferReq* request)
{
  uint32_t dmaEnable = 0;
  volatile uint8_t temp_data __attribute__ ((unused));

  // ensure the timer interrupt doesn't squash another transfer
  nvic_disable_irq(Hardware::cDmaIrqSpi1);
  nvic_disable_irq(Hardware::cTubePwmTimerIrq);

  if (_selectedSlave != 0)
  {
    // aaaannnnddd reenable it
    nvic_enable_irq(Hardware::cTubePwmTimerIrq);
    nvic_enable_irq(Hardware::cDmaIrqSpi1);
    // Let the caller know it was busy if so
    return Hardware::HwReqAck::HwReqAckBusy;
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
  // Hardware::spiSelectPeripheral(peripheral);

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

  // aaaannnnddd reenable these
  nvic_enable_irq(Hardware::cTubePwmTimerIrq);
  nvic_enable_irq(Hardware::cDmaIrqSpi1);

  return SpiReqAck::SpiReqAckOk;
}


void SpiMaster::transferComplete()
{

}


bool SpiMaster::busy()
{
  return 0;
}


}
