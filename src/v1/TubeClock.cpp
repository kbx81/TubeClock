//
// kbx81's tube clock
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
#ifdef ENABLE_PROFILING
#include <libopencm3/stm32/gpio.h>
#endif
#include "Hardware.h"
#if HARDWARE_VERSION == 1
#include "Hardware_v1.h"
#else
#error HARDWARE_VERSION must be defined with a value of 1
#endif
#include "Animator.h"
#include "Application.h"
#include "DisplayManager.h"
#include "Dmx-512-Controller.h"
#include "Dmx-512-Rx.h"
#include "GpsReceiver.h"
#include "InfraredRemote.h"
#include "Keys.h"
#include "SerialRemote.h"
#include "UsbSerial.h"

using namespace kbxTubeClock;

// Minimal terminate handler replacing libstdc++'s __verbose_terminate_handler.
// The default pulls in __cxa_demangle (27 KB) and sprintf (23 KB) -- useless
// on bare-metal where there's no console to print to anyway.
namespace __gnu_cxx {
void __verbose_terminate_handler() {
  while (1) {
  }
}
}  // namespace __gnu_cxx

// Stub for the C runtime 'finish' routine. Called by __libc_fini_array during
// shutdown, which is pulled in when static C++ objects with non-trivial
// (virtual) destructors register via __cxa_atexit. The firmware never exits,
// so this is never actually called.
extern "C" void _fini() {}

/* ADC DMA */
void dma1_channel1_isr(void) { Hardware::dmaCh1Isr(); }

/* SPI receive completed with DMA */
void dma1_channel2_3_dma2_channel1_2_isr(void) { Hardware::dmaCh2to3Isr(); }

/* I2C and USART transmit/receive completed with DMA */
void dma1_channel4_7_dma2_channel3_5_isr() { Hardware::dmaCh4to7Isr(); }

/* EXTI 4-15 interrupt request (for PPS/SQW_In) */
void exti4_15_isr() { Hardware::exti415Isr(); }

/* Called when systick fires */
void sys_tick_handler(void) {
  Hardware::systickIsr();
  Keys::repeatHandler();
  Dmx512Controller::strobeTimer();
  DisplayManager::tick();
  Animator::tick();
  Application::tick();
}

/* Timer 2 interrupt -- used for generating PWM for tubes */
void tim2_isr() {
  Hardware::tim2Isr();  // clear UIF flag first to prevent RMW race
#ifdef ENABLE_PROFILING
  gpio_set(Hardware::cProfilingPort, Hardware::cProfilingPin);
#endif
  DisplayManager::tickPWM();
#ifdef ENABLE_PROFILING
  gpio_clear(Hardware::cProfilingPort, Hardware::cProfilingPin);
#endif
}

/* Timer 7 interrupt -- used for IR remote signal monitoring */
void tim7_isr() {
  InfraredRemote::overflow();
  Hardware::tim7Isr();
}

/* Timer 15 interrupt -- used for DMX-512 break length measurement */
void tim15_isr() {
  Dmx512Rx::timerUartIsr();
  Hardware::tim15Isr();
}

/* Timer 16 interrupt -- used for DMX-512 signal monitoring */
void tim16_isr() {
  Dmx512Rx::timerSupervisorIsr();
  Hardware::tim16Isr();
}

/* TSC */
void tsc_isr(void) { Hardware::tscIsr(); }

/* USART1 -- GPS RX and serial remote TX/RX */
void usart1_isr(void) {
  GpsReceiver::rxIsr();
  SerialRemote::rxIsr(USART1);
  SerialRemote::txIsr(USART1);
  Hardware::usart1Isr();
}

/* USB -- CDC-ACM serial interface */
void usb_isr(void) { UsbSerial::poll(); }

/* USART2 -- DMX-512 */
void usart2_isr(void) {
  Dmx512Rx::rxIsr();
  Hardware::usart2Isr();
}

/* USART3/4 -- serial remote control RX (USART3) and TX (USART4) */
void usart3_4_isr(void) {
  SerialRemote::rxIsr(USART3);
  SerialRemote::txIsr(USART4);
  Hardware::usart3_4Isr();
}

/* Where all life begins...more or less */
int main() {
  // Must check before Hardware::initialize() — reads only a RAM variable,
  // no peripheral clocks needed.
  if (Hardware::isBootloaderFlagSet()) {
    Hardware::enterBootloader();
  }

  Hardware::initialize();

  Application::initialize();

  Application::loop();
}
