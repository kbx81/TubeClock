//
// kbx81's tube clock Hardware class
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

// #include <cstddef>
#include <cstdint>
// #include <errno.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/crc.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/iwdg.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/tsc.h>
#include <libopencm3/stm32/usart.h>

#include "Application.h"
#include "DateTime.h"
#include "Display.h"
#include "DisplayManager.h"
#include "Dmx-512-Rx.h"
#include "DS1722.h"
#include "DS3234.h"
#include "GpsReceiver.h"
#include "Hardware.h"
#include "InfraredRemote.h"
#include "LM74.h"
#include "RgbLed.h"

#if HARDWARE_VERSION == 1
  #include "Hardware_v1.h"
#else
  #error HARDWARE_VERSION must be defined with a value of 1
#endif


namespace kbxTubeClock {

namespace Hardware {


  // I2C1 state
  //
  enum I2cState : uint8_t {
    I2cIdle,
    I2cBusy
  };


// USART baud rates
//
static const uint32_t cUsart1BaudRate = 115200;
static const uint32_t cUsart2BaudRate = 250000;

// number of ADC channels we're using
//
static const uint8_t cAdcChannelCount = 4;

// indexes of ADC channels we're DMA'ing into our buffer
//
static const uint8_t cAdcPhototransistor = 0;
static const uint8_t cAdcTemperature = 1;
static const uint8_t cAdcVref = 2;
static const uint8_t cAdcVbat = 3;

// how frequently we will sample things
//
static const uint16_t cAdcSampleInterval = 500;

// number of samples averaged for light level and temp
//
static const uint8_t cAdcSamplesToAverage = 16;

// value used to increase precision in our maths
//
static const int16_t cBaseMultiplier = 1000;

// maximum time to wait before disconnecting Vbat from the ADC's bridge divider
//
static const uint16_t cBatteryMeasuringTimeout = 5000;

// the last page number in the flash memory
//
static const uint32_t cFlashPageNumberMaximum = 63;

// the last page number in the flash memory
//
static const uint32_t cFlashPageSize = 0x800;

// maximum time to wait for an ACK on I2C
//
static const uint16_t cI2cTimeout = 5000;

// maximum number of times we'll fail requests because I2C is busy
//
static const uint8_t cI2cMaxFailBusyCount = 20;

// value by which we multiply the fractional part of temperature sensor readings
//
static const int16_t cTempFracMultiplier = 125;

// minimum frequency allowed for tones
//
static const uint8_t cToneFrequencyMinimum = 92;

// maximum volume level allowed for tones
//
static const uint8_t cToneVolumeMaximum = 7;

// maximum minimum allowed to consider a key pressed/touched
//
static const uint16_t cTscMaximumMinimum = 1536;

// minimum required difference between min and max readings
//
static const uint16_t cTscMinimumDelta = 400;

// number of samples averaged for sensors
//
static const uint8_t cTscSamplesToAverage = 32;

// calibration voltage used (3.3 volts)
//
static const int32_t cVddCalibrationVoltage = 3300;


// tracks which array element the next sample is to be written to
//
static uint8_t _adcSampleCounter = 0;

// tracks when to take a sample
//
static uint16_t _adcSampleTimer = 0;

// samples averaged for light
//
static uint16_t _adcSampleSetLight[cAdcSamplesToAverage];

// samples averaged for temperature
//
static uint16_t _adcSampleSetTemp[cAdcSamplesToAverage];

// samples averaged for VddA voltage
//
static uint16_t _adcSampleSetVoltageVddA[cAdcSamplesToAverage];

// samples averaged for battery voltage
//
static uint16_t _adcSampleSetVoltageBatt[cAdcSamplesToAverage];

// threshold above which we do not BLANK during display refreshes
//  used when autoAdjustIntensities is true
static uint16_t _blankingThreshold = 300;

// counter used by voltageBatt() to automatically disconnect the ADC's bridge divider
//
volatile static uint16_t _batteryMeasuringCounter = 0;

// a place for data from the ADC to live
//
static uint16_t _bufferADC[cAdcChannelCount];

// buttons that have change states since the last sampling
//
static uint8_t _buttonsChanged = 0;

// button/touchkey states (one bit per button)
//
static uint8_t _buttonStates = 0;

// the button states the last time they were sampled
//
static uint8_t _buttonStatesPrevious = 0;

// counter used by delay()
//
volatile static uint32_t _delayCounter = 0;
volatile static uint32_t _delayCounterNB = 0;

// tracks how many times tx/rx requests were blocked because the I2C is busy
//
static uint8_t _i2cBusyFailCount = 0;

// device address I2C must use to resume communication
//
volatile static uint8_t _i2cAddr;

// buffer used when I2C resumes communication
//
static uint8_t *_i2cBufferRx;

// number of bytes to transfer when I2C resumes communication
//
volatile static size_t _i2cNumberRx;

// tracks what the I2C is doing
//
volatile static I2cState _i2cState = I2cState::I2cIdle;

// tracks the last hour in which RTCs were synchronized with the GPS time
//
static uint8_t _lastRtcGpsSyncHour = 255;

// last second at which we incremented the counter
//
static uint8_t _onTimeLastSecond = 0;

// seconds of HV on time
//
static uint32_t _onTimeSecondsCounter = 0;

// tracks what the SPI MISO is set to
//
volatile static SpiPeripheral _spiActivePeripheral = SpiPeripheral::None;

// the next transfer the SPI needs to do
//
volatile static SpiTransferReq* _spiXferQueue[4] = { nullptr, nullptr, nullptr, nullptr };

// provides a way to calibrate the temperature indication
//
static int8_t _temperatureAdjustment = -9;

// tracks when to silence a tone
//
static uint16_t _toneTimer = 0;

// contains the next tone's duration and frequency, if any (not zero)
//
static uint16_t _toneTimerNext = 0;
static uint16_t _toneFrequencyNext = 0;

// volume level for tones
//
static uint8_t _toneVolume = 0;

// tracks which array element the next sample is to be written to
//
static uint8_t _tscSampleCounter = 0;

// samples averaged for buttons
//
static uint16_t _tscSampleSets[cTscChannelCount][cTscSamplesToAverage];

// minimum read values for buttons
//
static uint16_t _tscMinimums[cTscChannelCount];

// maximum read values for buttons
//
static uint16_t _tscMaximums[cTscChannelCount];

// tracks which TSC channel(s) we'll sample next
//
static uint8_t _tscChannelCounter = 0;

// a place for data from the TSC to live
//
static uint16_t _tscAcquisitionValues[cTscChannelCount];

// date & time stored by Refresh()
//
static DateTime _currentDateTime;

// contains the temperature in degrees Celsius times cBaseMultiplier
//
static int32_t _temperatureXcBaseMultiplier = 0;

// state of the BLANK pin
//
static bool _displayBlankingState = false;

// indicates to setDisplayBlankPin() that it should NOT clear the BLANK pin as
//  a display write is in progress with flicker reduction active
volatile static bool _displayBlankForWrite = false;

// true if DS3234 is detected
//
static bool _externalRtcConnected = false;

// inverts NSS state during SPI transfers (for DS1722) if true
//
static bool _invertCsForTemperatureSensor = false;

// state of the HV SHUTDOWN pin
//
static bool _hvState = false;

// true if the external RTC needs to be refreshed
//
volatile static bool _refreshRTCNow = false;

// true if the external temperature sensor needs to be refreshed
//
volatile static bool _refreshTempNow = false;

// indicates if the app can rely on a periodic interrupt (PPS) to trigger
//  external RTC and temperature sensor refreshes
static PeripheralRefreshTrigger _peripheralRefreshTrigger = PeripheralRefreshTrigger::SysTick;

// true if RTC is set/valid
//
static bool _rtcIsSet = false;

// true if temp sensor is detected
//
static TempSensorType _externalTemperatureSensor = TempSensorType::NoTempSensor;

// a string used for USART tx
//
// char _buffer[80];
// writeSerial(USART1, sprintf(_buffer, "thing = %d\r\n", thing), _buffer);


// set up the ADC
//
void _adcSetup()
{
  uint8_t channelArray[] = {cPhototransistorChannel, ADC_CHANNEL_TEMP, ADC_CHANNEL_VREF, ADC_CHANNEL_VBAT};

	adc_power_off(ADC1);

	adc_calibrate(ADC1);
  while ((ADC_CR(ADC1) & ADC_CR_ADCAL) != 0);

  adc_power_on(ADC1);
  while ((ADC_ISR(ADC1) & ADC_ISR_ADRDY) == 0);

  adc_set_clk_source(ADC1, ADC_CLKSOURCE_ADC);
	adc_set_operation_mode(ADC1, ADC_MODE_SCAN);
	adc_disable_external_trigger_regular(ADC1);
	adc_set_right_aligned(ADC1);
	adc_enable_temperature_sensor();
  adc_enable_vrefint();
  // adc_enable_vbat_sensor();  // don't do this here!
  adc_set_sample_time_on_all_channels(ADC1, ADC_SMPTIME_239DOT5);
	adc_set_regular_sequence(ADC1, cAdcChannelCount, channelArray);
	adc_set_resolution(ADC1, ADC_RESOLUTION_12BIT);
	adc_disable_analog_watchdog(ADC1);
  adc_set_continuous_conversion_mode(ADC1);
  ADC_CFGR1(ADC1) |= ADC_CFGR1_DMACFG;
  adc_enable_dma(ADC1);

  ADC_CR(ADC1) |= ADC_CR_ADSTART;   // adc_start_conversion_regular blocks :(
}


// Set STM32's clock to 48 MHz from 8 MHz HSE oscillator
//
void _clockSetup()
{
  rcc_clock_setup_in_hse_8mhz_out_48mhz();

  // Enable clocks to various subsystems we'll need
	// PWR must be enabled first as we can't disable the backup domain protection
  //  without it online
  rcc_periph_clock_enable(RCC_PWR);

  // SYSCFG is needed to remap USART DMA channels
  rcc_periph_clock_enable(RCC_SYSCFG_COMP);

  rcc_periph_clock_enable(RCC_DMA);

  rcc_periph_clock_enable(RCC_PWR);

  rcc_periph_clock_enable(RCC_CRC);

  rcc_periph_clock_enable(RCC_TIM1);
  rcc_periph_clock_enable(RCC_TIM2);
  rcc_periph_clock_enable(RCC_TIM3);
  rcc_periph_clock_enable(RCC_TIM7);
  rcc_periph_clock_enable(RCC_TIM15);
  rcc_periph_clock_enable(RCC_TIM16);

  rcc_periph_clock_enable(RCC_ADC);

  rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
  rcc_periph_clock_enable(RCC_GPIOD);

  // Set SYSCLK (not the default HSI) as I2C1's clock source
  // rcc_set_i2c_clock_hsi(I2C1);
	// RCC_CFGR3 |= RCC_CFGR3_I2C1SW;
  // rcc_periph_clock_enable(RCC_I2C1);

  rcc_periph_clock_enable(RCC_SPI1);
  rcc_periph_clock_enable(RCC_SPI2);
  rcc_periph_clock_enable(RCC_USART1);
  rcc_periph_clock_enable(RCC_USART2);

  rcc_periph_clock_enable(RCC_TSC);
}


// Configure DMA
//
void _dmaSetup()
{
  // Remap DMA channels as necessary
  SYSCFG_CFGR1 |= cDmaChannelRemaps;

  // Reset DMA channels
  dma_channel_reset(DMA1, DMA_CHANNEL1);
  dma_channel_reset(DMA1, DMA_CHANNEL2);
  dma_channel_reset(DMA1, DMA_CHANNEL3);
  dma_channel_reset(DMA1, DMA_CHANNEL4);
  dma_channel_reset(DMA1, DMA_CHANNEL5);
  dma_channel_reset(DMA1, DMA_CHANNEL6);
  dma_channel_reset(DMA1, DMA_CHANNEL7);

  // Set up ADC DMA -- it has higher priority to avoid overrun
  dma_set_peripheral_address(DMA1, cAdcDmaChannel, (uint32_t)&ADC1_DR);
  dma_set_memory_address(DMA1, cAdcDmaChannel, (uint32_t)_bufferADC);
  dma_set_number_of_data(DMA1, cAdcDmaChannel, cAdcChannelCount);
  dma_set_read_from_peripheral(DMA1, cAdcDmaChannel);
  dma_enable_memory_increment_mode(DMA1, cAdcDmaChannel);
  dma_set_peripheral_size(DMA1, cAdcDmaChannel, DMA_CCR_PSIZE_16BIT);
  dma_set_memory_size(DMA1, cAdcDmaChannel, DMA_CCR_MSIZE_16BIT);
  dma_set_priority(DMA1, cAdcDmaChannel, DMA_CCR_PL_VERY_HIGH);
  dma_enable_circular_mode(DMA1, cAdcDmaChannel);
  dma_enable_channel(DMA1, cAdcDmaChannel);
}


// Configure GPIOs
//
void _gpioSetup()
{
  // Configure the SPI NSS pins for the HV drivers, RTC, and temp sensor
  gpio_mode_setup(cNssPort, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, cNssDisplayPin | cNssRtcPin | cNssTemperaturePin);
  // Configure the BLANK pin for the LED drivers
  gpio_mode_setup(cBlankDisplayPort, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, cBlankDisplayPin);
  // Configure the analog input pin for the phototransistor
  gpio_mode_setup(cPhototransistorPort, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, cPhototransistorPin);
  // Configure the SHUTDOWN pin for the boost converter
  gpio_mode_setup(cHvShutdownPort, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, cHvShutdownPin);

  // Set the inital pin states for the pins we configured above
  gpio_set(cHvShutdownPort, cHvShutdownPin);
  gpio_set(cNssPort, cNssDisplayPin | cNssRtcPin | cNssTemperaturePin);
  gpio_clear(cBlankDisplayPort, cBlankDisplayPin);

  // Next, configure the external alarm input pins
  for (uint8_t i = 0; i < cAlarmInputPinCount; i++)
  {
    gpio_mode_setup(cAlarmInputPort, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, cAlarmInputPins[i]);
  }

  // Next, configure the external PPS/SQW_In pulse-per-second input pin and interrupt
  gpio_mode_setup(cPpsPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, cPpsPin);
  exti_select_source(EXTI13, cPpsPort);
  exti_set_trigger(EXTI13, EXTI_TRIGGER_BOTH);
  exti_enable_request(EXTI13);

  // Finally, configure the external IR remote sensor input pin and interrupt
  gpio_mode_setup(cIrPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, cIrPin);
  exti_select_source(EXTI15, cIrPort);
  exti_set_trigger(EXTI15, EXTI_TRIGGER_BOTH);
  exti_enable_request(EXTI15);
}


// Configure the Independent Watchdog Timer
//
void _iwdgSetup()
{
  iwdg_set_period_ms(5000);
  iwdg_start();
}


void _i2cSetup()
{
  // Configure GPIOs SCL=PB8 and SDA=PB9 so we can reset the bus nicely
  gpio_mode_setup(cI2c1Port, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, cI2c1SlcPin | cI2c1SdaPin);
  gpio_set_output_options(cI2c1Port, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, cI2c1SlcPin | cI2c1SdaPin);

  gpio_set(cI2c1Port, cI2c1SlcPin | cI2c1SdaPin);
  delay(1);
  gpio_clear(cI2c1Port, cI2c1SdaPin);
  delay(1);
  gpio_clear(cI2c1Port, cI2c1SlcPin);
  delay(1);
  gpio_set(cI2c1Port, cI2c1SlcPin);
  delay(1);
  gpio_set(cI2c1Port, cI2c1SdaPin);
  delay(1);

  gpio_mode_setup(cI2c1Port, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, cI2c1SdaPin);
  // Clock out any remaining bits any slave is trying to send
  while (gpio_get(cI2c1Port, cI2c1SdaPin) == false)
  {
    gpio_clear(cI2c1Port, cI2c1SlcPin);
    delay(1);
    gpio_set(cI2c1Port, cI2c1SlcPin);
    delay(1);
  }

  // Now configure GPIOs for I2C use
	gpio_mode_setup(cI2c1Port, GPIO_MODE_AF, GPIO_PUPD_PULLUP, cI2c1SlcPin | cI2c1SdaPin);

	// Setup I2C1 pins as alternate function one
	gpio_set_af(cI2c1Port, GPIO_AF1, cI2c1SlcPin | cI2c1SdaPin);

	// Set alternate functions for the SCL and SDA pins of I2C1
  gpio_set_output_options(cI2c1Port, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, cI2c1SlcPin | cI2c1SdaPin);

  // Enable heftier drivers for I2C pins...maybe helpful?
  // SYSCFG_CFGR1 |= (SYSCFG_CFGR1_I2C_PB8_FMP | SYSCFG_CFGR1_I2C_PB9_FMP);

	// Reset/Disable the I2C before changing any configuration
  i2c_reset(I2C1);

  // 400KHz - I2C Fast Mode - SYSCLK is 48 MHz
  i2c_set_speed(I2C1, i2c_speed_fm_400k, 48);

  // Set our slave address in case we should want to receive from other masters
	i2c_set_own_7bit_slave_address(I2C1, 0x11);

  // Configure ANFOFF DNF[3:0] in CR1
	// i2c_enable_analog_filter(I2C1);
	// i2c_set_digital_filter(I2C1, I2C_CR1_DNF_DISABLED);

	// Configure No-Stretch CR1
  //  (only relevant in slave mode, must be disabled in master mode)
	// i2c_disable_stretching(I2C1);

	// Once everything is configured, enable the peripheral
	i2c_peripheral_enable(I2C1);

  _i2cState = I2cState::I2cIdle;
}


void _i2cRecover()
{
  // Ensure status is busy -- it'll be reset to Idle by _i2cSetup()
  _i2cState = I2cState::I2cBusy;

  // Stop/cancel any on-going or pending DMA
  dma_channel_reset(DMA1, cI2c1TxDmaChannel);
  dma_channel_reset(DMA1, cI2c1RxDmaChannel);

  // Disable the I2C before all this
	i2c_peripheral_disable(I2C1);

  _i2cSetup();

  _i2cBusyFailCount = 0;
}


void _nvicSetup()
{
  // nvic_set_priority(NVIC_ADC_COMP_IRQ, 0);
	// nvic_enable_irq(NVIC_ADC_COMP_IRQ);
  // nvic_set_priority(NVIC_DMA1_CHANNEL1_IRQ, 0);
	// nvic_enable_irq(NVIC_DMA1_CHANNEL1_IRQ);
  nvic_set_priority(NVIC_DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_IRQ, 0);
	nvic_enable_irq(NVIC_DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_IRQ);
  nvic_set_priority(NVIC_DMA1_CHANNEL4_7_DMA2_CHANNEL3_5_IRQ, 0);
	nvic_enable_irq(NVIC_DMA1_CHANNEL4_7_DMA2_CHANNEL3_5_IRQ);

  nvic_set_priority(NVIC_EXTI4_15_IRQ, 64);
	nvic_enable_irq(NVIC_EXTI4_15_IRQ);

  nvic_set_priority(NVIC_TIM2_IRQ, 64);
	nvic_enable_irq(NVIC_TIM2_IRQ);

  nvic_set_priority(NVIC_TIM7_IRQ, 128);
	nvic_enable_irq(NVIC_TIM7_IRQ);

  nvic_set_priority(NVIC_TSC_IRQ, 128);
  nvic_enable_irq(NVIC_TSC_IRQ);

  nvic_set_priority(NVIC_USART1_IRQ, 64);
	nvic_enable_irq(NVIC_USART1_IRQ);
  nvic_set_priority(NVIC_USART2_IRQ, 64);
	nvic_enable_irq(NVIC_USART2_IRQ);
}


// Configure RTC and ensure it's running
// Status LED behavior:
//  Cyan: Normal startup
//  Red/Cyan blink: Oscillator was stopped, reinitialization occured (clock will need to be updated by user)
//  Red/Yellow blink: Oscillator did not start within the expected time (clock may not function)
//
void _rtcSetup()
{
  // these values are the power-on defaults for the prescaler.
  // we'll set them anyway to be sure they're there
	const uint32_t async = 127;
  uint32_t sync = 255;
  uint16_t timeout = 474;

  if (rcc_is_osc_ready(RCC_LSE) == false)
  {
    blinkStatusLed(Application::red, Application::cyan, 4, 100);

    pwr_disable_backup_domain_write_protect();

    // reset the RTC to ensure it's in a known state
    RCC_BDCR |= RCC_BDCR_BDRST;
    RCC_BDCR &= ~RCC_BDCR_BDRST;

    if (_externalRtcConnected == true)
    {
      rcc_osc_bypass_enable(RCC_LSE);
    }

    rcc_osc_on(RCC_LSE);
    rcc_wait_for_osc_ready(RCC_LSE);

    rcc_set_rtc_clock_source(RCC_LSE);

  	rcc_enable_rtc_clock();

  	rtc_unlock();

  	// enter init mode -- this lets us test that everything is working as expected
  	RTC_ISR |= RTC_ISR_INIT;
    while (((RTC_ISR & RTC_ISR_INITF) == 0) && (--timeout > 0));

    // do a blinky thing to indicate the problem if we had to wait too long :(
    if (timeout == 0)
    {
      // reset the RTC to undo anything that might've been done above
      RCC_BDCR |= RCC_BDCR_BDRST;
      RCC_BDCR &= ~RCC_BDCR_BDRST;

      blinkStatusLed(Application::red, Application::yellow, 20, 500);
    }

  	// set synch prescaler, using defaults for 1Hz out
  	rtc_set_prescaler(sync, async);

  	// exit init mode
  	RTC_ISR &= ~(RTC_ISR_INIT);

    rtc_lock();

  	// And wait for synchro...
  	rtc_wait_for_synchro();
    pwr_enable_backup_domain_write_protect();
  }
  else
  {
    setStatusLed(Application::cyan);
  }
}


// Deactivates all SPI peripheral CS/CE lines
//
void _spiReleaseAllCs()
{
  if (_invertCsForTemperatureSensor == true)
  {
    gpio_set(cNssPort, cNssRtcPin);
    gpio_clear(cNssPort, cNssTemperaturePin | cNssDisplayPin);
  }
  else
  {
    gpio_set(cNssPort, cNssRtcPin | cNssTemperaturePin);
    gpio_clear(cNssPort, cNssDisplayPin);
  }
}


// Toggle SPI MISO between display drivers and the other peripherals and
//  activate the appropriate CS/CE line
// ***** Make sure SPI is not busy/in use before switching things around! *****
void _spiSelectPeripheral(const SpiPeripheral peripheral)
{
  // We won't waste time switching things around if we don't need to
  if (_spiActivePeripheral != peripheral)
  {
    _spiActivePeripheral = peripheral;    // save for later!

    if (peripheral != SpiPeripheral::None)
    {
      // Reset SPI, SPI_CR1 register cleared, SPI is disabled
      spi_disable(SPI1);

      _spiReleaseAllCs();

      // Configure some SPI parameters common to all peripherals
      spi_fifo_reception_threshold_8bit(SPI1);
    	spi_set_data_size(SPI1, SPI_CR2_DS_8BIT);
      /*
       * Set NSS management to software.
       *
       * Note:
       * Setting nss high is very important, even if we are controlling the GPIO
       * ourselves this bit needs to be at least set to 1, otherwise the spi
       * peripheral will not send any data out.
       */
    	spi_enable_software_slave_management(SPI1);
    	spi_set_nss_high(SPI1);

      // Configure more settings for not-special peripherals
      if (peripheral != SpiPeripheral::HvDrivers)
      {
        // Everything else we use expects the MSb first
        spi_send_msb_first(SPI1);
        // ...and this phase
        spi_set_clock_phase_1(SPI1);
        // Configure display driver peripheral MISO pin as input
        gpio_mode_setup(cSpi1AltPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, cSpi1MisoDisplayPin);
        // Configure GPIOs: SCK=PA5, MISO=PA6 and MOSI=PA7
        gpio_mode_setup(cSpi1Port, GPIO_MODE_AF, GPIO_PUPD_NONE, cSpi1MisoPin);
      }
    }

    // Configure settings for special peripherals and activate CS/CE lines
    switch (peripheral)
    {
      case SpiPeripheral::HvDrivers:
        // Write the LSbs out first to the HV drivers
        spi_send_lsb_first(SPI1);
        // ...and the HV drivers prefer this
        spi_set_clock_phase_0(SPI1);
        // Configure other peripherals MISO pin as input
        gpio_mode_setup(cSpi1Port, GPIO_MODE_INPUT, GPIO_PUPD_NONE, cSpi1MisoPin);
        // MISO=PB4 from HV drivers
        gpio_mode_setup(cSpi1AltPort, GPIO_MODE_AF, GPIO_PUPD_NONE, cSpi1MisoDisplayPin);

        // gpio_clear(cNssPort, cNssDisplayPin);   // this shouldn't be necessary here
        break;

      case SpiPeripheral::Rtc:
        gpio_clear(cNssPort, cNssRtcPin);
        break;

      case SpiPeripheral::TempSensor:
        if (_invertCsForTemperatureSensor == true)
        {
          gpio_set(cNssPort, cNssTemperaturePin);
        }
        else
        {
          gpio_clear(cNssPort, cNssTemperaturePin);
        }
        break;

      default:
        _spiReleaseAllCs();
        break;
    }
     // Enable SPI1 peripheral
    spi_enable(SPI1);
  }
}


// Configure SPI
//
void _spiSetup()
{
  // Reset SPI, SPI_CR1 register cleared, SPI is disabled
  spi_reset(SPI1);

  /* Set up SPI in Master mode with:
   * Clock baud rate: 1/8 of peripheral clock frequency
   * Clock polarity: Idle High
   * Clock phase: Data valid on 1st clock pulse
   * Frame format: MSB First
   */
  spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_8, SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
                  SPI_CR1_CPHA_CLK_TRANSITION_2, SPI_CR1_MSBFIRST);

  // Setup SPI1 pins as alternate function zero
  gpio_set_af(cSpi1Port, GPIO_AF0, cSpi1SckPin | cSpi1MisoPin | cSpi1MosiPin);
  // MISO=PB4 from LED drivers
  gpio_set_af(cSpi1AltPort, GPIO_AF0, cSpi1MisoDisplayPin);
  // Configure GPIOs: SCK=PA5 and MOSI=PA7
  gpio_mode_setup(cSpi1Port, GPIO_MODE_AF, GPIO_PUPD_NONE, cSpi1SckPin | cSpi1MosiPin);
  // There is still more to do, but it will be taken care of by whatever calls
  //  _spiSelectPeripheral() later on
}


// Set up timer to fire every x milliseconds
// This is a unusual usage of systick, be very careful with the 24bit range
// of the systick counter!  You can range from 1 to 2796ms with this.
//
void _systickSetup(const uint16_t xms)
{
	// div8 per ST, stays compatible with M3/M4 parts, well done ST
	systick_set_clocksource(STK_CSR_CLKSOURCE_EXT);
	// clear counter so it starts right away
	STK_CVR = 0;

	systick_set_reload(rcc_ahb_frequency / 8 / 1000 * xms);
	systick_counter_enable();
	systick_interrupt_enable();
}


// Set up the timer for the beeper/buzzer
//
void _timerSetupBeeper()
{
  // Set the timer's global mode to:
  // - use no divider
  // - alignment edge
  // - count direction up
  timer_set_mode(cBuzzerTimer,  TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_set_prescaler(cBuzzerTimer, 8);
  timer_set_repetition_counter(cBuzzerTimer, 0);
  timer_continuous_mode(cBuzzerTimer);
  timer_set_period(cBuzzerTimer, 1);

  timer_disable_oc_output(cBuzzerTimer, cBuzzerTimerOC);
  timer_set_oc_mode(cBuzzerTimer, cBuzzerTimerOC, TIM_OCM_PWM1);
  timer_set_oc_value(cBuzzerTimer, cBuzzerTimerOC, 0);

  timer_enable_break_main_output(cBuzzerTimer);
  timer_enable_oc_output(cBuzzerTimer, cBuzzerTimerOC);

  timer_enable_preload(cBuzzerTimer);

  timer_enable_counter(cBuzzerTimer);

  // Configure our beeper GPIO pins
  gpio_mode_setup(cBuzzerPort, GPIO_MODE_AF, GPIO_PUPD_NONE, cBuzzerPin);
  gpio_set_af(cBuzzerPort, GPIO_AF2, cBuzzerPin);
  gpio_set_output_options(cBuzzerPort, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, cBuzzerPin);
}


// Set up the timer for the status LEDs
//
void _timerSetupStatusLED()
{
  // Set the timer's global mode to:
  // - use no divider
  // - alignment edge
  // - count direction up
  timer_set_mode(cLedPwmTimer,  TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_set_prescaler(cLedPwmTimer, 0);
  timer_set_repetition_counter(cLedPwmTimer, 0);
  timer_continuous_mode(cLedPwmTimer);
  timer_set_period(cLedPwmTimer, RgbLed::cLedMaxIntensity);

  timer_disable_oc_output(cLedPwmTimer, cLed0PwmOc);
  timer_disable_oc_output(cLedPwmTimer, cLed1PwmOc);
  timer_disable_oc_output(cLedPwmTimer, cLed2PwmOc);
  timer_set_oc_mode(cLedPwmTimer, cLed0PwmOc, TIM_OCM_PWM1);
  timer_set_oc_mode(cLedPwmTimer, cLed1PwmOc, TIM_OCM_PWM1);
  timer_set_oc_mode(cLedPwmTimer, cLed2PwmOc, TIM_OCM_PWM1);
  timer_set_oc_value(cLedPwmTimer, cLed0PwmOc, 0);
  timer_set_oc_value(cLedPwmTimer, cLed1PwmOc, 0);
  timer_set_oc_value(cLedPwmTimer, cLed2PwmOc, 0);
  timer_enable_oc_output(cLedPwmTimer, cLed0PwmOc);
  timer_enable_oc_output(cLedPwmTimer, cLed1PwmOc);
  timer_enable_oc_output(cLedPwmTimer, cLed2PwmOc);

  timer_enable_preload(cLedPwmTimer);

  timer_enable_counter(cLedPwmTimer);

  setRedLed(RgbLed::cLedMaxIntensity); // show signs of life...

  // Configure status LED GPIO pins
  gpio_mode_setup(cLedPort, GPIO_MODE_AF, GPIO_PUPD_NONE, cLed0Pin | cLed1Pin | cLed2Pin);
  gpio_set_af(cLedPort, cLedPortAF, cLed0Pin | cLed1Pin | cLed2Pin);
  gpio_set_output_options(cLedPort, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, cLed0Pin | cLed1Pin | cLed2Pin);
}


// Set up the timer for measuring pulses on the IR remote sensor input
//
void _timerSetupIR()
{
  // Set the timer's global mode to:
  // - use no divider
  // - alignment edge
  // - count direction up
  timer_set_mode(cIrTimer,  TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

  timer_set_prescaler(cIrTimer, InfraredRemote::cTimerPeriod);
  timer_set_repetition_counter(cIrTimer, 0);
  timer_continuous_mode(cIrTimer);
  timer_set_period(cIrTimer, 15000);

  timer_enable_preload(cIrTimer);

  timer_enable_irq(cIrTimer, TIM_DIER_UIE);

  timer_enable_counter(cIrTimer);
}


// Set up the timer for the display PWM generation
//
void _timerSetupPWM()
{
  // Set the timer's global mode to:
  // - use no divider
  // - alignment edge
  // - count direction up
  timer_set_mode(TIM2,  TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

  timer_set_prescaler(TIM2, 0);
  timer_set_repetition_counter(TIM2, 0);
  timer_continuous_mode(TIM2);
  timer_set_period(TIM2, 9600);

  timer_enable_preload(TIM2);

  timer_enable_irq(TIM2, TIM_DIER_UIE);

  timer_enable_counter(TIM2);
}


// Set up the Timers
//
void _timerSetup()
{
  _timerSetupStatusLED();
  _timerSetupBeeper();
  _timerSetupIR();
  _timerSetupPWM();
}


// Set up the Touch Sensing Controller
//
void _tscSetup()
{
  gpio_mode_setup(cTscPort, GPIO_MODE_AF, GPIO_PUPD_NONE, cTscTouchKeyPins | cTscSamplingCapPins);
  gpio_set_output_options(cTscPort, GPIO_OTYPE_OD, GPIO_OSPEED_MED, cTscSamplingCapPins);
  gpio_set_output_options(cTscPort, GPIO_OTYPE_PP, GPIO_OSPEED_MED, cTscTouchKeyPins);
	gpio_set_af(cTscPort, GPIO_AF3, cTscTouchKeyPins | cTscSamplingCapPins);

  gpio_mode_setup(cTscPortC, GPIO_MODE_AF, GPIO_PUPD_NONE, cTscTouchKeyPinsC);
  gpio_set_output_options(cTscPortC, GPIO_OTYPE_PP, GPIO_OSPEED_MED, cTscTouchKeyPinsC);
  gpio_set_af(cTscPortC, GPIO_AF0, cTscTouchKeyPinsC);

  /* To allow the control of the sampling capacitor I/O by the TSC peripheral,
   the corresponding GPIO must be first set to alternate output open drain
   mode and then the corresponding Gx_IOy bit in the TSC_IOSCR register must
   be set. */
	TSC_IOSCR = cTscSamplingControlBits;

  _tscChannelCounter = 0;

  /* To allow the control of the channel I/O by the TSC peripheral, the
  corresponding GPIO must be first set to alternate output push-pull mode and
  the corresponding Gx_IOy bit in the TSC_IOCCR register must be set. */
  TSC_IOCCR = cTscChannelControlBits[_tscChannelCounter];

  // The GxE bits in the TSC_IOGCSR registers specify which analog I/O groups are enabled
  TSC_IOGCSR = cTscGroupControlBits;

  // schmitt trigger hysteresis disabled for all groups
  TSC_IOHCR = 0x00;
  // TSC_IOHCR = (TSC_IOHCR_G3(2) | TSC_IOHCR_G3(3) |
  //              TSC_IOHCR_G5(3) | TSC_IOHCR_G5(4) |
  //              TSC_IOHCR_G6(2) | TSC_IOHCR_G6(3));

  // enable the end of acquisition and max count error interrupts
  TSC_ICR = TSC_ICR_EOAIC | TSC_ICR_MCEIC;
  TSC_IER = TSC_IER_EOAIE | TSC_IER_MCEIE;

  // define the pulse generator prescaler value
  TSC_CR |= (0x01 << TSC_CR_PGPSC_SHIFT);
  // define the duration of the high state of the charge transfer pulse (charge of CX)
  TSC_CR |= (0x01 << TSC_CR_CTPH_SHIFT);
  // define the duration of the low state of the charge transfer pulse (transfer of charge from CX to CS)
  TSC_CR |= (0x01 << TSC_CR_CTPL_SHIFT);
  // define the maximum number of charge transfer pulses that can be generated before a max count error is generated
  TSC_CR |= (0x06 << TSC_CR_MCV_SHIFT);
  // configure and enable spread spectrum
  TSC_CR |= (0x3f << TSC_CR_SSD_SHIFT) | TSC_CR_SSE;

  // Enable the TSC
  TSC_CR |= TSC_CR_TSCE;
  // Normal acquisition mode:
  //  acquisition starts when the START bit in the TSC_CR register is set
  TSC_CR |= TSC_CR_START;
}


void _usart1Setup()
{
  // Setup USART1 TX & RX pins as alternate function
  if (cUsart1TxPin == GPIO9)
  {
    gpio_set_af(cUsart1TxPort, GPIO_AF1, cUsart1TxPin);
  }
  else if (cUsart1TxPin == GPIO6)
  {
    gpio_set_af(cUsart1TxPort, GPIO_AF0, cUsart1TxPin);
  }

  if (cUsart1RxPin == GPIO10)
  {
    gpio_set_af(cUsart1RxPort, GPIO_AF1, cUsart1RxPin);
  }
  else if (cUsart1RxPin == GPIO7)
  {
    gpio_set_af(cUsart1RxPort, GPIO_AF0, cUsart1RxPin);
  }
  // Set up USART1 parameters
  usart_set_databits(USART1, 8);
	usart_set_baudrate(USART1, cUsart1BaudRate);
  usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_mode(USART1, USART_MODE_TX_RX);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

  USART1_CR2 |= (USART_CR2_ABRMOD_FALL_EDGE | USART_CR2_ABREN);    // enable auto-baud rate detection

  usart_enable_rx_interrupt(USART1);
  // usart_enable_tx_interrupt(USART1);
  usart_enable_error_interrupt(USART1);
  // Enable the USART
  usart_enable(USART1);

  // Setup GPIO pins for USART1 transmit and recieve
  gpio_mode_setup(cUsart1RxPort, GPIO_MODE_AF, GPIO_PUPD_NONE, cUsart1RxPin);
  gpio_mode_setup(cUsart1TxPort, GPIO_MODE_AF, GPIO_PUPD_NONE, cUsart1TxPin);
}


void _usart2Setup()
{
  // Setup USART2 TX & RX pins as alternate function
  gpio_set_af(cUsart2Port, GPIO_AF1, cUsart2RxPin | cUsart2TxPin | cUsart2DePin);
  // Set up USART2 parameters
  usart_set_databits(USART2, 8);
	usart_set_baudrate(USART2, cUsart2BaudRate);
  usart_set_stopbits(USART2, USART_STOPBITS_2);
	usart_set_parity(USART2, USART_PARITY_NONE);
	usart_set_mode(USART2, USART_MODE_RX);
	usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
  // Enable the USART's RS-485 driver mode output
  USART2_CR3 |= USART_CR3_DEM;

  // usart_enable_rx_interrupt(USART2);
  // usart_enable_tx_interrupt(USART2);
  usart_enable_error_interrupt(USART2);
  // Enable the USART
  usart_enable(USART2);

  // Setup GPIO pins for USART2 transmit, recieve, and direction
  gpio_mode_setup(cUsart2Port, GPIO_MODE_AF, GPIO_PUPD_NONE, cUsart2RxPin | cUsart2TxPin | cUsart2DePin);
}


// Set up USARTs
//
void _usartSetup()
{
  _usart1Setup();
  _usart2Setup();
}


void _incrementOnTimeSecondsCounter()
{
  if ((_onTimeLastSecond != _currentDateTime.second()) && (_hvState == true))
  {
    _onTimeLastSecond = _currentDateTime.second();

    pwr_disable_backup_domain_write_protect();

    RTC_BKPXR(0) = ++_onTimeSecondsCounter;

    pwr_enable_backup_domain_write_protect();

    // if connected, save to DS3234 RAM, also
    // if (_externalRtcConnected == true)
    // {
    //   DS3234::writeSram(0x00, (uint8_t*)&_onTimeSecondsCounter, 4, false);
    // }
  }
}


HwReqAck _refreshTemp()
{
  HwReqAck status = HwReqAck::HwReqAckOk;

  // Determine where to get the temperature from and get it
  switch (_externalTemperatureSensor)
  {
    case TempSensorType::DS3234:
      // Registers will have been refreshed above
      _temperatureXcBaseMultiplier =  DS3234::getTemperatureWholePart() * cBaseMultiplier;
      _temperatureXcBaseMultiplier += ((DS3234::getTemperatureFractionalPart() >> 1) * cTempFracMultiplier);
      break;

    case TempSensorType::LM74:
      status = LM74::refresh();
      _temperatureXcBaseMultiplier =  LM74::getTemperatureWholePart() * cBaseMultiplier;
      _temperatureXcBaseMultiplier += ((LM74::getTemperatureFractionalPart() >> 1) * cTempFracMultiplier);
      break;

    case TempSensorType::DS1722:
      status = DS1722::refresh();
      _temperatureXcBaseMultiplier =  DS1722::getTemperatureWholePart() * cBaseMultiplier;
      _temperatureXcBaseMultiplier += ((DS1722::getTemperatureFractionalPart() >> 1) * cTempFracMultiplier);
      break;

    default:
      uint32_t totalTemp = 0;

      // Add up some values so we can compute some averages...
      for (uint8_t i = 0; i < cAdcSamplesToAverage; i++)
      {
        totalTemp += _adcSampleSetTemp[i];
      }
      // Get the averages of the last several readings
      int32_t averageTemp = totalTemp / cAdcSamplesToAverage;
      // Now we can compute the temperature based on RM's formula, * cBaseMultiplier
      _temperatureXcBaseMultiplier = (averageTemp * voltageVddA() / cVddCalibrationVoltage) - ST_TSENSE_CAL1_30C;
      _temperatureXcBaseMultiplier = _temperatureXcBaseMultiplier * (110 - 30) * cBaseMultiplier;
      _temperatureXcBaseMultiplier = _temperatureXcBaseMultiplier / (ST_TSENSE_CAL2_110C - ST_TSENSE_CAL1_30C);
      _temperatureXcBaseMultiplier = _temperatureXcBaseMultiplier + 30000 + (_temperatureAdjustment * cBaseMultiplier);
  }

  return status;
}


HwReqAck _refreshRTC()
{
  HwReqAck status = HwReqAck::HwReqAckOk;
  // Update time/date if it's the DS323x's turn
  if (_externalRtcConnected == true)
  {
    status = DS3234::refresh();

    if (status == HwReqAck::HwReqAckOk)
    {
      _currentDateTime = DS3234::getDateTime();
      // _rtcIsSet = DS3234::isValid();
    }
  }
  else
  {
    uint32_t dr = RTC_DR, tr = RTC_TR;
    uint16_t year = 2000;
    uint8_t month, day, hour, minute, second;

    year += (10 * ((dr >> RTC_DR_YT_SHIFT) & RTC_DR_YT_MASK));
    year += ((dr >> RTC_DR_YU_SHIFT) & RTC_DR_YU_MASK);

    month = 10 * ((dr >> RTC_DR_MT_SHIFT) & RTC_DR_MT_MASK);
    month += ((dr >> RTC_DR_MU_SHIFT) & RTC_DR_MU_MASK);

    day = 10 * ((dr >> RTC_DR_DT_SHIFT) & RTC_DR_DT_MASK);
    day += ((dr >> RTC_DR_DU_SHIFT) & RTC_DR_DU_MASK);

    hour = 10 * ((tr >> RTC_TR_HT_SHIFT) & RTC_TR_HT_MASK);
    hour += ((tr >> RTC_TR_HU_SHIFT) & RTC_TR_HU_MASK);

    minute = 10 * ((tr >> RTC_TR_MNT_SHIFT) & RTC_TR_MNT_MASK);
    minute += ((tr >> RTC_TR_MNU_SHIFT) & RTC_TR_MNU_MASK);

    second = 10 * ((tr >> RTC_TR_ST_SHIFT) & RTC_TR_ST_MASK);
    second += ((tr >> RTC_TR_SU_SHIFT) & RTC_TR_SU_MASK);

    _currentDateTime.setDate(year, month, day);
    _currentDateTime.setTime(hour, minute, second);
    // _rtcIsSet = RTC_ISR & RTC_ISR_INITS;
  }

  return status;
}


void _syncRtcWithGps()
{
  DateTime gpsTime = GpsReceiver::getLocalDateTime();

  if ((GpsReceiver::isConnected() == true)
      && (GpsReceiver::isValid() == true)
      && (gpsTime.hour() != _lastRtcGpsSyncHour)
      && (gpsTime != _currentDateTime))
  {
    setDateTime(gpsTime);
    _lastRtcGpsSyncHour = gpsTime.hour();
  }
}


// Starts it all up
//
void initialize()
{
  Display startupDisplay(cTargetHardwareVersion);

  // First, set up all the hardware things
  _clockSetup();
  _iwdgSetup();
  _gpioSetup();
  _timerSetup();
  _dmaSetup();
  _adcSetup();
  _tscSetup();

  _systickSetup(1);   // tick every 1 mS
  _nvicSetup();

  _spiSetup();
  _usartSetup();
  // _i2cSetup();

  Dmx512Rx::initialize();

  // Let's initialize some memory/data structures
  for (_adcSampleCounter = 0; _adcSampleCounter < cAdcSamplesToAverage; _adcSampleCounter++)
  {
    _adcSampleSetLight[_adcSampleCounter] = _bufferADC[cAdcPhototransistor];
    _adcSampleSetTemp[_adcSampleCounter] = _bufferADC[cAdcTemperature];
    _adcSampleSetVoltageVddA[_adcSampleCounter] = _bufferADC[cAdcVref];
    _adcSampleSetVoltageBatt[_adcSampleCounter] = _bufferADC[cAdcVbat];
  }

  for (_tscChannelCounter = 0; _tscChannelCounter < cTscChannelCount; _tscChannelCounter++)
  {
    _tscAcquisitionValues[_tscChannelCounter] = 0;
    _tscMinimums[_tscChannelCounter] = 0xffff;
    _tscMaximums[_tscChannelCounter] = 0;
  }

  // Here we check for other temperature sensors
  if (LM74::isConnected() == true)
  {
    _externalTemperatureSensor = TempSensorType::LM74;
  }
  else
  {
    _invertCsForTemperatureSensor = true;

    if (DS1722::isConnected() == true)
    {
      _externalTemperatureSensor = TempSensorType::DS1722;
    }
    else
    {
      _invertCsForTemperatureSensor = false;
    }
  }

  _externalRtcConnected = DS3234::isConnected();

  if (_externalRtcConnected == true)
  {
    uint8_t buffer[] = { 0, 0x08 };

    while (_spiActivePeripheral != SpiPeripheral::None);
    // isConnected() refreshes the status register so this will work without a refresh()
    _rtcIsSet = DS3234::isValid();
    // enable PPS and 32 kHz outputs
    while (DS3234::setRegister(0x0e, buffer, 2, true) != HwReqAck::HwReqAckOk);
    if (_externalTemperatureSensor == TempSensorType::NoTempSensor)
    {
      _externalTemperatureSensor = TempSensorType::DS3234;
    }
  }
  else
  {
    _rtcIsSet = RTC_ISR & RTC_ISR_INITS;
  }

  _rtcSetup();

  _onTimeSecondsCounter = RTC_BKPXR(0);

  Hardware::setHvState(true);

  DisplayManager::initialize();
  DisplayManager::setStatusLedAutoRefreshing(true);
  DisplayManager::writeDisplay(startupDisplay, Application::darkGray);
  DisplayManager::setDisplayBlanking(false);

  delay(500);

  startupDisplay.setTubesOff(0b111111);
  DisplayManager::writeDisplay(startupDisplay, RgbLed());
  delay(250);

  _refreshRTC();
  _refreshTemp();
}


// Should be called frequently from the main loop
//
void refresh()
{
  iwdg_reset();

  // _refreshPeripherals();
  _syncRtcWithGps();

  DisplayManager::refresh();
}


HwReqAck tick()
{
  return tone(cToneFrequencyMinimum * 8, 2);
}


HwReqAck tone(const uint16_t frequency, const uint16_t duration)
{
  // period = (48000000 / 8 (prescaler)) / frequency
  // oc_value = period / 2
  if (_toneTimer == 0)
  {
    if ((frequency >= cToneFrequencyMinimum) && (_toneVolume < cToneVolumeMaximum))
    {
      // processor speed divided by prescaler...
      uint32_t period = (48000000 / 8) / frequency,
               ocValue = period / (2 << _toneVolume);
      timer_set_period(cBuzzerTimer, period);
      timer_set_oc_value(cBuzzerTimer, cBuzzerTimerOC, ocValue);
    }
    else
    {
      // timer_set_period(cBuzzerTimer, 1);
      timer_set_oc_value(cBuzzerTimer, cBuzzerTimerOC, 0);
    }

    _toneTimer = duration;

    return HwReqAck::HwReqAckOk;
  }
  else if (_toneTimerNext == 0)
  {
    _toneTimerNext = duration;
    _toneFrequencyNext = frequency;

    return HwReqAck::HwReqAckBusy;
  }
  else
  {
    return HwReqAck::HwReqAckError;
  }
}


bool alarmInput(const uint8_t alarmInputNumber)
{
  if (alarmInputNumber < cAlarmInputPinCount)
  {
    // if the pin is low, return true (alarm is active if pin is pulled down)
    return (gpio_get(cAlarmInputPort, cAlarmInputPins[alarmInputNumber]) == 0);
  }

  return false;
}


bool button(const uint8_t button)
{
  if (_buttonStates & (1 << button))
  {
    return true;
  }

  return false;
}


uint8_t buttons()
{
  return _buttonStates;
}


uint8_t buttonsPressed()
{
  return _buttonStates & _buttonsChanged;
}


uint8_t buttonsReleased()
{
  return ~_buttonStates & _buttonsChanged;
}


void buttonsRefresh()
{
  uint32_t avg = 0;
  uint16_t mid = 0;
  uint8_t  button = 0, sample = 0;

  for (button = 0; button < cTscChannelCount; button++)
  {
    avg = 0;
    mid = (_tscMaximums[button] - _tscMinimums[button]) / 3;

    for (sample = 0; sample < cTscSamplesToAverage; sample++)
    {
      avg += _tscSampleSets[button][sample];
    }
    avg = avg / cTscSamplesToAverage;

    if ((avg < _tscMinimums[button] + mid) && (_tscMaximums[button] - _tscMinimums[button] > cTscMinimumDelta) && _tscMinimums[button] < cTscMaximumMinimum)
    {
      _buttonStates |= (1 << button);
    }
    else
    {
      _buttonStates &= ~(1 << button);
    }
  }

  _buttonsChanged = _buttonStates ^ _buttonStatesPrevious;
  _buttonStatesPrevious = _buttonStates;
}


void setHvState(const bool hvEnabled)
{
  _hvState = hvEnabled;

  if (hvEnabled == false)
  {
    gpio_set(cHvShutdownPort, cHvShutdownPin);
  }
  else
  {
    gpio_clear(cHvShutdownPort, cHvShutdownPin);
  }
}


void setDisplayHardwareBlanking(const bool blankingState)
{
  _displayBlankingState = blankingState;

  if (blankingState == false)
  {
    // blank the display
    gpio_set(cBlankDisplayPort, cBlankDisplayPin);
  }
  else
  {
    gpio_clear(cBlankDisplayPort, cBlankDisplayPin);
  }
}


bool dstState()
{
  return RTC_CR & RTC_CR_BKP;
}


DateTime getDateTime()
{
  // if ((GpsReceiver::isConnected() == true) && (GpsReceiver::isValid() == true))
  // {
  //   return GpsReceiver::getLocalDateTime();
  // }
  return _currentDateTime;
}


void setDateTime(const DateTime &dateTime)
{
  _currentDateTime = dateTime;
  _rtcIsSet = true;

  if (_externalRtcConnected == true)
  {
    DS3234::setDateTime(_currentDateTime);
  }
  // set the internal RTC regardless
  uint8_t dayOfWeek = _currentDateTime.dayOfWeek();

  // STM32 RTC does not accept 0; it uses 7 for Sunday, instead
  if (dayOfWeek == 0)
  {
    dayOfWeek = 7;
  }

  uint32_t dr = ((_currentDateTime.yearShort(true) & 0xff) << RTC_DR_YU_SHIFT) |
                ((_currentDateTime.month(true) & 0x1f) << RTC_DR_MU_SHIFT) |
                ((_currentDateTime.day(true) & 0x3f) << RTC_DR_DU_SHIFT) |
                ((dayOfWeek) << RTC_DR_WDU_SHIFT);

  uint32_t tr = ((_currentDateTime.hour(true, false) & 0x3f) << RTC_TR_HU_SHIFT) |
                ((_currentDateTime.minute(true) & 0x7f) << RTC_TR_MNU_SHIFT) |
                ((_currentDateTime.second(true) & 0x7f) << RTC_TR_SU_SHIFT);

  pwr_disable_backup_domain_write_protect();
  rtc_unlock();

  // enter init mode
  RTC_ISR |= RTC_ISR_INIT;
  while ((RTC_ISR & RTC_ISR_INITF) == 0);

  RTC_DR = dr;
  RTC_TR = tr;

  // exit init mode
  RTC_ISR &= ~(RTC_ISR_INIT);

  rtc_lock();
  rtc_wait_for_synchro();
  pwr_enable_backup_domain_write_protect();

  return;
}


bool rtcIsSet()
{
  return _rtcIsSet;
}


int32_t temperature(const bool fahrenheit, const bool bcd)
{
  if (fahrenheit == true)
  {
    if (bcd == true)
    {
      return uint32ToBcd((uint16_t)((_temperatureXcBaseMultiplier * 18) / 10000) + 32);
    }
    return ((_temperatureXcBaseMultiplier * 18) / 10000) + 32;
  }
  else
  {
    if (bcd == true)
    {
      return uint32ToBcd((uint16_t)_temperatureXcBaseMultiplier / cBaseMultiplier);
    }
    return _temperatureXcBaseMultiplier / cBaseMultiplier;
  }
}


uint16_t lightLevel()
{
  uint32_t totalLight = 0;

  // Add up some values so we can compute some averages...
  for (uint8_t i = 0; i < cAdcSamplesToAverage; i++)
  {
    totalLight += _adcSampleSetLight[i];
  }
  // Compute average light level and save it for access later
  return totalLight / cAdcSamplesToAverage;
}


uint32_t onTimeSeconds()
{
  return _onTimeSecondsCounter;
}


void onTimeSecondsReset()
{
  _onTimeSecondsCounter = 0;
}


uint16_t voltageBatt()
{
  uint32_t adcSampleSum = 0;

  _batteryMeasuringCounter = 0;
  adc_enable_vbat_sensor();

  // Add up some values so we can compute some averages...
  for (uint8_t i = 0; i < cAdcSamplesToAverage; i++)
  {
    adcSampleSum += _adcSampleSetVoltageBatt[i];
  }
  // Get the averages of the last several readings
  int32_t adcSampleAverage = adcSampleSum / cAdcSamplesToAverage;
  // Determine the voltage as it affects the temperature calculation
  return cVddCalibrationVoltage * (int32_t)ST_VREFINT_CAL / adcSampleAverage;
}


uint16_t voltageVddA()
{
  uint32_t adcSampleSum = 0;

  // Add up some values so we can compute some averages...
  for (uint8_t i = 0; i < cAdcSamplesToAverage; i++)
  {
    adcSampleSum += _adcSampleSetVoltageVddA[i];
  }
  // Get the averages of the last several readings
  int32_t adcSampleAverage = adcSampleSum / cAdcSamplesToAverage;
  // Determine the voltage as it affects the temperature calculation
  return cVddCalibrationVoltage * (int32_t)ST_VREFINT_CAL / adcSampleAverage;
}


void setDstState(const bool enableDst, const bool adjustRtcHardware)
{
  // number of seconds in an hour
  int16_t rtcAdjustment = 3600;
  // bit to set in RTC_CR to adjust the STM32's internal RTC
  uint32_t rtcDstAdjustBit = 0;
  // get the current state of the hardware into dstHwState
  bool dstHwState = RTC_CR & RTC_CR_BKP;
  // if the new state and the hardware state are not the same...
  if (enableDst != dstHwState)
  {
    // get ready to toggle the BKP bit...
    pwr_disable_backup_domain_write_protect();
    rtc_unlock();

    if (enableDst == true)
    {
      RTC_CR |= RTC_CR_BKP;

      rtcDstAdjustBit = RTC_CR_ADD1H;
    }
    else
    {
      RTC_CR &= ~RTC_CR_BKP;

      rtcDstAdjustBit = RTC_CR_SUB1H;

      rtcAdjustment *= -1;
    }

    if (adjustRtcHardware == true)
    {
      RTC_CR |= rtcDstAdjustBit;

      if (_externalRtcConnected == true)
      {
        DS3234::setDateTime(_currentDateTime.addSeconds(rtcAdjustment));
      }

      DisplayManager::doubleBlink();
    }

    rtc_lock();
    rtc_wait_for_synchro();
    pwr_enable_backup_domain_write_protect();
  }
}


void setFlickerReduction(const uint16_t value)
{
  _blankingThreshold = value;
}


void setTemperatureCalibration(const int8_t value)
{
  _temperatureAdjustment = value;
}


void setVolume(const uint8_t volumeLevel)
{
  if (volumeLevel > cToneVolumeMaximum)
  {
    _toneVolume = 0;
  }
  else
  {
    _toneVolume = cToneVolumeMaximum - volumeLevel;
  }
}


uint32_t getCRC(uint32_t *inputData, uint32_t numElements)
{
  crc_reset();

  return crc_calculate_block(inputData, numElements);
}


PeripheralRefreshTrigger getPeripheralRefreshTrigger()
{
  return _peripheralRefreshTrigger;
}


bool getPpsInputState()
{
  if (gpio_get(Hardware::cPpsPort, Hardware::cPpsPin) != 0)
  {
    return true;
  }

  return false;
}


RtcType getRTCType()
{
  if (_externalRtcConnected == true)
  {
    return RtcType::DS323x;
  }
  return RtcType::Stm32;
}


TempSensorType getTempSensorType()
{
  return _externalTemperatureSensor;
}


uint32_t eraseFlash(uint32_t startAddress)
{
	uint32_t pageAddress = startAddress,
	         flashStatus = 0;

	// check if startAddress is in proper range
	if ((startAddress - FLASH_BASE) >= (cFlashPageSize * (cFlashPageNumberMaximum + 1)))
  {
    return 0xff;
  }

	// calculate current page address
  if ((startAddress % cFlashPageSize) != 0)
  {
    pageAddress -= (startAddress % cFlashPageSize);
  }

	flash_unlock();

	// Erase page(s)
	flash_erase_page(pageAddress);
	flashStatus = flash_get_status_flags();
	if (flashStatus == FLASH_SR_EOP)
  {
    flashStatus = 0;
  }

  flash_lock();

	return flashStatus;
}


void readFlash(uint32_t startAddress, uint16_t numElements, uint8_t *outputData)
{
	uint16_t i;
	uint32_t *memoryPtr = (uint32_t*)startAddress;

	for (i = 0; i < numElements / 4; i++)
	{
		*(uint32_t*)outputData = *(memoryPtr + i);
		outputData += 4;
	}
}


uint32_t writeFlash(uint32_t startAddress, uint8_t *inputData, uint16_t numElements)
{
	uint16_t i;
  const uint32_t erasedValue = 0xffffffff;
	uint32_t currentAddress = startAddress,
	         pageAddress = startAddress,
	         flashStatus = 0,
           returnStatus = 0;

	// check if startAddress is in proper range
	if ((startAddress - FLASH_BASE) >= (cFlashPageSize * (cFlashPageNumberMaximum + 1)))
  {
    return 0xff;
  }

	// calculate current page address
	if ((startAddress % cFlashPageSize) != 0)
  {
    pageAddress -= (startAddress % cFlashPageSize);
  }

	flash_unlock();

	// Erasing page
	flash_erase_page(pageAddress);
	flashStatus = flash_get_status_flags();
	if (flashStatus != FLASH_SR_EOP)
  {
    returnStatus = flashStatus;
  }
  else
  {
    // verify flash memory was completely erased
  	for (i = 0; i < numElements; i += 4)
  	{
      // verify if address was erased properly
  		if (*((uint32_t*)(currentAddress + i)) != erasedValue)
      {
        returnStatus = 0x40;
        break;
      }
    }

    if (returnStatus == 0)
    {
      // programming flash memory
    	for (i = 0; i < numElements; i += 4)
    	{
    		// programming word data
    		flash_program_word(currentAddress + i, *((uint32_t*)(inputData + i)));
    		flashStatus = flash_get_status_flags();
    		if (flashStatus != FLASH_SR_EOP)
        {
          returnStatus = 0x80 | flashStatus;
          break;
        }

    		// verify if correct data is programmed
    		if (*((uint32_t*)(currentAddress + i)) != *((uint32_t*)(inputData + i)))
        {
          returnStatus = 0x80 | FLASH_SR_PGERR;
          break;
        }
    	}
    }
  }

  flash_lock();

	return returnStatus;
}


HwReqAck i2cTransfer(const uint8_t addr, const uint8_t *bufferTx, size_t numberTx, uint8_t *bufferRx, size_t numberRx)
{
  // this could be arranged a little better but this way prevents the DMA
  //  complete interrupt from being called before the receive data is populated
  if (_i2cState != I2cState::I2cIdle)
  {
    if (_i2cBusyFailCount++ > cI2cMaxFailBusyCount)
    {
      _i2cRecover();
    }
    // Let the caller know it was busy if so
    return HwReqAck::HwReqAckBusy;
  }
  else
  {
    if (numberTx > 0)
    {
      _i2cAddr = addr;
      _i2cBufferRx = bufferRx;
      _i2cNumberRx = numberRx;

      return i2cTransmit(addr, bufferTx, numberTx, (numberRx == 0));
    }
    else if (numberRx > 0)
    {
      return i2cReceive(addr, bufferRx, numberRx, true);
    }
  }
  return HwReqAck::HwReqAckError;   // catchall
}


// Transfers the given buffers to/from the given peripheral through the SPI via DMA
//
HwReqAck i2cReceive(const uint8_t addr, uint8_t *bufferRx, const size_t numberRx, const bool autoEndXfer)
{
  if (_i2cState != I2cState::I2cIdle)
  {
    if (_i2cBusyFailCount++ > cI2cMaxFailBusyCount)
    {
      _i2cRecover();
    }
    // Let the caller know it was busy if so
    return HwReqAck::HwReqAckBusy;
  }

  _i2cBusyFailCount = 0;

  if (numberRx > 0)
  {
    _i2cState = I2cState::I2cBusy;
    i2c_set_7bit_address(I2C1, addr);
    i2c_set_read_transfer_dir(I2C1);
    i2c_set_bytes_to_transfer(I2C1, numberRx);

    // Reset DMA channel
    dma_channel_reset(DMA1, cI2c1RxDmaChannel);

    // Set up rx dma, note it has higher priority to avoid overrun
    dma_set_peripheral_address(DMA1, cI2c1RxDmaChannel, (uint32_t)&I2C1_RXDR);
    dma_set_memory_address(DMA1, cI2c1RxDmaChannel, (uint32_t)bufferRx);
    dma_set_number_of_data(DMA1, cI2c1RxDmaChannel, numberRx);
    dma_set_read_from_peripheral(DMA1, cI2c1RxDmaChannel);
    dma_enable_memory_increment_mode(DMA1, cI2c1RxDmaChannel);
    dma_set_peripheral_size(DMA1, cI2c1RxDmaChannel, DMA_CCR_PSIZE_8BIT);
    dma_set_memory_size(DMA1, cI2c1RxDmaChannel, DMA_CCR_MSIZE_8BIT);
    dma_set_priority(DMA1, cI2c1RxDmaChannel, DMA_CCR_PL_VERY_HIGH);

    // Enable dma transfer complete interrupt
  	dma_enable_transfer_complete_interrupt(DMA1, cI2c1RxDmaChannel);

  	// Activate dma channel
  	dma_enable_channel(DMA1, cI2c1RxDmaChannel);

    i2c_send_start(I2C1);

    if (autoEndXfer == true)
    {
      /* important to do it afterwards to do a proper repeated start! */
      i2c_enable_autoend(I2C1);
    }
    else
    {
      i2c_disable_autoend(I2C1);
    }

  	/* Enable the I2C transfer via DMA
  	 * This will immediately start the transmission, after which when the receive
     * is complete, the receive DMA will activate
  	 */
  	i2c_enable_rxdma(I2C1);
  }

  return HwReqAck::HwReqAckOk;
}


// Transfers the given buffers to/from the given peripheral through the SPI via DMA
//
HwReqAck i2cTransmit(const uint8_t addr, const uint8_t *bufferTx, const size_t numberTx, const bool autoEndXfer)
{
  if (_i2cState != I2cState::I2cIdle)
  {
    if (_i2cBusyFailCount++ > cI2cMaxFailBusyCount)
    {
      _i2cRecover();
    }
    // Let the caller know it was busy if so
    return HwReqAck::HwReqAckBusy;
  }

  _i2cBusyFailCount = 0;

  if (numberTx > 0)
  {
    _i2cState = I2cState::I2cBusy;
    /* Setting transfer properties */
    i2c_set_7bit_address(I2C1, addr);
    i2c_set_write_transfer_dir(I2C1);
    i2c_set_bytes_to_transfer(I2C1, numberTx);

    // Reset DMA channel
    dma_channel_reset(DMA1, cI2c1TxDmaChannel);

    // Set up tx dma
    dma_set_peripheral_address(DMA1, cI2c1TxDmaChannel, (uint32_t)&I2C1_TXDR);
    dma_set_memory_address(DMA1, cI2c1TxDmaChannel, (uint32_t)bufferTx);
    dma_set_number_of_data(DMA1, cI2c1TxDmaChannel, numberTx);
    dma_set_read_from_memory(DMA1, cI2c1TxDmaChannel);
    dma_enable_memory_increment_mode(DMA1, cI2c1TxDmaChannel);
    dma_set_peripheral_size(DMA1, cI2c1TxDmaChannel, DMA_CCR_PSIZE_8BIT);
    dma_set_memory_size(DMA1, cI2c1TxDmaChannel, DMA_CCR_MSIZE_8BIT);
    dma_set_priority(DMA1, cI2c1TxDmaChannel, DMA_CCR_PL_HIGH);

    // Enable dma transfer complete interrupt
  	dma_enable_transfer_complete_interrupt(DMA1, cI2c1TxDmaChannel);

  	// Activate dma channel
  	dma_enable_channel(DMA1, cI2c1TxDmaChannel);

    if (autoEndXfer == true)
    {
      i2c_enable_autoend(I2C1);
    }
    else
    {
      i2c_disable_autoend(I2C1);
    }

    /* start transfer */
    i2c_send_start(I2C1);

  	/* Enable the I2C transfer via DMA
  	 * This will immediately start the transmission, after which when the receive
     * is complete, the receive DMA will activate
  	 */
  	i2c_enable_txdma(I2C1);
  }

  return HwReqAck::HwReqAckOk;
}


void i2cAbort()
{
  _i2cRecover();
}


bool i2cIsBusy()
{
  return (_i2cState != I2cState::I2cIdle);
}


HwReqAck readSerial(const uint32_t usart, const uint32_t length, char* data)
{
  switch (usart)
  {
    case USART2:
    if ((DMA1_ISR & DMA_ISR_TCIF6) || !(USART2_CR3 & USART_CR3_DMAR))
    {
      dma_channel_reset(DMA1, cUsart2RxDmaChannel);
      // Set up rx dma, note it has higher priority to avoid overrun
      dma_set_peripheral_address(DMA1, cUsart2RxDmaChannel, (uint32_t)&USART2_RDR);
      dma_set_memory_address(DMA1, cUsart2RxDmaChannel, (uint32_t)data);
      dma_set_number_of_data(DMA1, cUsart2RxDmaChannel, length);
      dma_set_read_from_peripheral(DMA1, cUsart2RxDmaChannel);
      dma_enable_memory_increment_mode(DMA1, cUsart2RxDmaChannel);
      dma_set_peripheral_size(DMA1, cUsart2RxDmaChannel, DMA_CCR_PSIZE_8BIT);
      dma_set_memory_size(DMA1, cUsart2RxDmaChannel, DMA_CCR_MSIZE_8BIT);
      dma_set_priority(DMA1, cUsart2RxDmaChannel, DMA_CCR_PL_VERY_HIGH);

      // Enable dma transfer complete interrupt
    	dma_enable_transfer_complete_interrupt(DMA1, cUsart2RxDmaChannel);

    	// Activate dma channel
    	dma_enable_channel(DMA1, cUsart2RxDmaChannel);

      usart_enable_rx_dma(USART2);
    }
    else
    {
      return HwReqAck::HwReqAckBusy;
    }
    break;

    default:
    if ((DMA1_ISR & DMA_ISR_TCIF5) || !(USART1_CR3 & USART_CR3_DMAR))
    {
      dma_channel_reset(DMA1, cUsart1RxDmaChannel);
      // Set up rx dma, note it has higher priority to avoid overrun
      dma_set_peripheral_address(DMA1, cUsart1RxDmaChannel, (uint32_t)&USART1_RDR);
      dma_set_memory_address(DMA1, cUsart1RxDmaChannel, (uint32_t)data);
      dma_set_number_of_data(DMA1, cUsart1RxDmaChannel, length);
      dma_set_read_from_peripheral(DMA1, cUsart1RxDmaChannel);
      dma_enable_memory_increment_mode(DMA1, cUsart1RxDmaChannel);
      dma_set_peripheral_size(DMA1, cUsart1RxDmaChannel, DMA_CCR_PSIZE_8BIT);
      dma_set_memory_size(DMA1, cUsart1RxDmaChannel, DMA_CCR_MSIZE_8BIT);
      dma_set_priority(DMA1, cUsart1RxDmaChannel, DMA_CCR_PL_VERY_HIGH);

      // Enable dma transfer complete interrupt
    	dma_enable_transfer_complete_interrupt(DMA1, cUsart1RxDmaChannel);

    	// Activate dma channel
    	dma_enable_channel(DMA1, cUsart1RxDmaChannel);

      usart_enable_rx_dma(USART1);
    }
    else
    {
      return HwReqAck::HwReqAckBusy;
    }
  }
  return HwReqAck::HwReqAckOk;
}


HwReqAck writeSerial(const uint32_t usart, const uint32_t length, const char* data)
{
  switch (usart)
  {
    case USART2:
    if (USART2_ISR & USART_ISR_TC)
    {
      dma_channel_reset(DMA1, cUsart2TxDmaChannel);
      // Set up tx dma
      dma_set_peripheral_address(DMA1, cUsart2TxDmaChannel, (uint32_t)&USART1_TDR);
      dma_set_memory_address(DMA1, cUsart2TxDmaChannel, (uint32_t)data);
      dma_set_number_of_data(DMA1, cUsart2TxDmaChannel, length);
      dma_set_read_from_memory(DMA1, cUsart2TxDmaChannel);
      dma_enable_memory_increment_mode(DMA1, cUsart2TxDmaChannel);
      dma_set_peripheral_size(DMA1, cUsart2TxDmaChannel, DMA_CCR_PSIZE_8BIT);
      dma_set_memory_size(DMA1, cUsart2TxDmaChannel, DMA_CCR_MSIZE_8BIT);
      dma_set_priority(DMA1, cUsart2TxDmaChannel, DMA_CCR_PL_HIGH);

      // Enable dma transfer complete interrupt
    	dma_enable_transfer_complete_interrupt(DMA1, cUsart2TxDmaChannel);

    	// Activate dma channel
    	dma_enable_channel(DMA1, cUsart2TxDmaChannel);

      usart_enable_tx_dma(USART2);
    }
    else
    {
      return HwReqAck::HwReqAckBusy;
    }
    break;

    default:
    if (USART1_ISR & USART_ISR_TC)
    {
      dma_channel_reset(DMA1, cUsart1TxDmaChannel);
      // Set up tx dma
      dma_set_peripheral_address(DMA1, cUsart1TxDmaChannel, (uint32_t)&USART1_TDR);
      dma_set_memory_address(DMA1, cUsart1TxDmaChannel, (uint32_t)data);
      dma_set_number_of_data(DMA1, cUsart1TxDmaChannel, length);
      dma_set_read_from_memory(DMA1, cUsart1TxDmaChannel);
      dma_enable_memory_increment_mode(DMA1, cUsart1TxDmaChannel);
      dma_set_peripheral_size(DMA1, cUsart1TxDmaChannel, DMA_CCR_PSIZE_8BIT);
      dma_set_memory_size(DMA1, cUsart1TxDmaChannel, DMA_CCR_MSIZE_8BIT);
      dma_set_priority(DMA1, cUsart1TxDmaChannel, DMA_CCR_PL_HIGH);

      // Enable dma transfer complete interrupt
    	dma_enable_transfer_complete_interrupt(DMA1, cUsart1TxDmaChannel);

      USART1_ICR |= USART_ICR_TCCF;

    	// Activate dma channel
    	dma_enable_channel(DMA1, cUsart1TxDmaChannel);

      usart_enable_tx_dma(USART1);
    }
    else
    {
      return HwReqAck::HwReqAckBusy;
    }
  }
  return HwReqAck::HwReqAckOk;
}


void _refreshPeripherals()
{
  if (_refreshRTCNow == true)
  {
    if (_refreshRTC() == HwReqAck::HwReqAckOk)
    {
      _refreshRTCNow = false;
    }
  }

  if (_refreshTempNow == true)
  {
    if (_refreshTemp() == HwReqAck::HwReqAckOk)
    {
      _refreshTempNow = false;
    }
  }
}


void _spiDoNextQueued()
{
  if (_spiXferQueue[0] != nullptr)
  {
    _spiXferQueue[0]->state = HwReqAck::HwReqAckOk;
    _spiXferQueue[0] = nullptr;
  }

  if (_spiXferQueue[1] != nullptr)
  {
    spiTransfer(_spiXferQueue[1]->peripheral, _spiXferQueue[1]->bufferIn, _spiXferQueue[1]->bufferOut, _spiXferQueue[1]->length, _spiXferQueue[1]->use16BitXfers);
    _spiXferQueue[1]->state = HwReqAck::HwReqAckBusy;
    _spiXferQueue[0] = _spiXferQueue[1];
    _spiXferQueue[1] = nullptr;
  }

  _refreshPeripherals();
}


HwReqAck spiTransferRequest(SpiTransferReq* request)
{
  HwReqAck status = HwReqAck::HwReqAckBusy;

  // nvic_disable_irq(NVIC_DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_IRQ);
  // nvic_disable_irq(NVIC_TIM2_IRQ);

  if (_spiXferQueue[1] == nullptr)
  {
    request->state = HwReqAck::HwReqAckQueued;
    _spiXferQueue[1] = request;
    status = HwReqAck::HwReqAckOk;
  }

  // nvic_enable_irq(NVIC_TIM2_IRQ);
  // nvic_enable_irq(NVIC_DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_IRQ);

  return status;
}


HwReqAck spiTransfer(const SpiPeripheral peripheral, uint8_t *bufferIn, uint8_t *bufferOut, const uint16_t length, const bool use16BitXfers)
{
  uint16_t mSize = DMA_CCR_MSIZE_8BIT, pSize = DMA_CCR_PSIZE_8BIT;
  volatile uint8_t temp_data __attribute__ ((unused));

  // ensure the timer interrupt doesn't squash another transfer
  nvic_disable_irq(NVIC_DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_IRQ);
  nvic_disable_irq(NVIC_TIM2_IRQ);

  if (_spiActivePeripheral != SpiPeripheral::None)
  {
    // aaaannnnddd reenable it
    nvic_enable_irq(NVIC_TIM2_IRQ);
    nvic_enable_irq(NVIC_DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_IRQ);
    // Let the caller know it was busy if so
    return HwReqAck::HwReqAckBusy;
  }

  if (use16BitXfers == true)
  {
    mSize = DMA_CCR_MSIZE_16BIT;
    pSize = DMA_CCR_PSIZE_16BIT;
  }

	// Reset SPI data and status registers
	// First, ensure the SPI is not busy...
  while (SPI_SR(SPI1) & (SPI_SR_BSY));
  // ...now we purge the FIFO to ensure it's empty for the new inbound bits.
	while (SPI_SR(SPI1) & (SPI_SR_RXNE | SPI_SR_OVR))
  {
		temp_data = SPI_DR(SPI1);
	}

  // Configure SPI1 for use with the appropriate peripheral
  _spiSelectPeripheral(peripheral);

  // Reset DMA channels
  dma_channel_reset(DMA1, cSpi1RxDmaChannel);
  dma_channel_reset(DMA1, cSpi1TxDmaChannel);

  // Set up rx dma, note it has higher priority to avoid overrun
  dma_set_peripheral_address(DMA1, cSpi1RxDmaChannel, (uint32_t)&SPI1_DR);
  dma_set_memory_address(DMA1, cSpi1RxDmaChannel, (uint32_t)bufferIn);
  dma_set_number_of_data(DMA1, cSpi1RxDmaChannel, length);
  dma_set_read_from_peripheral(DMA1, cSpi1RxDmaChannel);
  dma_enable_memory_increment_mode(DMA1, cSpi1RxDmaChannel);
  dma_set_peripheral_size(DMA1, cSpi1RxDmaChannel, pSize);
  dma_set_memory_size(DMA1, cSpi1RxDmaChannel, mSize);
  dma_set_priority(DMA1, cSpi1RxDmaChannel, DMA_CCR_PL_VERY_HIGH);

  // Set up tx dma
  dma_set_peripheral_address(DMA1, cSpi1TxDmaChannel, (uint32_t)&SPI1_DR);
  dma_set_memory_address(DMA1, cSpi1TxDmaChannel, (uint32_t)bufferOut);
  dma_set_number_of_data(DMA1, cSpi1TxDmaChannel, length);
  dma_set_read_from_memory(DMA1, cSpi1TxDmaChannel);
  dma_enable_memory_increment_mode(DMA1, cSpi1TxDmaChannel);
  dma_set_peripheral_size(DMA1, cSpi1TxDmaChannel, pSize);
  dma_set_memory_size(DMA1, cSpi1TxDmaChannel, mSize);
  dma_set_priority(DMA1, cSpi1TxDmaChannel, DMA_CCR_PL_HIGH);

	// Enable dma transfer complete interrupts
	dma_enable_transfer_complete_interrupt(DMA1, cSpi1RxDmaChannel);
	dma_enable_transfer_complete_interrupt(DMA1, cSpi1TxDmaChannel);

	// Activate dma channels
	dma_enable_channel(DMA1, cSpi1RxDmaChannel);
	dma_enable_channel(DMA1, cSpi1TxDmaChannel);

	/* Enable the SPI transfer via DMA
	 * This will immediately start the transmission, after which when the receive
   * is complete, the receive DMA will activate
	 */
	spi_enable_rx_dma(SPI1);
	spi_enable_tx_dma(SPI1);

  // aaaannnnddd reenable these
  nvic_enable_irq(NVIC_TIM2_IRQ);
  nvic_enable_irq(NVIC_DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_IRQ);

  return HwReqAck::HwReqAckOk;
}


bool spiIsBusy()
{
  return _spiActivePeripheral != SpiPeripheral::None;
}


void setBlueLed(const uint32_t intensity)
{
  if (intensity <= RgbLed::cLedMaxIntensity)
  {
    timer_set_oc_value(cLedPwmTimer, cLed2PwmOc, intensity);
  }
  else
  {
    timer_set_oc_value(cLedPwmTimer, cLed2PwmOc, RgbLed::cLedMaxIntensity);
  }
}


void setGreenLed(const uint32_t intensity)
{
  if (intensity <= RgbLed::cLedMaxIntensity)
  {
    timer_set_oc_value(cLedPwmTimer, cLed1PwmOc, intensity);
  }
  else
  {
    timer_set_oc_value(cLedPwmTimer, cLed1PwmOc, RgbLed::cLedMaxIntensity);
  }
}


void setRedLed(const uint32_t intensity)
{
  if (intensity <= RgbLed::cLedMaxIntensity)
  {
    timer_set_oc_value(cLedPwmTimer, cLed0PwmOc, intensity);
  }
  else
  {
    timer_set_oc_value(cLedPwmTimer, cLed0PwmOc, RgbLed::cLedMaxIntensity);
  }
}


void setStatusLed(const RgbLed led)
{
  setBlueLed(led.getBlue());
  setGreenLed(led.getGreen());
  setRedLed(led.getRed());
}


void blinkStatusLed(const RgbLed led1, const RgbLed led2, uint32_t numberOfBlinks, const uint32_t delayLength)
{
  // be sure this is off so blinking is visible
  DisplayManager::setStatusLedAutoRefreshing(false);

  while (numberOfBlinks-- > 0)
  {
    setStatusLed(led1);
    delay(delayLength);
    setStatusLed(led2);
    delay(delayLength);
  }
}


void delay(const uint32_t length)
{
  _delayCounter = 0;

  while (_delayCounter < length)
  {
    DisplayManager::refresh();
  }
}


void delayNBStart()
{
  _delayCounterNB = 0;
}


bool delayNBComplete(const uint32_t length)
{
  DisplayManager::refresh();

  return (_delayCounterNB >= length);
}


uint32_t bcdToUint32(uint32_t bcdValue)
{
  uint32_t multiplier = 1, result = 0;

  while (bcdValue != 0)
  {
    result += multiplier * (bcdValue & 0xf);
    multiplier *= 10;
    bcdValue = bcdValue >> 4;
  }
  return result;
}


uint32_t uint32ToBcd(uint32_t uint32Value)
{
    uint32_t result = 0;
    uint8_t shift = 0;

    if (uint32Value > 99999999)
    {
      uint32Value = 0;
    }

    while (uint32Value != 0)
    {
        result +=  (uint32Value % 10) << shift;
        uint32Value = uint32Value / 10;
        shift += 4;
    }
    return result;
}


void dmaIsr()
{
  // we use DMA channels 2 and 3 for the SPI
  if ((DMA1_ISR & DMA_ISR_TCIF2) || (DMA1_ISR & DMA_ISR_TCIF3))
  {
    // channel 2 is SPI recieve
    if (DMA1_ISR & DMA_ISR_TCIF2)
  	{
  		DMA1_IFCR |= DMA_IFCR_CTCIF2;

  		dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL2);

  		spi_disable_rx_dma(SPI1);

  		dma_disable_channel(DMA1, DMA_CHANNEL2);
  	}
    // channel 3 is SPI transmit
  	if (DMA1_ISR & DMA_ISR_TCIF3)
  	{
  		DMA1_IFCR |= DMA_IFCR_CTCIF3;

  		dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL3);

  		spi_disable_tx_dma(SPI1);

  		dma_disable_channel(DMA1, DMA_CHANNEL3);
  	}
    // tx & rx should always finish at the same time; wait for the last bits to roll out
    while (SPI_SR(SPI1) & SPI_SR_BSY);
    // latch in the bits if we were sending to the display drivers
    if (_spiActivePeripheral == SpiPeripheral::HvDrivers)
    {
      gpio_set(cNssPort, cNssDisplayPin);   // _spiSelectPeripheral() will clear this for us!
    }
    // release the appropriate CS line
    _spiSelectPeripheral(SpiPeripheral::None);

    // fire off the next queued item to maintain more consistent timing for PWMing
    _spiDoNextQueued();
  }

  if (DMA1_ISR & DMA_ISR_TCIF4)
	{
		DMA1_IFCR |= DMA_IFCR_CTCIF4;

    dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL4);

    // this is terrible. we should track which peripheral was active and disable it accordingly...
    if (cUsart1TxDmaChannel == DMA_CHANNEL4)
    {
      usart_disable_tx_dma(USART1);
    }
    // this is terrible. we should track which peripheral was active and disable it accordingly...
    if (cUsart2TxDmaChannel == DMA_CHANNEL4)
    {
      usart_disable_tx_dma(USART2);
    }

		dma_disable_channel(DMA1, DMA_CHANNEL4);
	}

  if (DMA1_ISR & DMA_ISR_TCIF5)
	{
		DMA1_IFCR |= DMA_IFCR_CTCIF5;

		dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL5);

    // this is terrible. we should track which peripheral was active and disable it accordingly...
    if (cUsart1RxDmaChannel == DMA_CHANNEL5)
    {
      usart_disable_rx_dma(USART1);
    }
    // this is terrible. we should track which peripheral was active and disable it accordingly...
    if (cUsart2RxDmaChannel == DMA_CHANNEL5)
    {
      usart_disable_rx_dma(USART2);

      Dmx512Rx::rxCompleteIsr();
    }

		dma_disable_channel(DMA1, DMA_CHANNEL5);
	}

  if (DMA1_ISR & DMA_ISR_TCIF6)
	{
		DMA1_IFCR |= DMA_IFCR_CTCIF6;

		dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL6);
    // this is terrible. we should track which peripheral was active and disable it accordingly...
    if (cUsart2RxDmaChannel == DMA_CHANNEL6)
    {
      usart_disable_rx_dma(USART2);

      Dmx512Rx::rxCompleteIsr();
    }
    // this is terrible. we should track which peripheral was active and disable it accordingly...
    if (cI2c1TxDmaChannel == DMA_CHANNEL6)
    {
      i2c_disable_txdma(I2C1);

      _i2cState = I2cState::I2cIdle;

      if (_i2cNumberRx > 0)
      {
        i2cReceive(_i2cAddr, _i2cBufferRx, _i2cNumberRx, true);
      }
    }

		dma_disable_channel(DMA1, DMA_CHANNEL6);
	}

  if (DMA1_ISR & DMA_ISR_TCIF7)
	{
		DMA1_IFCR |= DMA_IFCR_CTCIF7;

		dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL7);
    // this is terrible. we should track which peripheral was active and disable it accordingly...
    if (cUsart2TxDmaChannel == DMA_CHANNEL7)
    {
      usart_disable_tx_dma(USART2);
    }
    // this is terrible. we should track which peripheral was active and disable it accordingly...
    if (cI2c1RxDmaChannel == DMA_CHANNEL7)
    {
      i2c_disable_rxdma(I2C1);

      _i2cState = I2cState::I2cIdle;

      _i2cBufferRx = nullptr;
      _i2cNumberRx = 0;
    }

		dma_disable_channel(DMA1, DMA_CHANNEL7);
	}
}


void exti415Isr()
{
  if (exti_get_flag_status(EXTI13) != false)
  {
    _refreshRTCNow = true;
    _refreshTempNow = true;

    _incrementOnTimeSecondsCounter();

    _peripheralRefreshTrigger = PeripheralRefreshTrigger::PpsExti;

    exti_reset_request(EXTI13);
  }

  if (exti_get_flag_status(EXTI15) != false)
  {
    InfraredRemote::tick();

    exti_reset_request(EXTI15);
  }

}


void systickIsr()
{
  // used by delay()
  _delayCounter++;
  _delayCounterNB++;

  // we'll do this if PPS isn't working
  if (_peripheralRefreshTrigger == PeripheralRefreshTrigger::SysTick)
  {
    _refreshRTCNow = true;
    _refreshTempNow = true;

    _incrementOnTimeSecondsCounter();
  }
  // update ADC samples
  if (_adcSampleTimer++ >= cAdcSampleInterval)
  {
    _adcSampleTimer = 0;

    if (++_adcSampleCounter >= cAdcSamplesToAverage)
    {
      _adcSampleCounter = 0;
    }

    _adcSampleSetLight[_adcSampleCounter] = _bufferADC[cAdcPhototransistor];
    _adcSampleSetTemp[_adcSampleCounter] = _bufferADC[cAdcTemperature];
    _adcSampleSetVoltageVddA[_adcSampleCounter] = _bufferADC[cAdcVref];
    _adcSampleSetVoltageBatt[_adcSampleCounter] = _bufferADC[cAdcVbat];
  }

  // update the tone timer and turn off the tone if the timer has expired
  if (_toneTimer > 0)
  {
    if (--_toneTimer == 0)
    {
      if (_toneTimerNext > 0)
      {
        tone(_toneFrequencyNext, _toneTimerNext);
        _toneFrequencyNext = 0;
        _toneTimerNext = 0;
      }
      else
      {
        timer_set_oc_value(cBuzzerTimer, cBuzzerTimerOC, 0);
      }
    }
  }

  if (_batteryMeasuringCounter++ > cBatteryMeasuringTimeout)
  {
    adc_disable_vbat_sensor();
  }
}


void tim2Isr()
{
	if (TIM2_SR & TIM_SR_UIF)
	{
    // clear the event flag
		TIM2_SR &= ~TIM_SR_UIF;
	}
}


void tim7Isr()
{
	if (TIM7_SR & TIM_SR_UIF)
	{
    // clear the event flag
		TIM7_SR &= ~TIM_SR_UIF;
	}
}


void tim15Isr()
{

}


void tim16Isr()
{

}


void tscIsr()
{
  // process acquisition, advance to next channels, and trigger another acquisition
  if (TSC_ISR & TSC_ISR_EOAF)
  {
    // ensure the counter is in bounds -- should already be as initialized above
    // if (_tscChannelCounter >= cTscChannelsPerGroup)
    // {
    //   _tscChannelCounter = 0;
    // }

    _tscAcquisitionValues[_tscChannelCounter + cTscChannelsPerGroup] = TSC_IOGxCR(6);
    _tscAcquisitionValues[_tscChannelCounter] = TSC_IOGxCR(3);

    for (uint8_t i = _tscChannelCounter; i < cTscChannelCount; i += cTscChannelsPerGroup)
    {
      // if the value we just got is less than the known minimum...
      if (_tscAcquisitionValues[i] < _tscMinimums[i])
      {
        // ...update the minimum with the new minimum
        _tscMinimums[i] = _tscAcquisitionValues[i];
      }

      // if the value we just got is greater than the known maximum...
      if (_tscAcquisitionValues[i] > _tscMaximums[i])
      {
        // ...update the maximum with the new value
        _tscMaximums[i] = _tscAcquisitionValues[i];
      }

      // update the sample set with the value we just got
      _tscSampleSets[i][_tscSampleCounter] = _tscAcquisitionValues[i];
    }

    if (++_tscChannelCounter >= cTscChannelsPerGroup)
    {
      _tscChannelCounter = 0;

      if (++_tscSampleCounter >= cTscSamplesToAverage)
      {
        _tscSampleCounter = 0;
      }
    }

    // select the next pair of channels to sample
    TSC_IOCCR = cTscChannelControlBits[_tscChannelCounter];

    TSC_ICR = TSC_ICR_EOAIC;

    /* kick off the next acquisition cycle */
    TSC_CR |= TSC_CR_START;
  }

  // deal with max count error if it happened
  if (TSC_ISR & TSC_ISR_MCEF)
  {
    TSC_ICR = TSC_ICR_MCEIC;
  }
}


void usart1Isr()
{
  // brute-force approace to USART errors here... :/
  if ((USART1_ISR & (USART_ISR_FE | USART_ISR_NF | USART_ISR_ORE | USART_ISR_RXNE | USART_ISR_ABRE)) != 0)
  {
    USART1_ICR = USART_ISR_FE | USART_ISR_NF | USART_ISR_ORE;
    // Clear the RXNE and ABRE flags -- they may be set with the flags above
    USART1_RQR = USART_RQR_ABKRQ | USART_RQR_RXFRQ;
  }
}


void usart2Isr()
{
  if (USART2_ISR & (USART_ISR_FE | USART_ISR_NF | USART_ISR_ORE | USART_ISR_RXNE))
  {
    USART2_ICR = USART_ISR_FE | USART_ISR_NF | USART_ISR_ORE;
  }
}


}

}
