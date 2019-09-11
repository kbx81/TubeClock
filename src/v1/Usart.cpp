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
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/usart.h>

#include "Hardware.h"
#include "Usart.h"

#if HARDWARE_VERSION == 1
  #include "Hardware_v1.h"
#else
  #error HARDWARE_VERSION must be defined with a value of 1
#endif


namespace kbxTubeClock {


Usart::Usart()
: _params{0, 0, 0, 0}
{
}


void Usart::initialize(const UsartParams* usartInit)
{
  _params.usart           = usartInit->usart;
  _params.dmaController   = usartInit->dmaController;
  _params.channelRx       = usartInit->channelRx;
  _params.channelTx       = usartInit->channelTx;
}


void Usart::configure(const UsartTransferParams* usartTransferParams)
{
  // Set up USART parameters
  usart_set_baudrate(_params.usart, usartTransferParams->baudRate);
  usart_set_databits(_params.usart, usartTransferParams->dataBits);
  usart_set_stopbits(_params.usart, usartTransferParams->stopBits);
	usart_set_parity(_params.usart, usartTransferParams->parity);
	usart_set_mode(_params.usart, usartTransferParams->mode);
	usart_set_flow_control(_params.usart, usartTransferParams->flowControl);

  USART_CR2(_params.usart) |= (usartTransferParams->autoBaudMode & ((USART_CR2_ABRMOD_MASK << USART_CR2_ABRMOD_SHIFT) | USART_CR2_ABREN));

  if (usartTransferParams->driverEnableMode == true)
  {
    USART_CR3(_params.usart) |= USART_CR3_DEM;
  }
}


Usart::UsartReqAck Usart::receive(UsartTransferReq* request)
{
  if ((DMA_ISR(_params.dmaController) & DMA_ISR_TCIF(_params.channelRx))
      || !(USART_CR3(_params.usart) & USART_CR3_DMAR))
  {
    // Reset DMA channel
    dma_channel_reset(_params.dmaController, _params.channelRx);
    // Set up Rx DMA, note it has higher priority to avoid overrun
    dma_set_peripheral_address(_params.dmaController, _params.channelRx, (uint32_t)&USART2_RDR);
    dma_set_memory_address(_params.dmaController, _params.channelRx, (uint32_t)request->buffer);
    dma_set_number_of_data(_params.dmaController, _params.channelRx, request->length);
    dma_set_read_from_peripheral(_params.dmaController, _params.channelRx);
    dma_enable_memory_increment_mode(_params.dmaController, _params.channelRx);
    dma_set_peripheral_size(_params.dmaController, _params.channelRx, DMA_CCR_PSIZE_8BIT);
    dma_set_memory_size(_params.dmaController, _params.channelRx, DMA_CCR_MSIZE_8BIT);
    dma_set_priority(_params.dmaController, _params.channelRx, DMA_CCR_PL_VERY_HIGH);
    // Enable DMA transfer complete interrupt
    dma_enable_transfer_complete_interrupt(_params.dmaController, _params.channelRx);
    // Activate DMA channel
    dma_enable_channel(_params.dmaController, _params.channelRx);

    usart_enable_rx_dma(_params.usart);

    return Usart::UsartReqAck::UsartReqAckOk;
  }
  return Usart::UsartReqAck::UsartReqAckBusy;
}


Usart::UsartReqAck Usart::transmit(UsartTransferReq* request)
{
  if (USART_ISR(_params.usart) & USART_ISR_TC)
  {
    // Reset DMA channel
    dma_channel_reset(_params.dmaController, _params.channelTx);
    // Set up tx DMA
    dma_set_peripheral_address(_params.dmaController, _params.channelTx, (uint32_t)&USART1_TDR);
    dma_set_memory_address(_params.dmaController, _params.channelTx, (uint32_t)request->buffer);
    dma_set_number_of_data(_params.dmaController, _params.channelTx, request->length);
    dma_set_read_from_memory(_params.dmaController, _params.channelTx);
    dma_enable_memory_increment_mode(_params.dmaController, _params.channelTx);
    dma_set_peripheral_size(_params.dmaController, _params.channelTx, DMA_CCR_PSIZE_8BIT);
    dma_set_memory_size(_params.dmaController, _params.channelTx, DMA_CCR_MSIZE_8BIT);
    dma_set_priority(_params.dmaController, _params.channelTx, DMA_CCR_PL_HIGH);
    // Enable DMA transfer complete interrupt
    dma_enable_transfer_complete_interrupt(_params.dmaController, _params.channelTx);
    // Activate DMA channel
    dma_enable_channel(_params.dmaController, _params.channelTx);

    usart_enable_tx_dma(_params.usart);

    return Usart::UsartReqAck::UsartReqAckOk;
  }
  return Usart::UsartReqAck::UsartReqAckBusy;
}


void Usart::dmaComplete()
{

}


}
