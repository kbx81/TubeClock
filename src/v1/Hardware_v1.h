//
// kbx81's tube clock Hardware class pin definitions
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
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/tsc.h>
#include <libopencm3/stm32/usart.h>


namespace kbxTubeClock {

namespace Hardware {

  // USART assignments
  //
  static const auto cGpsUsart = USART1;
  static const auto cDmxUsart = USART2;

  // USART Tx and Rx ports, pins, & IRQs
  //
  // static const auto cUsart1Port = GPIOA;
  static const auto cUsart1RxPort = GPIOA;
  static const auto cUsart1RxPin  = GPIO10;
  static const auto cUsart1RxAF   = GPIO_AF1;
  static const auto cUsart1TxPort = GPIOB;
  static const auto cUsart1TxPin  = GPIO6;
  static const auto cUsart1TxAF   = GPIO_AF0;
  static const auto cUsart1Irq    = NVIC_USART1_IRQ;

  static const auto cUsart2Port   = GPIOA;
  // static const auto cUsart2RxPort = GPIOA;
  static const auto cUsart2RxPin  = GPIO3;
  // static const auto cUsart2RxAF   = GPIO_AF1;
  // static const auto cUsart2TxPort = GPIOA;
  static const auto cUsart2TxPin  = GPIO2;
  // static const auto cUsart2TxAF   = GPIO_AF1;
  // static const auto cUsart2DePort = GPIOA;
  static const auto cUsart2DePin  = GPIO1;
  // static const auto cUsart2DeAF   = GPIO_AF1;
  static const auto cUsart2AF     = GPIO_AF1;
  static const auto cUsart2Irq    = NVIC_USART2_IRQ;

  // external alarm input port(s) & pins
  //
  static const auto     cAlarmInputPort     = GPIOC;
  static const uint8_t  cAlarmInputPinCount = 4;
  static const uint16_t cAlarmInputPins[cAlarmInputPinCount] = { GPIO1, GPIO4, GPIO9, GPIO13 };

  // buzzer I/O port & pin
  //
  static const auto cBuzzerPort    = GPIOA;
  static const auto cBuzzerPin     = GPIO8;
  static const auto cBuzzerTimer   = TIM1;
  static const auto cBuzzerTimerOC = TIM_OC1;

  // status LED I/O port(s) & pins
  //
  static const auto cLedPort    = GPIOC;
  static const auto cLed0Pin    = GPIO6;
  static const auto cLed1Pin    = GPIO7;
  static const auto cLed2Pin    = GPIO8;

  static const auto cLedPortAF  = GPIO_AF0;

  static const auto cLedPwmTimer = TIM3;
  static const auto cLed0PwmOc   = TIM_OC1;
  static const auto cLed1PwmOc   = TIM_OC2;
  static const auto cLed2PwmOc   = TIM_OC3;

  // timer used for tube display PWM generation and associated IRQ
  //
  static const auto cTubePwmTimer    = TIM2;
  static const auto cTubePwmTimerIrq = NVIC_TIM2_IRQ;

  // phototransistor I/O port, pin, and ADC channel
  //
  static const auto cPhototransistorPort = GPIOC;
  static const auto cPhototransistorPin  = GPIO0;
  static const uint8_t cPhototransistorChannel = 10;

  // IR demodulator I/O port, pin, and timer
  //
  static const auto cIrPort     = GPIOB;
  static const auto cIrPin      = GPIO15;
  static const auto cIrTimer    = TIM7;
  static const auto cIrTimerIrq = NVIC_TIM7_IRQ;

  // Pulse-Per-Second (SQW) I/O port, pin, & IRQ
  //
  static const auto cPpsPort = GPIOC;
  static const auto cPpsPin  = GPIO13;
  static const auto cPpsIrq  = NVIC_EXTI4_15_IRQ;

  // I2C ports & pins
  //
  static const auto cI2c1Port   = GPIOB;
  static const auto cI2c1SlcPin = GPIO8;
  static const auto cI2c1SdaPin = GPIO9;

  // SPI ports & pins
  //
  static const auto cSpi1Port           = GPIOA;
  static const auto cSpi1AltPort        = GPIOB;
  static const auto cSpi1SckPin         = GPIO5;
  static const auto cSpi1MisoPin        = GPIO6;
  static const auto cSpi1MosiPin        = GPIO7;
  static const auto cSpi1MisoDisplayPin = GPIO4;

  // SPI NSS I/O port & pins
  //
  static const auto cNssPort           = GPIOC;
  static const auto cNssDisplayPin     = GPIO10;
  static const auto cNssRtcPin         = GPIO11;
  static const auto cNssTemperaturePin = GPIO12;

  // driver blank I/O port & pin
  //   on the TLC5951 this is named XBLNK
  static const auto cBlankDisplayPort = GPIOB;
  static const auto cBlankDisplayPin  = GPIO3;

  // high voltage shutdown I/O port & pin
  //
  static const auto cHvShutdownPort = GPIOA;
  static const auto cHvShutdownPin  = GPIO4;

  // DMA channel assignments and remapping via SYSCFG_CFGR1
  //
  static const auto cAdcDmaChannel      = DMA_CHANNEL1;
  static const auto cSpi1RxDmaChannel   = DMA_CHANNEL2;
  static const auto cSpi1TxDmaChannel   = DMA_CHANNEL3;
  static const auto cI2c1RxDmaChannel   = 0;
  static const auto cI2c1TxDmaChannel   = 0;
  static const auto cUsart1RxDmaChannel = DMA_CHANNEL5;
  static const auto cUsart1TxDmaChannel = DMA_CHANNEL4;
  static const auto cUsart2RxDmaChannel = DMA_CHANNEL6;
  static const auto cUsart2TxDmaChannel = DMA_CHANNEL7;
  // USART1 and USART2 TX and RX remap bits set in SYSCFG_CFGR1 since
  //  SPI1 ties up the USART's default DMA channels
  static const auto cDmaChannelRemaps = SYSCFG_CFGR1_USART1_RX_DMA_RMP |
                                        SYSCFG_CFGR1_USART1_TX_DMA_RMP |
                                        SYSCFG_CFGR1_USART2_DMA_RMP;

  // DMA interrupts we use
  //
  static const auto cDmaIrqSpi1   = NVIC_DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_IRQ;
  static const auto cDmaIrqUsarts = NVIC_DMA1_CHANNEL4_7_DMA2_CHANNEL3_5_IRQ;

  // number of TSC channels we're using
  //
  static const uint8_t cTscChannelCount = 6;

  // number of TSC channels we're using per group
  //
  static const uint8_t cTscChannelsPerGroup = 3;

  // TSC IRQ
  //
  static const uint8_t cTscIrq = NVIC_TSC_IRQ;

  // touch sense controller I/O groups
  //
  static const auto cTscGroupControlBits = TSC_IOGCSR_GxE(3) | TSC_IOGCSR_GxE(6);

  // touch sensing controller I/O port
  //
  static const auto cTscPort  = GPIOB;
  static const auto cTscPortC = GPIOC;

  // pins used for sense capacitors
  //
  static const auto cTscSamplingCapPins = GPIO2 | GPIO14;

  // pins used for touch keys
  //
  static const auto cTscTouchKeyPins  = GPIO0 | GPIO1 | GPIO11 | GPIO12 | GPIO13;
  static const auto cTscTouchKeyPinsC = GPIO5;

  // touch sense controller I/O groups
  //
  static const uint32_t cTscChannelControlBits[] = { (1 << 8) | (1 << 20),
                                                     (1 << 9) | (1 << 21),
                                                     (1 << 10) | (1 << 22) };

  // touch sense controller I/O groups
  //
  static const uint32_t cTscSamplingControlBits = (1 << 11) | (1 << 23);

}

}
