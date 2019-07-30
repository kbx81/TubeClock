//
// kbx81's tube clock DMX-512 receiver class
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
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>
#include "Dmx-512-Packet.h"
#include "Dmx-512-Rx.h"
#include "Hardware.h"

#if HARDWARE_VERSION == 1
  #include "Hardware_v1.h"
#else
  #error HARDWARE_VERSION must be defined with a value of 1
#endif


namespace kbxTubeClock {

namespace Dmx512Rx {


// DMX-512 receive states
//
enum DmxRxState : uint8_t {
  DmxRxBreakValid,
  DmxRxReceiving,
  DmxRxComplete,
  DmxRxError
};


// DMX-512 break time (minimum) in microseconds
//
static const uint8_t cBreakDuration = 88;

// DMX-512 mark-after-break (MAB) time (minimum) in microseconds
//
static const uint8_t cMarkAfterBreakDuration = 8;

// Maximum allowed delay between packets before assuming loss of signal
//   Time unit depends on timer configuration. Here, it is in milliseconds.
static const uint8_t cMaximumTimeBetweenPackets = 250;

// Required number of packet checks passed before signal is considered active
//
static const uint8_t cMinimumNumberOfPacketsToActive = 5;

// USARTs I/O port
//
static const uint32_t cUsartPort = GPIOA;

// DMX-512 received data buffers
//
static Dmx512Packet* _pDmxPacketActive = new Dmx512Packet;
static Dmx512Packet* _pDmxPacketReady = new Dmx512Packet;
static Dmx512Packet* _pDmxPacketLast = new Dmx512Packet;

// state of the DMX-512 receive process
//
volatile static DmxRxState _dmxState = DmxRxState::DmxRxError;

// state of the DMX-512 signal
//
volatile static int8_t _dmxSignalCheckCount = 0;

// state of the DMX-512 signal
//
volatile static bool _dmxSignalActive = false;

// Number of packets received since last timer check
//
volatile static uint8_t _dmxPacketCount = 0;

// Number of checks that have demonstrated an active DMX-512 signal
//
volatile static uint8_t _dmxPassedChecks = 0;


void _readSerialSetup()
{
  // we would do this but we have to wait for the break detection to enable DMA
  // Hardware::readSerial(USART2, 513, _pDmxPacketActive);

  dma_channel_reset(DMA1, Hardware::cUsart2RxDmaChannel);
  // Set up Rx DMA, note it has higher priority to avoid overrun
  dma_set_peripheral_address(DMA1, Hardware::cUsart2RxDmaChannel, (uint32_t)&USART2_RDR);
  dma_set_memory_address(DMA1, Hardware::cUsart2RxDmaChannel, (uint32_t)_pDmxPacketActive);
  dma_set_number_of_data(DMA1, Hardware::cUsart2RxDmaChannel, 513);
  dma_set_read_from_peripheral(DMA1, Hardware::cUsart2RxDmaChannel);
  dma_enable_memory_increment_mode(DMA1, Hardware::cUsart2RxDmaChannel);
  dma_set_peripheral_size(DMA1, Hardware::cUsart2RxDmaChannel, DMA_CCR_PSIZE_8BIT);
  dma_set_memory_size(DMA1, Hardware::cUsart2RxDmaChannel, DMA_CCR_MSIZE_8BIT);
  dma_set_priority(DMA1, Hardware::cUsart2RxDmaChannel, DMA_CCR_PL_VERY_HIGH);
  // Enable DMA transfer complete interrupt
	dma_enable_transfer_complete_interrupt(DMA1, Hardware::cUsart2RxDmaChannel);
}


void initialize()
{
  // Other important stuff should be done by Hardware::init

  // configure the break/mab detection timer
  rcc_periph_reset_pulse(RST_TIM15);
  rcc_periph_reset_pulse(RST_TIM16);
  // timer_reset(TIM15);
  // timer_reset(TIM16);
  timer_set_mode(TIM15, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_set_mode(TIM16, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_set_prescaler(TIM15, 48);     // microseconds
  timer_set_prescaler(TIM16, 48000);  // milliseconds
  timer_set_repetition_counter(TIM15, 0);
  timer_set_repetition_counter(TIM16, 0);
  timer_continuous_mode(TIM15);
  timer_continuous_mode(TIM16);
  timer_set_period(TIM15, 0xffff);
  timer_set_period(TIM16, cMaximumTimeBetweenPackets);

  // timer_ic_set_input(TIM15, TIM_IC1, TIM_IC_IN_TI2);
  // timer_ic_set_input(TIM15, TIM_IC2, TIM_IC_IN_TI2);
  TIM_CCMR1(TIM15) = TIM_CCMR1_CC1S_IN_TI2 | TIM_CCMR1_CC2S_IN_TI2;

  timer_ic_set_prescaler(TIM15, TIM_IC1, TIM_IC_PSC_OFF);
  timer_ic_set_prescaler(TIM15, TIM_IC2, TIM_IC_PSC_OFF);

  // timer_ic_enable(TIM15, TIM_IC1);
  // timer_ic_enable(TIM15, TIM_IC2);
  // enable capture 1 and 2, trigger CC1 on rising edge and CC2 on falling edge
  TIM_CCER(TIM15) = TIM_CCER_CC2P | TIM_CCER_CC2E | TIM_CCER_CC1E;

  timer_enable_irq(TIM15, TIM_DIER_CC1IE | TIM_DIER_CC2IE);
  timer_enable_irq(TIM16, TIM_DIER_UIE);

  timer_enable_counter(TIM15);
  timer_enable_counter(TIM16);

  nvic_set_priority(NVIC_TIM15_IRQ, 0);
	nvic_enable_irq(NVIC_TIM15_IRQ);
  nvic_set_priority(NVIC_TIM16_IRQ, 128);
	nvic_enable_irq(NVIC_TIM16_IRQ);

  USART2_CR1 &= ~USART_CR1_RE;

  // Swap USART2 Rx pin to timer to measure the break and mark after break
  gpio_set_af(cUsartPort, GPIO_AF0, GPIO3);
}


bool signalIsActive()
{
  return _dmxSignalActive;
}


Dmx512Packet* getLastPacket()
{
  // check if the "Ready" packet is actually ready
  if (_pDmxPacketReady->getBufferState() == Dmx512Packet::DmxBufferValid)
  {
    // clean up the old object if it exists
    if (_pDmxPacketLast != nullptr)
    {
      delete _pDmxPacketLast;
    }
    // save the current "Ready" packet as the "Last" (returnetd) packet
    _pDmxPacketLast = _pDmxPacketReady;
    // create a new Dmx512Packet object for the next "Ready" packet
    _pDmxPacketReady = new Dmx512Packet;
  }

  return _pDmxPacketLast;
}


void rxIsr()
{
  // Here we just deal with error flags. Ideally we only see the framing error
  //  flag (FE) at the start of each DMX-512 packet. Regardless, any errors just
  //  result in the DmxRxError state being set, which ultimately results in the
  //  packet being discarded after its reception has been completed.
  if (USART2_ISR & (USART_ISR_FE | USART_ISR_NF | USART_ISR_ORE | USART_ISR_RXNE))
  {
    // Clear the RXNE flag -- it may be set with the flags above
    USART2_RQR = USART_RQR_RXFRQ;
    // Disable the receiver
    USART2_CR1 &= ~USART_CR1_RE;

    // Reset timer counter to start break time detection
    timer_generate_event(TIM15, TIM_EGR_UG);
    // Swap USART2 Rx pin to the timer to measure the break and mark after break
  	gpio_set_af(cUsartPort, GPIO_AF0, GPIO3);

    _dmxState = DmxRxState::DmxRxError;
  }

}


void rxCompleteIsr()
{
  Dmx512Packet* temp;
  if (_dmxState == DmxRxState::DmxRxReceiving)
  {
    // If we finish and _dmxState is still DmxRxReceiving, no errors occured so
    //  the data in the buffer should be valid. We flag it as such and swap
    //  pointers so that the next packet goes into the other buffer, instead.
    _pDmxPacketActive->setBufferState(Dmx512Packet::DmxBufferValid);
    // now swap buffer pointers
    temp = _pDmxPacketReady;
    _pDmxPacketReady = _pDmxPacketActive;
    _pDmxPacketActive = temp;
    // set receiver state
    _dmxState = DmxRxState::DmxRxComplete;
    // the now-active buffer is now being written
    _pDmxPacketActive->setBufferState(Dmx512Packet::DmxBufferBeingWritten);
  }
  else
  {
    _pDmxPacketActive->setBufferState(Dmx512Packet::DmxBufferInvalid);
  }
}


// It is expected that _dmxState == DmxRxError as this runs; this state should
// be set when the UART triggers the framing error. If the break is of a
// sufficient duration, it will advance to the DmxRxBreakValid state. Once in
// the DmxRxBreakValid state, it will advance to the DmxRxReceiving state if
// the mark after break (MAB) is of a sufficient duration. In short, it must
// not advance to the DmxRxReceiving state if the break AND mark after break
// are not both of sufficient durations.
void timerUartIsr()
{
  uint16_t breakTime;

  // CC1 is set when the Rx line transitions from low to high
  if (timer_get_flag(TIM15, TIM_SR_CC1IF) == true)
  {
    breakTime = TIM15_CCR1;
    // It takes 10 bits for the UART to generate the frame error; bits are 4 uS
    //  This could probably be a little better but...eh...
    if (breakTime > cBreakDuration - 40)
    {
      _dmxState = DmxRxState::DmxRxBreakValid;
      _readSerialSetup();
      // This part kind of sucks because it blocks, but it enables us to catch
      //  invalid (too short) MABs. CCR2 will capture a value that's too small
      //  if the MAB isn't of a sufficient duration.
      while ((TIM15_CNT - breakTime < cMarkAfterBreakDuration) && (timer_get_flag(TIM15, TIM_SR_CC2IF) == false));
    }
  }

  // CC2 is set when the Rx line transitions from high to low
  if (timer_get_flag(TIM15, TIM_SR_CC2IF) == true)
  {
    // If we get in here, the MAB was too short. We'll clear the status register
    //  flags and just move along.
    TIM15_SR = 0;
    // The break was too short, so we maintain the error status
    _dmxState = DmxRxState::DmxRxError;
    _dmxPacketCount = 0;
  }
  else if (_dmxState == DmxRxState::DmxRxBreakValid)
  {
    // If we get in here, the Rx line did NOT transition to low before the
    //  required duration elapsed and the MAB can be considered valid. If the
    //  break also proved to be of a sufficient duration above, we can give the
    //  pin back to the UART and activate the DMA to begin recieving data.
    gpio_set_af(cUsartPort, GPIO_AF1, GPIO3);

    dma_enable_channel(DMA1, Hardware::cUsart2RxDmaChannel);
    usart_enable_rx_dma(USART2);
    USART2_CR1 |= USART_CR1_RE;

    _dmxState = DmxRxState::DmxRxReceiving;

    _dmxPacketCount++;
    // the AF swap above will have triggered a capture so let's clear the flags
    TIM15_SR = 0;
  }
  // This resets the timer's counter so we can properly detect the duration of
  //  the next bit on the line.
  timer_generate_event(TIM15, TIM_EGR_UG);
}


// This is debatably a little silly but the idea is to create a short delay
//  (about one second) before toggling the state of _dmxSignalActive both just
//  after a signal is detected as well as just after it is lost. This allows the
//  signal to stabilize before the application begins using it and also keeps
//  the application from exiting the DMX-512 view mode in the event of a brief
//  hiccup in the signal (which is not an uncommon occurrence).
void timerSupervisorIsr()
{
  TIM16_SR = 0;

  if ((_dmxPacketCount > 0) && (_dmxPacketCount < 50))
  {
    _dmxSignalCheckCount++;
  }
  else
  {
    _dmxSignalCheckCount--;
    _dmxState = DmxRxState::DmxRxError;
  }

  if (_dmxSignalCheckCount >= cMinimumNumberOfPacketsToActive)
  {
    _dmxSignalCheckCount = cMinimumNumberOfPacketsToActive;
    _dmxSignalActive = true;
  }
  if (_dmxSignalCheckCount < 0)
  {
    _dmxSignalCheckCount = 0;
    _dmxSignalActive = false;
  }

  _dmxPacketCount = 0;
}


}

}
