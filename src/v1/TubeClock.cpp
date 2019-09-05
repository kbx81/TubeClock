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
#include "Hardware.h"
#include "Animator.h"
#include "Application.h"
#include "DisplayManager.h"
#include "Dmx-512-Controller.h"
#include "Dmx-512-Rx.h"
#include "GpsReceiver.h"
#include "InfraredRemote.h"
#include "Keys.h"


using namespace kbxTubeClock;


/* ADC DMA */
void dma1_channel1_isr(void)
{
	Hardware::dmaCh1Isr();
}


/* SPI receive completed with DMA */
void dma1_channel2_3_dma2_channel1_2_isr(void)
{
	Hardware::dmaCh2to3Isr();
}


/* I2C and USART transmit/receive completed with DMA */
void dma1_channel4_7_dma2_channel3_5_isr()
{
	Hardware::dmaCh4to7Isr();
}


/* EXTI 4-15 interrupt request (for PPS/SQW_In) */
void exti4_15_isr()
{
	Hardware::exti415Isr();
}


/* Called when systick fires */
void sys_tick_handler(void)
{
	Hardware::systickIsr();
	Keys::repeatHandler();
  Dmx512Controller::strobeTimer();
  DisplayManager::tick();
	Animator::tick();
	Application::tick();
}


/* Timer 2 interrupt -- used for generating PWM for tubes */
void tim2_isr()
{
	DisplayManager::tickPWM();
	Hardware::tim2Isr();
}


/* Timer 7 interrupt -- used for IR remote signal monitoring */
void tim7_isr()
{
	InfraredRemote::overflow();
  Hardware::tim7Isr();
}


/* Timer 15 interrupt -- used for DMX-512 break length measurement */
void tim15_isr()
{
	Dmx512Rx::timerUartIsr();
	Hardware::tim15Isr();
}


/* Timer 16 interrupt -- used for DMX-512 signal monitoring */
void tim16_isr()
{
	Dmx512Rx::timerSupervisorIsr();
	Hardware::tim16Isr();
}


/* TSC */
void tsc_isr(void)
{
	Hardware::tscIsr();
}


/* USART1 */
void usart1_isr(void)
{
	GpsReceiver::rxIsr();
	Hardware::usart1Isr();
}


/* USART1 */
void usart2_isr(void)
{
	Dmx512Rx::rxIsr();
	Hardware::usart2Isr();
}


/* Where all life begins...more or less */
int main()
{
	Hardware::initialize();

  Application::initialize();

  Application::loop();
}
