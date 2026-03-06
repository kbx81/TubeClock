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
#include <libopencm3/cm3/scb.h>
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
#include "SpiMaster.h"
#include "Usart.h"
#include "UsbSerial.h"

#if HARDWARE_VERSION == 1
#include "Hardware_v1.h"
#else
#error HARDWARE_VERSION must be defined with a value of 1
#endif

namespace kbxTubeClock::Hardware {

// Bootloader flag -- placed in .noinit so it survives a system reset
// but is NOT preserved across power-on resets.
//
static __attribute__((section(".noinit"))) uint32_t _bootloaderMagic;
static constexpr uint32_t cBootloaderMagic = 0xDEADB007u;

// I2C1 state
//
enum I2cState : uint8_t { I2cIdle, I2cBusy };

// SPI interface initialization objects
//
static const SpiMaster::SpiMasterParams _spi1MasterParams = {
    SPI1,               // spi
    DMA1,               // dmaController
    cSpi1RxDmaChannel,  // channelRx
    cSpi1TxDmaChannel   // channelTx
};

// USART interface initialization objects
//
static const Usart::UsartParams _usartParams[cNumberOfUsarts] = {{
                                                                     cGpsUsart,            // USART
                                                                     DMA1,                 // dmaController
                                                                     cUsart1RxDmaChannel,  // channelRx
                                                                     cUsart1TxDmaChannel   // channelTx
                                                                 },
                                                                 {
                                                                     cDmxUsart,            // USART
                                                                     DMA1,                 // dmaController
                                                                     cUsart2RxDmaChannel,  // channelRx
                                                                     cUsart2TxDmaChannel   // channelTx
                                                                 },
                                                                 {
                                                                     cSerialRemoteUsartRx,  // USART
                                                                     0,                     // dmaController (no DMA)
                                                                     0,                     // channelRx (no DMA)
                                                                     0                      // channelTx (no DMA)
                                                                 },
                                                                 {
                                                                     cSerialRemoteUsartTx,  // USART
                                                                     0,                     // dmaController (no DMA)
                                                                     0,                     // channelRx (no DMA)
                                                                     0                      // channelTx (no DMA)
                                                                 }};

// USART interface initialization objects
//
static const Usart::UsartTransferParams _usartTransferParams[cNumberOfUsarts] = {
    {
        cUsart1BaudRate,      // Baud rate
        cUsart1StopBits,      // Number of stop bits
        cUsart1Parity,        // Parity
        cUsart1Mode,          // Mode (Tx, Rx, Tx+Rx)
        cUsart1FlowControl,   // Flow control
        cUsart1AutoBaud,      // Enable auto-baud rate detection
        cUsart1DataBits,      // Number of data bits
        cUsart1DriverEnable,  // Enable hardware DE output (for RS-485)
        cUsart1SwapTxRx       // TX/RX pin swap
    },
    {
        cUsart2BaudRate,      // Baud rate
        cUsart2StopBits,      // Number of stop bits
        cUsart2Parity,        // Parity
        cUsart2Mode,          // Mode (Tx, Rx, Tx+Rx)
        cUsart2FlowControl,   // Flow control
        cUsart2AutoBaud,      // Enable auto-baud rate detection
        cUsart2DataBits,      // Number of data bits
        cUsart2DriverEnable,  // Enable hardware DE output (for RS-485)
        cUsart2SwapTxRx       // TX/RX pin swap
    },
    {
        cUsart3BaudRate,      // Baud rate
        cUsart3StopBits,      // Number of stop bits
        cUsart3Parity,        // Parity
        cUsart3Mode,          // Mode (Rx only)
        cUsart3FlowControl,   // Flow control
        cUsart3AutoBaud,      // Auto-baud (disabled)
        cUsart3DataBits,      // Number of data bits
        cUsart3DriverEnable,  // DE output (disabled)
        cUsart3SwapTxRx       // TX/RX pin swap (PB10 is normally TX, swapped to RX)
    },
    {
        cUsart4BaudRate,      // Baud rate
        cUsart4StopBits,      // Number of stop bits
        cUsart4Parity,        // Parity
        cUsart4Mode,          // Mode (Tx only)
        cUsart4FlowControl,   // Flow control
        cUsart4AutoBaud,      // Auto-baud (disabled)
        cUsart4DataBits,      // Number of data bits
        cUsart4DriverEnable,  // DE output (disabled)
        cUsart4SwapTxRx       // TX/RX pin swap (disabled)
    }};

// number of ADC channels we're using
//
static const uint8_t cAdcChannelCount = 4;

// indexes of ADC channels we're DMA'ing into our buffer
//
static const uint8_t cAdcPhototransistor = 0;
static const uint8_t cAdcTemperature = 1;
static const uint8_t cAdcVref = 2;
static const uint8_t cAdcVbat = 3;

// how frequently we will sample things (in milliseconds)
//
static const uint16_t cAdcSampleIntervalLight = 500;  // Light sensor sampling (500ms = 2 Hz)
static const uint16_t cAdcSampleIntervalSlow = 2000;  // Temp/voltage sampling (2000ms = 0.5 Hz)

// IIR filter coefficients (shift amounts for division by power of 2)
// Lower shift = faster response but more noise; Higher shift = slower but smoother
// We use upscaling to maintain precision in integer math
//
static const uint8_t cAdcLightFilterShift = 5;        // 1/32 weight (~16s time constant @ 500ms sample rate)
static const uint8_t cAdcTempVoltageFilterShift = 3;  // 1/8 weight (~16s time constant @ 2000ms sample rate)
static const uint8_t cAdcFilterUpscale = 8;           // Scale factor for precision (multiply by 256)

// value used to increase precision in our maths
//
static const int16_t cBaseMultiplier = 1000;

// maximum time to wait before disconnecting Vbat from the ADC's bridge divider
//
static const uint16_t cBatteryMeasuringTimeout = 5000;

// delay (in milliseconds) after PPS edge before reading RTC
// gives external RTC time to update its registers after PPS trigger
static const uint8_t cPpsRtcReadDelay = 10;

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

// minimum frequency allowed for tones
//
static const uint8_t cToneFrequencyMinimum = 92;

// maximum volume level allowed for tones
//
static const uint8_t cToneVolumeMaximum = 7;

// Touch detection thresholds
//
static const int16_t cTscTouchThreshold = 150;    // Delta value to trigger touch detection
static const int16_t cTscReleaseThreshold = 100;  // Delta value to release touch (hysteresis)

// Adaptive baseline tracking speed (shift amount for division)
//  Higher value = slower tracking (more stable but slower to adapt)
//  6 = 1/64 per sample, 7 = 1/128 per sample
static const uint8_t cTscBaselineShift = 6;

// IIR filter speed (shift amount for division)
//  Lower value = faster response but more noise
//  3 = ~8 sample equivalent, 4 = ~16 sample equivalent
static const uint8_t cTscFilterShift = 3;

// calibration voltage used (3.3 volts)
//
static const int32_t cVddCalibrationVoltage = 3300;

// SpiMaster object for SPI
//
static SpiMaster _spi1Master;

// Usart objects for USARTs
//
static Usart _usart[cNumberOfUsarts];

// tracks when to take ADC samples
//
static uint16_t _adcSampleTimerLight = 0;
static uint16_t _adcSampleTimerSlow = 0;

// IIR filtered ADC values (replaces averaging arrays - saves 96 bytes!)
// These use exponential moving average for better noise rejection
// Values are upscaled by cAdcFilterUpscale to maintain precision in integer math
//
static uint32_t _adcFilteredLight = 0;
static uint32_t _adcFilteredTemp = 0;
static uint32_t _adcFilteredVoltageVddA = 0;
static uint32_t _adcFilteredVoltageBatt = 0;

// Cached light level result (0-4095) to avoid redundant calculation
//
static uint16_t _cachedLightLevel = 0;

// Flag set by systickIsr when ADC filtered values are updated
//
static volatile bool _adcValuesUpdated = false;

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

// minimum GPS year considered plausible; rejects cold-start default epochs
//
static const uint16_t cGpsMinValidYear = 2020;

// maximum drift (seconds) between GPS and system time before forcing a re-sync within the same hour
//
static const uint32_t cGpsMaxDriftSeconds = 120;

// last second at which we incremented the counter
//
static uint8_t _onTimeLastSecond = 0;

// seconds of HV on time
//
static uint32_t _onTimeSecondsCounter = 0;

// provides a way to calibrate the STM32 ADC temperature indication (Cx10)
//
static int16_t _temperatureAdjustmentADCCx10 = -90;

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

// tracks which TSC channel(s) we'll sample next
//
static uint8_t _tscChannelCounter = 0;

// adaptive baseline for each channel (tracks untouched capacitance)
//
static uint16_t _tscBaseline[cTscChannelCount];

// filtered sensor readings for each channel
//
static uint16_t _tscFiltered[cTscChannelCount];

// touch state for each channel (bit 0-5 correspond to channels 0-5)
//
static uint8_t _tscTouchState = 0;

// date & time stored by Refresh()
//
static DateTime _currentDateTime;

// per-sensor temperature cache in tenths of degrees Celsius (Cx10)
//  sentinel value (cTempSentinel) indicates sensor not detected/available
static int32_t _temperatureCx10Cached[cTempSensorCount] = {cTempSentinel, cTempSentinel, cTempSentinel, cTempSentinel,
                                                           cTempSentinel};

// flag set when any temperature reading is updated; cleared by temperatureUpdated()
static volatile bool _temperatureUpdated = false;

// state of the BLANK pin
//
static bool _displayBlankingState = true;

// indicates to setDisplayBlankPin() that it should NOT clear the BLANK pin as
//  a display write is in progress with flicker reduction active
volatile static bool _displayBlankForWrite = false;

// true if DS3234 is detected
//
static bool _externalRtcConnected = false;

// state of the HV SHUTDOWN pin
//
static bool _hvState = false;

// true if the external RTC needs to be refreshed
//
volatile static bool _refreshRTCNow = false;

// true if the external temperature sensor needs to be refreshed
//
volatile static bool _refreshTempNow = false;

// countdown timer for delaying RTC read after PPS (in milliseconds)
//  gives external RTC time to update its registers after PPS edge
volatile static uint8_t _ppsDelayCounter = 0;

// true if a non-blocking DS3234 SPI refresh has been queued and we are
//  waiting for the DMA transfer to complete before reading the result
volatile static bool _rtcReadPending = false;

// last raw RTC_TR value read from the STM32 internal RTC
//  used to detect when the shadow register actually advances to a new second;
//  initialized to ~0u (invalid) so the first read always commits an update
static uint32_t _lastRtcTR = ~0u;

// true if a non-blocking temperature sensor SPI refresh has been queued and
//  we are waiting for the DMA transfer to complete before reading the result
volatile static bool _tempReadPending = false;

// indicates if the app can rely on a periodic interrupt (PPS) to trigger
//  external RTC and temperature sensor refreshes
static PeripheralRefreshTrigger _peripheralRefreshTrigger = PeripheralRefreshTrigger::SysTick;

// result of RTC start-up (oscillator status, etc.)
//
static RtcStartResult _rtcStartupResult = RtcStartResult::UnknownError;

// true if RTC is set/valid
//
static bool _rtcIsSet = false;

// true if temp sensor is detected
//
static TempSensorType _externalTemperatureSensor = TempSensorType::STM32ADC;

// a string and some bits used for printf debugging :)
//
// char _buffer[80];
// Usart::UsartTransferReq request;
// request.state = Usart::UsartReqAck::UsartReqAckQueued;
// request.length = sprintf(_buffer, "date: %lx\r\n", dr);
// request.buffer = (uint8_t*)_buffer;
// _usart[0].transmitDma(&request);

// set up the ADC
//
void _adcSetup() {
  uint8_t channelArray[] = {cPhototransistorChannel, ADC_CHANNEL_TEMP, ADC_CHANNEL_VREF, ADC_CHANNEL_VBAT};

  adc_power_off(ADC1);

  adc_calibrate(ADC1);
  while ((ADC_CR(ADC1) & ADC_CR_ADCAL) != 0)
    ;

  adc_power_on(ADC1);
  while ((ADC_ISR(ADC1) & ADC_ISR_ADRDY) == 0)
    ;

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

  ADC_CR(ADC1) |= ADC_CR_ADSTART;  // adc_start_conversion_regular blocks :(
}

// Set STM32's clock to 48 MHz from 8 MHz HSE oscillator
//
void _clockSetup() {
  rcc_clock_setup_in_hse_8mhz_out_48mhz();

  // Enable clocks to various subsystems we'll need
  // PWR must be enabled first as we can't disable the backup domain protection
  //  without it online
  rcc_periph_clock_enable(RCC_PWR);

  // SYSCFG is needed to remap USART DMA channels
  rcc_periph_clock_enable(RCC_SYSCFG_COMP);

  rcc_periph_clock_enable(RCC_DMA);

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
  rcc_periph_clock_enable(RCC_USART3);
  rcc_periph_clock_enable(RCC_USART4);

  rcc_periph_clock_enable(RCC_TSC);
}

// Configure DMA
//
void _dmaSetup() {
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
  dma_set_peripheral_address(DMA1, cAdcDmaChannel, (uint32_t) &ADC1_DR);
  dma_set_memory_address(DMA1, cAdcDmaChannel, (uint32_t) _bufferADC);
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
void _gpioSetup() {
#ifdef ENABLE_PROFILING
  // Configure profiling pin as output for ISR profiling (scope probe)
  gpio_mode_setup(cProfilingPort, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, cProfilingPin);
  gpio_clear(cProfilingPort, cProfilingPin);
#endif

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
  for (uint8_t i = 0; i < cAlarmInputPinCount; i++) {
    gpio_mode_setup(cAlarmInputPort, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, cAlarmInputPins[i]);
  }

  // Next, configure the external PPS/SQW_In pulse-per-second input pin and interrupt
  // Trigger only on rising edge to avoid double-triggering on square wave PPS signal
  gpio_mode_setup(cPpsPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, cPpsPin);
  exti_select_source(EXTI13, cPpsPort);
  exti_set_trigger(EXTI13, EXTI_TRIGGER_RISING);
  exti_enable_request(EXTI13);

  // Finally, configure the external IR remote sensor input pin and interrupt
  gpio_mode_setup(cIrPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, cIrPin);
  exti_select_source(EXTI15, cIrPort);
  exti_set_trigger(EXTI15, EXTI_TRIGGER_BOTH);
  exti_enable_request(EXTI15);
}

// Configure the Independent Watchdog Timer
//
void _iwdgSetup() {
  iwdg_set_period_ms(5000);
  iwdg_start();
}

void _i2cSetup() {
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
  while (gpio_get(cI2c1Port, cI2c1SdaPin) == false) {
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

void _i2cRecover() {
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

void _nvicSetup() {
  // DMA interrupts
  // nvic_set_priority(NVIC_ADC_COMP_IRQ, 0);
  // nvic_enable_irq(NVIC_ADC_COMP_IRQ);
  // nvic_set_priority(NVIC_DMA1_CHANNEL1_IRQ, 0);
  // nvic_enable_irq(NVIC_DMA1_CHANNEL1_IRQ);
  // DMA interrupts set to priority 64 to ensure they don't preempt the critical PWM timer
  nvic_set_priority(cDmaIrqSpi1, 64);
  nvic_enable_irq(cDmaIrqSpi1);
  nvic_set_priority(cDmaIrqUsarts, 64);
  nvic_enable_irq(cDmaIrqUsarts);
  // EXTI interrupts -- PPS and IR sensor inputs
  nvic_set_priority(cPpsIrq, 64);
  nvic_enable_irq(cPpsIrq);
  // PWM timer interrupt (triggers refresh of tubes) - HIGHEST priority (0) for deterministic timing
  nvic_set_priority(cTubePwmTimerIrq, 0);
  nvic_enable_irq(cTubePwmTimerIrq);
  // IR interrupt (overflow = timeout)
  nvic_set_priority(cIrTimerIrq, 64);
  nvic_enable_irq(cIrTimerIrq);
  // TSC interrupt
  nvic_set_priority(cTscIrq, 128);
  nvic_enable_irq(cTscIrq);
  // USART1 interrupts
  nvic_set_priority(cUsart1Irq, 64);
  nvic_enable_irq(cUsart1Irq);
  // USART2 interrupts
  nvic_set_priority(cUsart2Irq, 64);
  nvic_enable_irq(cUsart2Irq);
  // USART3/4 shared interrupt (serial remote control)
  nvic_set_priority(cUsart3_4Irq, 64);
  nvic_enable_irq(cUsart3_4Irq);
}

// Configure RTC and ensure it's running
//
void _rtcSetup() {
  // these values are the power-on defaults for the prescaler.
  // we'll set them anyway to be sure they're there
  const uint32_t async = 127;
  uint32_t sync = 255;
  uint16_t timeout = 474;

  if (rcc_is_osc_ready(RCC_LSE) == false) {
    _rtcStartupResult = RtcStartResult::OscStopped;

    pwr_disable_backup_domain_write_protect();

    // reset the RTC to ensure it's in a known state
    RCC_BDCR |= RCC_BDCR_BDRST;
    RCC_BDCR &= ~RCC_BDCR_BDRST;

    if (_externalRtcConnected == true) {
      rcc_osc_bypass_enable(RCC_LSE);
    }

    rcc_osc_on(RCC_LSE);
    rcc_wait_for_osc_ready(RCC_LSE);

    rcc_set_rtc_clock_source(RCC_LSE);

    rcc_enable_rtc_clock();

    rtc_unlock();

    // enter init mode -- this lets us test that everything is working as expected
    RTC_ISR |= RTC_ISR_INIT;
    while (((RTC_ISR & RTC_ISR_INITF) == 0) && (--timeout > 0))
      ;

    // do a blinky thing to indicate the problem if we had to wait too long :(
    if (timeout == 0) {
      // reset the RTC to undo anything that might've been done above
      RCC_BDCR |= RCC_BDCR_BDRST;
      RCC_BDCR &= ~RCC_BDCR_BDRST;

      _rtcStartupResult = RtcStartResult::OscTimeout;
    }

    // set synch prescaler, using defaults for 1Hz out
    rtc_set_prescaler(sync, async);

    // exit init mode
    RTC_ISR &= ~(RTC_ISR_INIT);

    rtc_lock();

    // And wait for synchro...
    rtc_wait_for_synchro();
    pwr_enable_backup_domain_write_protect();
  } else {
    _rtcStartupResult = RtcStartResult::Ok;
  }
}

// Configure SPI
//
void _spiSetup() {
  // Configure SPI instance with SPI hardware and DMA channels
  _spi1Master.initialize(&_spi1MasterParams);

  // Setup SPI1 pins as alternate function zero
  gpio_set_af(cSpi1Port, GPIO_AF0, cSpi1SckPin | cSpi1MisoPin | cSpi1MosiPin);
  // MISO=PB4 from HV drivers
  gpio_set_af(cSpi1AltPort, GPIO_AF0, cSpi1MisoDisplayPin);
  // Configure GPIOs
  gpio_mode_setup(cSpi1Port, GPIO_MODE_AF, GPIO_PUPD_NONE, cSpi1SckPin | cSpi1MosiPin);
}

// Set up timer to fire every x milliseconds
// This is a unusual usage of systick, be very careful with the 24bit range
// of the systick counter!  You can range from 1 to 2796ms with this.
//
void _systickSetup(const uint16_t xms) {
  // div8 per ST, stays compatible with M3/M4 parts, well done ST
  systick_set_clocksource(STK_CSR_CLKSOURCE_EXT);
  // Lower SysTick priority to 128 so both TIM2 PWM ISR (priority 0) and the
  // SPI DMA complete ISR (priority 64) can preempt it. This ensures the HV5622
  // latch strobe happens promptly after each DMA transfer completes, giving
  // consistent output timing critical for flicker-free low-intensity PWM.
  // On Cortex-M0, SHPR3[31:30] controls SysTick priority (2 bits = 4 levels: 0, 64, 128, 192)
  SCB_SHPR3 |= (128 << 24);
  // clear counter so it starts right away
  STK_CVR = 0;

  systick_set_reload(rcc_ahb_frequency / 8 / 1000 * xms);
  systick_counter_enable();
  systick_interrupt_enable();
}

// Set up the timer for the beeper/buzzer
//
void _timerSetupBeeper() {
  // Set the timer's global mode to:
  // - use no divider
  // - alignment edge
  // - count direction up
  timer_set_mode(cBuzzerTimer, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
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
void _timerSetupStatusLED() {
  // Set the timer's global mode to:
  // - use no divider
  // - alignment edge
  // - count direction up
  timer_set_mode(cLedPwmTimer, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
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

  // Configure status LED GPIO pins
  gpio_mode_setup(cLedPort, GPIO_MODE_AF, GPIO_PUPD_NONE, cLed0Pin | cLed1Pin | cLed2Pin);
  gpio_set_af(cLedPort, cLedPortAF, cLed0Pin | cLed1Pin | cLed2Pin);
  gpio_set_output_options(cLedPort, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, cLed0Pin | cLed1Pin | cLed2Pin);
}

// Set up the timer for measuring pulses on the IR remote sensor input
//
void _timerSetupIR() {
  // Set the timer's global mode to:
  // - use no divider
  // - alignment edge
  // - count direction up
  timer_set_mode(cIrTimer, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

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
void _timerSetupPWM() {
  // Set the timer's global mode to:
  // - use no divider
  // - alignment edge
  // - count direction up
  timer_set_mode(cTubePwmTimer, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

  timer_set_prescaler(cTubePwmTimer, 0);
  timer_set_repetition_counter(cTubePwmTimer, 0);
  timer_continuous_mode(cTubePwmTimer);
  // Timer frequency: 48MHz / (3125+1) = 15,360 Hz (period = 65.10µs)
  // With 256 PWM steps (uint8_t rollover), full PWM cycle = 256/15360 = 16.67ms (60Hz)
  timer_set_period(cTubePwmTimer, 3125);

  timer_enable_preload(cTubePwmTimer);

  timer_enable_irq(cTubePwmTimer, TIM_DIER_UIE);

  timer_enable_counter(cTubePwmTimer);
}

// Set up the Timers
//
void _timerSetup() {
  _timerSetupStatusLED();
  _timerSetupBeeper();
  _timerSetupIR();
  _timerSetupPWM();
}

// Set up the Touch Sensing Controller
//
void _tscSetup() {
  gpio_mode_setup(cTscPort, GPIO_MODE_AF, GPIO_PUPD_NONE, cTscTouchKeyPins | cTscSamplingCapPins);
  gpio_set_output_options(cTscPort, GPIO_OTYPE_OD, GPIO_OSPEED_MED, cTscSamplingCapPins);
  gpio_set_output_options(cTscPort, GPIO_OTYPE_PP, GPIO_OSPEED_MED, cTscTouchKeyPins);
  gpio_set_af(cTscPort, GPIO_AF3, cTscTouchKeyPins | cTscSamplingCapPins);

  gpio_mode_setup(cTscPortC, GPIO_MODE_AF, GPIO_PUPD_NONE, cTscTouchKeyPinsC);
  gpio_set_output_options(cTscPortC, GPIO_OTYPE_PP, GPIO_OSPEED_MED, cTscTouchKeyPinsC);
  gpio_set_af(cTscPortC, GPIO_AF0, cTscTouchKeyPinsC);

  // To allow the control of the sampling capacitor I/O by the TSC peripheral,
  //  the corresponding GPIO must be first set to alternate output open drain
  //  mode and then the corresponding Gx_IOy bit in the TSC_IOSCR register must
  //  be set.
  TSC_IOSCR = cTscSamplingControlBits;

  _tscChannelCounter = 0;

  // To allow the control of the channel I/O by the TSC peripheral, the
  //  corresponding GPIO must be first set to alternate output push-pull mode
  //  and the corresponding Gx_IOy bit in the TSC_IOCCR register must be set.
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

// Set up USARTs
//
void _usartSetup() {
  // Setup USART1 TX & RX pins as alternate function
  gpio_set_af(cUsart1TxPort, cUsart1TxAF, cUsart1TxPin);
  gpio_set_af(cUsart1RxPort, cUsart1RxAF, cUsart1RxPin);
  gpio_mode_setup(cUsart1RxPort, GPIO_MODE_AF, GPIO_PUPD_NONE, cUsart1RxPin);
  gpio_mode_setup(cUsart1TxPort, GPIO_MODE_AF, GPIO_PUPD_NONE, cUsart1TxPin);
  // Setup USART2 TX, RX, and DE pins as alternate function
  gpio_set_af(cUsart2Port, cUsart2AF, cUsart2RxPin | cUsart2TxPin | cUsart2DePin);
  gpio_mode_setup(cUsart2Port, GPIO_MODE_AF, GPIO_PUPD_NONE, cUsart2RxPin | cUsart2TxPin | cUsart2DePin);
  // Setup USART3 RX pin as alternate function (pull-up: USART idle = HIGH)
  gpio_set_af(cUsart3RxPort, cUsart3RxAF, cUsart3RxPin);
  gpio_mode_setup(cUsart3RxPort, GPIO_MODE_AF, GPIO_PUPD_PULLUP, cUsart3RxPin);
  // Setup USART4 TX pin as alternate function
  gpio_set_af(cUsart4TxPort, cUsart4TxAF, cUsart4TxPin);
  gpio_mode_setup(cUsart4TxPort, GPIO_MODE_AF, GPIO_PUPD_NONE, cUsart4TxPin);

  // Configure all Usart instances
  for (uint8_t usart = 0; usart < cNumberOfUsarts; usart++) {
    _usart[usart].initialize(&_usartParams[usart]);
    _usart[usart].configure(&_usartTransferParams[usart]);
  }

  // Enable relevant interrupts
  _usart[0].enableRxInterrupt();  // USART1 GPS RX
  usart_enable_error_interrupt(cGpsUsart);
  usart_enable_error_interrupt(cDmxUsart);
  // NOTE: USART3 RX interrupt is enabled later by SerialRemote::initialize()
  // to avoid unhandled RXNE interrupts before the ISR handler is ready.

  // Enable all USARTs
  for (uint8_t usart = 0; usart < cNumberOfUsarts; usart++) {
    _usart[usart].enable();
  }
}

void _incrementOnTimeSecondsCounter() {
  if (_onTimeLastSecond == _currentDateTime.second() || !_hvState) {
    return;
  }

  bool dbpState = (PWR_CR & PWR_CR_DBP);

  _onTimeLastSecond = _currentDateTime.second();

  pwr_disable_backup_domain_write_protect();

  RTC_BKPXR(0) = ++_onTimeSecondsCounter;

  if (dbpState == false) {
    pwr_enable_backup_domain_write_protect();
  }

  // if connected, save to DS3234 RAM, also
  // if (_externalRtcConnected == true)
  // {
  //   DS3234::writeSram(0x00, (uint8_t*)&_onTimeSecondsCounter, 4, false);
  // }
}

void _refreshTemp() {
  // Read ALL detected temperature sensors (not just the active one) so the
  // serial remote can report every sensor's value.  Phase 1 handles immediate
  // reads (internal ADC, DS3234 cached registers) and queues SPI reads.
  // Phase 2 harvests SPI results on subsequent calls.

  // --- Phase 1: immediate reads + queue SPI ---
  if ((_refreshTempNow == true) && (_tempReadPending == false)) {
    // Internal ADC temperature (always available)
    {
      int32_t averageTemp = _adcFilteredTemp >> cAdcFilterUpscale;
      int32_t t = (averageTemp * voltageVddA() / cVddCalibrationVoltage) - ST_TSENSE_CAL1_30C;
      t = t * (110 - 30) * cBaseMultiplier;
      t = t / (ST_TSENSE_CAL2_110C - ST_TSENSE_CAL1_30C);
      t = t + 30000;
      _temperatureCx10Cached[TempSensorType::STM32ADC] = t / (cBaseMultiplier / 10) + _temperatureAdjustmentADCCx10;
    }

    // DS3234 temperature is updated in _refreshRTC() Phase 2, where the
    // register snapshot is guaranteed fresh.

    // SPI temperature sensor -- LM74 and DS1722 are mutually exclusive
    if (LM74::isConnected()) {
      SpiMaster::SpiReqAck status = LM74::refresh();
      if ((status == SpiMaster::SpiReqAck::SpiReqAckOk) || (status == SpiMaster::SpiReqAck::SpiReqAckQueued)) {
        _tempReadPending = true;
      }
    } else if (DS1722::isConnected()) {
      SpiMaster::SpiReqAck status = DS1722::refresh();
      if ((status == SpiMaster::SpiReqAck::SpiReqAckOk) || (status == SpiMaster::SpiReqAck::SpiReqAckQueued)) {
        _tempReadPending = true;
      }
    }

    _refreshTempNow = false;

    if (!_tempReadPending) {
      // No SPI read queued -- all readings are complete this cycle
      _temperatureUpdated = true;
    }
  }

  // --- Phase 2: harvest SPI result ---
  if (_tempReadPending == true) {
    bool harvested = false;

    if (LM74::isConnected() && (LM74::transferComplete() == true)) {
      _temperatureCx10Cached[TempSensorType::LM74] = LM74::getTemperatureCx10();
      harvested = true;
    } else if (DS1722::isConnected() && (DS1722::transferComplete() == true)) {
      _temperatureCx10Cached[TempSensorType::DS1722] = DS1722::getTemperatureCx10();
      harvested = true;
    }

    if (harvested) {
      _tempReadPending = false;
      _temperatureUpdated = true;
    }
  }
}

void _refreshRTC() {
  if (_externalRtcConnected) {
    // Phase 1: queue the SPI refresh (non-blocking)
    if (_refreshRTCNow) {
      SpiMaster::SpiReqAck status = DS3234::refresh();  // non-blocking
      if ((status == SpiMaster::SpiReqAck::SpiReqAckOk) || (status == SpiMaster::SpiReqAck::SpiReqAckQueued)) {
        _rtcReadPending = true;
        _refreshRTCNow = false;
      }
    }

    // Phase 2: harvest the result once DMA completes
    if (_rtcReadPending && DS3234::transferComplete()) {
      _currentDateTime = DS3234::getDateTime();
      // Also update cached temperature -- the DS3234 register snapshot already
      // contains the temperature bytes, and _refreshTemp() cannot read them
      // while _rtcReadPending is true (which it always is when _refreshTempNow
      // fires, because both flags are set simultaneously).
      if (DS3234::isConnected()) {
        _temperatureCx10Cached[TempSensorType::DS3234] = DS3234::getTemperatureCx10();
      }
      _rtcReadPending = false;
    }
    return;
  }

  if (!_refreshRTCNow) {
    return;
  }

  // Read TR first per STM32 RM: reading TR latches the shadow registers so DR
  // is consistent.  Also use TR as a change-detection sentinel: when the GPS
  // PPS fires we read here 10 ms later, but the STM32 RTC's own second
  // boundary may not have crossed yet (phase offset from setDateTime()).
  // If TR hasn't changed, leave _refreshRTCNow true so SysTick retries every
  // 1 ms until the boundary fires.  This prevents display skips caused by the
  // phase drifting across the cPpsRtcReadDelay threshold.
  uint32_t tr = RTC_TR;
  uint32_t dr = RTC_DR;  // MUST always read DR after TR to release the shadow-register
                         // latch (STM32 RM §25.4.8): reading TR locks shadow regs until
                         // DR is read; skipping this read prevents any future updates.
  if (tr == _lastRtcTR) {
    return;  // RTC hasn't ticked yet; retry next SysTick
  }
  _lastRtcTR = tr;

  uint16_t year = 2000;
  uint8_t month, day, hour, minute, second;

  year += (10 * ((dr >> RTC_DR_YT_SHIFT) & RTC_DR_YT_MASK));
  year += ((dr >> RTC_DR_YU_SHIFT) & RTC_DR_YU_MASK);
  // 2019-10-05 - RTC_DR_MT_MASK is incorrect in rtc_common_l1f024.h
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

  _refreshRTCNow = false;
}

void _refreshPeripherals() {
  _refreshRTC();
  _refreshTemp();
}

void _syncRtcWithGps() {
  // Only read GPS time when we might actually sync (avoids reading GPS data every loop)
  // Only sync when not waiting for PPS refresh to avoid race with _currentDateTime update
  if (!GpsReceiver::isConnected() || !GpsReceiver::isValid() || _ppsDelayCounter != 0) {
    return;
  }

  DateTime gpsTime = GpsReceiver::getLocalDateTime();

  // Reject implausible GPS time (e.g., cold-start default epoch year)
  if (gpsTime.year() < cGpsMinValidYear) {
    return;
  }

  // Sync RTCs when the hour changes OR when GPS time has drifted significantly from system
  // time -- the drift check recovers from a bad time accepted at the first GPS fix
  int32_t drift = _currentDateTime.secondsTo(gpsTime);
  if (drift < 0) {
    drift = -drift;
  }

  if (gpsTime.hour() != _lastRtcGpsSyncHour || (uint32_t) drift > cGpsMaxDriftSeconds) {
    setDateTime(gpsTime);
    _lastRtcGpsSyncHour = gpsTime.hour();
  }
}

bool isBootloaderFlagSet() { return (_bootloaderMagic == cBootloaderMagic); }

void enterBootloader() {
  _bootloaderMagic = 0;
  asm volatile("cpsid i" ::: "memory");  // disable all interrupts (set PRIMASK)
  // STM32F072 system bootloader is at system memory base 0x1FFFC800.
  // Word 0 is the initial stack pointer; word 1 is the reset handler address.
  const volatile uint32_t *sysboot = reinterpret_cast<const volatile uint32_t *>(0x1FFFC800u);
  uint32_t sp = sysboot[0];
  uint32_t pc = sysboot[1];
  asm volatile("msr msp, %0" ::"r"(sp) : "memory");
  reinterpret_cast<void (*)()>(pc)();
  while (true) {
  }  // unreachable; satisfies [[noreturn]]
}

void setBootloaderFlag() { _bootloaderMagic = cBootloaderMagic; }

void clearBootloaderFlag() { _bootloaderMagic = 0; }

void systemReset() { scb_reset_system(); }

// Starts it all up
//
void initialize() {
  // First, set up all the hardware things
  _clockSetup();
  _iwdgSetup();
  _gpioSetup();
  _timerSetup();
  _dmaSetup();
  _adcSetup();
  _tscSetup();

  _spiSetup();
  _usartSetup();
  // _i2cSetup();

  DisplayManager::initialize();
  DS3234::initialize();
  LM74::initialize();
  DS1722::initialize();

  _systickSetup(1);  // tick every 1 mS
  _nvicSetup();

  UsbSerial::initialize();

  Dmx512Rx::initialize();

  // Initialize IIR filters with current ADC readings (upscaled for precision)
  _adcFilteredLight = (uint32_t) _bufferADC[cAdcPhototransistor] << cAdcFilterUpscale;
  _adcFilteredTemp = (uint32_t) _bufferADC[cAdcTemperature] << cAdcFilterUpscale;
  _adcFilteredVoltageVddA = (uint32_t) _bufferADC[cAdcVref] << cAdcFilterUpscale;
  _adcFilteredVoltageBatt = (uint32_t) _bufferADC[cAdcVbat] << cAdcFilterUpscale;
  _cachedLightLevel = _bufferADC[cAdcPhototransistor];

  // Initialize TSC baseline and filter values
  // Start with mid-range baseline; will adapt quickly during first acquisitions
  for (_tscChannelCounter = 0; _tscChannelCounter < cTscChannelCount; _tscChannelCounter++) {
    _tscBaseline[_tscChannelCounter] = 2048;  // Mid-range initial baseline
    _tscFiltered[_tscChannelCounter] = 2048;  // Mid-range initial filter
  }
  _tscTouchState = 0;  // All buttons released initially

  // Here we check for other temperature sensors
  if (LM74::checkConnection() == true) {
    _externalTemperatureSensor = TempSensorType::LM74;
  } else {
    gpio_clear(cNssPort, cNssTemperaturePin);

    if (DS1722::checkConnection() == true) {
      _externalTemperatureSensor = TempSensorType::DS1722;
    }
  }

  _externalRtcConnected = DS3234::checkConnection();

  if (_externalRtcConnected == true) {
    uint8_t buffer[] = {0, 0x08};

    // checkConnection() does a blocking refresh() so _ds3234Registers are valid here;
    // populate _currentDateTime now so Application::dateTime() returns real data immediately
    // instead of zeros, which would persist until the first PPS-triggered DMA refresh.
    _rtcIsSet = DS3234::isValid();
    if (_rtcIsSet) {
      _currentDateTime = DS3234::getDateTime();
    }
    // enable PPS and 32 kHz outputs
    while (DS3234::setRegister(0x0e, buffer, 2, true) != SpiMaster::SpiReqAck::SpiReqAckOk)
      ;
    if (_externalTemperatureSensor == TempSensorType::STM32ADC) {
      _externalTemperatureSensor = TempSensorType::DS3234;
    }
  } else {
    _rtcIsSet = RTC_ISR & RTC_ISR_INITS;
  }

  _rtcSetup();

  _onTimeSecondsCounter = RTC_BKPXR(0);
}

// Should be called frequently from the main loop
//
void refresh() {
  iwdg_reset();

  // _refreshPeripherals();
  _syncRtcWithGps();

  DisplayManager::refresh();
}

HwReqAck tick() { return tone(cToneFrequencyMinimum * 8, 2); }

HwReqAck tone(const uint16_t frequency, const uint16_t duration) {
  // period = (48000000 / 8 (prescaler)) / frequency
  // oc_value = period / 2
  if (_toneTimer == 0) {
    if ((frequency >= cToneFrequencyMinimum) && (_toneVolume < cToneVolumeMaximum)) {
      // processor speed divided by prescaler...
      uint32_t period = (48000000 / 8) / frequency, ocValue = period / (2 << _toneVolume);
      timer_set_period(cBuzzerTimer, period);
      timer_set_oc_value(cBuzzerTimer, cBuzzerTimerOC, ocValue);
    } else {
      // timer_set_period(cBuzzerTimer, 1);
      timer_set_oc_value(cBuzzerTimer, cBuzzerTimerOC, 0);
    }

    _toneTimer = duration;

    return HwReqAck::HwReqAckOk;
  } else if (_toneTimerNext == 0) {
    _toneTimerNext = duration;
    _toneFrequencyNext = frequency;

    return HwReqAck::HwReqAckBusy;
  } else {
    return HwReqAck::HwReqAckError;
  }
}

bool alarmInput(const uint8_t alarmInputNumber) {
  if (alarmInputNumber >= cAlarmInputPinCount) {
    return false;
  }

  // if the pin is low, return true (alarm is active if pin is pulled down)
  return (gpio_get(cAlarmInputPort, cAlarmInputPins[alarmInputNumber]) == 0);
}

bool button(const uint8_t button) { return (_buttonStates & (1 << button)) != 0; }

uint8_t buttons() { return _buttonStates; }

uint8_t buttonsPressed() { return _buttonStates & _buttonsChanged; }

uint8_t buttonsReleased() { return ~_buttonStates & _buttonsChanged; }

void buttonsRefresh() {
  // Touch detection is performed entirely in the ISR (tscIsr)
  // Here we simply copy the ISR-computed state and track changes
  _buttonsChanged = _buttonStates ^ _buttonStatesPrevious;
  _buttonStatesPrevious = _buttonStates;
}

void setHvState(const bool hvEnabled) {
  if (_hvState == hvEnabled) {
    return;
  }

  _hvState = hvEnabled;

  if (hvEnabled == false) {
    gpio_set(cHvShutdownPort, cHvShutdownPin);
  } else {
    gpio_clear(cHvShutdownPort, cHvShutdownPin);
  }
}

bool getHvState() { return _hvState; }

void setDisplayHardwareBlanking(const bool blankingState) {
  if (_displayBlankingState == blankingState) {
    return;
  }

  _displayBlankingState = blankingState;

  if (blankingState == false) {
    // blank the display
    gpio_set(cBlankDisplayPort, cBlankDisplayPin);
  } else {
    gpio_clear(cBlankDisplayPort, cBlankDisplayPin);
  }
}

// bool dstState()
// {
//   return RTC_CR & RTC_CR_BKP;
// }

DateTime dateTime() {
  // if ((GpsReceiver::isConnected() == true) && (GpsReceiver::isValid() == true))
  // {
  //   return GpsReceiver::getLocalDateTime();
  // }
  return _currentDateTime;
}

void setDateTime(const DateTime &dateTime) {
  _currentDateTime = dateTime;
  _rtcIsSet = true;

  if (_externalRtcConnected == true) {
    DS3234::setDateTime(_currentDateTime);
  }
  // set the internal RTC regardless
  uint8_t dayOfWeek = _currentDateTime.dayOfWeek();

  // STM32 RTC does not accept 0; it uses 7 for Sunday, instead
  if (dayOfWeek == 0) {
    dayOfWeek = 7;
  }

  uint32_t dr = ((_currentDateTime.yearShort(true) & 0xff) << RTC_DR_YU_SHIFT) |
                ((_currentDateTime.month(true) & 0x1f) << RTC_DR_MU_SHIFT) |
                ((_currentDateTime.day(true) & 0x3f) << RTC_DR_DU_SHIFT) | ((dayOfWeek) << RTC_DR_WDU_SHIFT);

  uint32_t tr = ((_currentDateTime.hour(true, false) & 0x3f) << RTC_TR_HU_SHIFT) |
                ((_currentDateTime.minute(true) & 0x7f) << RTC_TR_MNU_SHIFT) |
                ((_currentDateTime.second(true) & 0x7f) << RTC_TR_SU_SHIFT);

  pwr_disable_backup_domain_write_protect();
  rtc_unlock();

  // enter init mode
  RTC_ISR |= RTC_ISR_INIT;
  while ((RTC_ISR & RTC_ISR_INITF) == 0)
    ;

  RTC_DR = dr;
  RTC_TR = tr;

  // exit init mode
  RTC_ISR &= ~(RTC_ISR_INIT);

  rtc_lock();
  rtc_wait_for_synchro();
  // Reset sentinel so the next _refreshRTC() retry unconditionally commits the
  // freshly-written time rather than skipping because TR happens to equal the
  // previous value.
  _lastRtcTR = ~0u;
  pwr_enable_backup_domain_write_protect();

  return;
}

bool rtcIsSet() { return _rtcIsSet; }

int32_t temperature(const bool fahrenheit, const bool bcd) {
  int32_t tempCx10 = _temperatureCx10Cached[_externalTemperatureSensor];

  if (fahrenheit == true) {
    int32_t tempF = (tempCx10 * 18) / 100 + 32;
    if (bcd == true) {
      return uint32ToBcd((uint16_t) tempF);
    }
    return tempF;
  } else {
    int32_t tempC = tempCx10 / 10;
    if (bcd == true) {
      return uint32ToBcd((uint16_t) tempC);
    }
    return tempC;
  }
}

int32_t temperatureCx10() { return _temperatureCx10Cached[_externalTemperatureSensor]; }

int32_t temperatureCx10(TempSensorType type) {
  if (type < cTempSensorCount) {
    return _temperatureCx10Cached[type];
  }
  return cTempSentinel;
}

bool temperatureUpdated() {
  if (_temperatureUpdated) {
    _temperatureUpdated = false;
    return true;
  }
  return false;
}

uint16_t lightLevel() {
  // Return cached IIR-filtered light level (updated in systickIsr)
  // No computation needed - saves ~200 cycles per call!
  return _cachedLightLevel;
}

uint32_t onTimeSeconds() { return _onTimeSecondsCounter; }

void onTimeSecondsReset() { _onTimeSecondsCounter = 0; }

uint16_t voltageBatt() {
  _batteryMeasuringCounter = 0;
  adc_enable_vbat_sensor();

  // Use IIR filtered battery voltage (downscale to actual ADC range)
  int32_t adcSampleAverage = _adcFilteredVoltageBatt >> cAdcFilterUpscale;
  // Determine the voltage
  return cVddCalibrationVoltage * (int32_t) ST_VREFINT_CAL / adcSampleAverage;
}

uint16_t voltageVddA() {
  // Use IIR filtered VddA voltage (downscale to actual ADC range)
  int32_t adcSampleAverage = _adcFilteredVoltageVddA >> cAdcFilterUpscale;
  // Determine the voltage as it affects the temperature calculation
  return cVddCalibrationVoltage * (int32_t) ST_VREFINT_CAL / adcSampleAverage;
}

bool adcValuesUpdated() {
  if (_adcValuesUpdated) {
    _adcValuesUpdated = false;
    return true;
  }
  return false;
}

void setFlickerReduction(const uint16_t value) { _blankingThreshold = value; }

void setTemperatureCalibration(TempSensorType type, int16_t offsetCx10) {
  switch (type) {
    case TempSensorType::STM32ADC:
      _temperatureAdjustmentADCCx10 = offsetCx10;
      break;
    case TempSensorType::DS3234:
      DS3234::setTemperatureOffset(offsetCx10);
      break;
    case TempSensorType::DS1722:
      DS1722::setTemperatureOffset(offsetCx10);
      break;
    case TempSensorType::LM74:
      LM74::setTemperatureOffset(offsetCx10);
      break;
    default:
      break;
  }
}

void setTemperature(const int32_t temperatureCx10) {
  _temperatureCx10Cached[TempSensorType::ExternalSerial] = temperatureCx10;
  _externalTemperatureSensor = TempSensorType::ExternalSerial;
  _temperatureUpdated = true;
}

void setVolume(const uint8_t volumeLevel) {
  if (volumeLevel > cToneVolumeMaximum) {
    _toneVolume = 0;
  } else {
    _toneVolume = cToneVolumeMaximum - volumeLevel;
  }
}

uint32_t getCRC(uint32_t *inputData, uint32_t numElements) {
  crc_reset();

  return crc_calculate_block(inputData, numElements);
}

PeripheralRefreshTrigger getPeripheralRefreshTrigger() { return _peripheralRefreshTrigger; }

bool getPpsInputState() { return gpio_get(Hardware::cPpsPort, Hardware::cPpsPin) != 0; }

RtcStartResult getRTCStartupResult() { return _rtcStartupResult; }

RtcType getRTCType() {
  if (_externalRtcConnected == true) {
    return RtcType::DS323x;
  }
  return RtcType::Stm32;
}

TempSensorType getTempSensorType() { return _externalTemperatureSensor; }

bool setTempSensorType(TempSensorType type) {
  switch (type) {
    case TempSensorType::STM32ADC:
      break;
    case TempSensorType::ExternalSerial:
      if (_temperatureCx10Cached[TempSensorType::ExternalSerial] == cTempSentinel) {
        return false;
      }
      break;
    case TempSensorType::DS3234:
      if (!DS3234::isConnected()) {
        return false;
      }
      break;
    case TempSensorType::DS1722:
      if (!DS1722::isConnected()) {
        return false;
      }
      break;
    case TempSensorType::LM74:
      if (!LM74::isConnected()) {
        return false;
      }
      break;
    default:
      return false;
  }
  _externalTemperatureSensor = type;
  return true;
}

bool isTempSensorDetected(TempSensorType type) {
  switch (type) {
    case TempSensorType::STM32ADC:
      return true;
    case TempSensorType::ExternalSerial:
      return _temperatureCx10Cached[TempSensorType::ExternalSerial] != cTempSentinel;
    case TempSensorType::DS3234:
      return DS3234::isConnected();
    case TempSensorType::DS1722:
      return DS1722::isConnected();
    case TempSensorType::LM74:
      return LM74::isConnected();
    default:
      return false;
  }
}

uint32_t eraseFlash(uint32_t startAddress) {
  uint32_t pageAddress = startAddress, flashStatus = 0;

  // check if startAddress is in proper range
  if ((startAddress - FLASH_BASE) >= (cFlashPageSize * (cFlashPageNumberMaximum + 1))) {
    return 0xff;
  }

  // calculate current page address
  if ((startAddress % cFlashPageSize) != 0) {
    pageAddress -= (startAddress % cFlashPageSize);
  }

  flash_unlock();

  // Erase page(s)
  flash_erase_page(pageAddress);
  flashStatus = flash_get_status_flags();
  if (flashStatus == FLASH_SR_EOP) {
    flashStatus = 0;
  }

  flash_lock();

  return flashStatus;
}

void readFlash(uint32_t startAddress, uint16_t numElements, uint8_t *outputData) {
  uint16_t i;
  uint32_t *memoryPtr = (uint32_t *) startAddress;

  for (i = 0; i < numElements / 4; i++) {
    *(uint32_t *) outputData = *(memoryPtr + i);
    outputData += 4;
  }
}

uint32_t writeFlash(uint32_t startAddress, uint8_t *inputData, uint16_t numElements) {
  uint16_t i;
  const uint32_t erasedValue = 0xffffffff;
  uint32_t currentAddress = startAddress, pageAddress = startAddress, flashStatus = 0, returnStatus = 0;

  // check if startAddress is in proper range
  if ((startAddress - FLASH_BASE) >= (cFlashPageSize * (cFlashPageNumberMaximum + 1))) {
    return 0xff;
  }

  // calculate current page address
  if ((startAddress % cFlashPageSize) != 0) {
    pageAddress -= (startAddress % cFlashPageSize);
  }

  flash_unlock();

  // Erasing page
  flash_erase_page(pageAddress);
  flashStatus = flash_get_status_flags();
  if (flashStatus != FLASH_SR_EOP) {
    returnStatus = flashStatus;
  } else {
    // verify flash memory was completely erased
    for (i = 0; i < numElements; i += 4) {
      // verify if address was erased properly
      if (*((uint32_t *) (currentAddress + i)) != erasedValue) {
        returnStatus = 0x40;
        break;
      }
    }

    if (returnStatus == 0) {
      // programming flash memory
      for (i = 0; i < numElements; i += 4) {
        // programming word data
        flash_program_word(currentAddress + i, *((uint32_t *) (inputData + i)));
        flashStatus = flash_get_status_flags();
        if (flashStatus != FLASH_SR_EOP) {
          returnStatus = 0x80 | flashStatus;
          break;
        }

        // verify if correct data is programmed
        if (*((uint32_t *) (currentAddress + i)) != *((uint32_t *) (inputData + i))) {
          returnStatus = 0x80 | FLASH_SR_PGERR;
          break;
        }
      }
    }
  }

  flash_lock();

  return returnStatus;
}

HwReqAck i2cTransfer(const uint8_t addr, const uint8_t *bufferTx, size_t numberTx, uint8_t *bufferRx, size_t numberRx) {
  // this could be arranged a little better but this way prevents the DMA
  //  complete interrupt from being called before the receive data is populated
  if (_i2cState != I2cState::I2cIdle) {
    if (_i2cBusyFailCount++ > cI2cMaxFailBusyCount) {
      _i2cRecover();
    }
    // Let the caller know it was busy if so
    return HwReqAck::HwReqAckBusy;
  }

  if (numberTx > 0) {
    _i2cAddr = addr;
    _i2cBufferRx = bufferRx;
    _i2cNumberRx = numberRx;

    return i2cTransmit(addr, bufferTx, numberTx, (numberRx == 0));
  }
  if (numberRx > 0) {
    return i2cReceive(addr, bufferRx, numberRx, true);
  }
  return HwReqAck::HwReqAckError;  // catchall
}

// Transfers the given buffers to/from the given peripheral through the SPI via DMA
//
HwReqAck i2cReceive(const uint8_t addr, uint8_t *bufferRx, const size_t numberRx, const bool autoEndXfer) {
  if (_i2cState != I2cState::I2cIdle) {
    if (_i2cBusyFailCount++ > cI2cMaxFailBusyCount) {
      _i2cRecover();
    }
    // Let the caller know it was busy if so
    return HwReqAck::HwReqAckBusy;
  }

  _i2cBusyFailCount = 0;

  if (numberRx > 0) {
    _i2cState = I2cState::I2cBusy;
    i2c_set_7bit_address(I2C1, addr);
    i2c_set_read_transfer_dir(I2C1);
    i2c_set_bytes_to_transfer(I2C1, numberRx);

    // Reset DMA channel
    dma_channel_reset(DMA1, cI2c1RxDmaChannel);

    // Set up rx dma, note it has higher priority to avoid overrun
    dma_set_peripheral_address(DMA1, cI2c1RxDmaChannel, (uint32_t) &I2C1_RXDR);
    dma_set_memory_address(DMA1, cI2c1RxDmaChannel, (uint32_t) bufferRx);
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

    if (autoEndXfer == true) {
      /* important to do it afterwards to do a proper repeated start! */
      i2c_enable_autoend(I2C1);
    } else {
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
HwReqAck i2cTransmit(const uint8_t addr, const uint8_t *bufferTx, const size_t numberTx, const bool autoEndXfer) {
  if (_i2cState != I2cState::I2cIdle) {
    if (_i2cBusyFailCount++ > cI2cMaxFailBusyCount) {
      _i2cRecover();
    }
    // Let the caller know it was busy if so
    return HwReqAck::HwReqAckBusy;
  }

  _i2cBusyFailCount = 0;

  if (numberTx > 0) {
    _i2cState = I2cState::I2cBusy;
    /* Setting transfer properties */
    i2c_set_7bit_address(I2C1, addr);
    i2c_set_write_transfer_dir(I2C1);
    i2c_set_bytes_to_transfer(I2C1, numberTx);

    // Reset DMA channel
    dma_channel_reset(DMA1, cI2c1TxDmaChannel);

    // Set up tx dma
    dma_set_peripheral_address(DMA1, cI2c1TxDmaChannel, (uint32_t) &I2C1_TXDR);
    dma_set_memory_address(DMA1, cI2c1TxDmaChannel, (uint32_t) bufferTx);
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

    if (autoEndXfer == true) {
      i2c_enable_autoend(I2C1);
    } else {
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

void i2cAbort() { _i2cRecover(); }

bool i2cIsBusy() { return (_i2cState != I2cState::I2cIdle); }

SpiMaster *getSpiMaster() { return &_spi1Master; }

Usart *getUsart(const uint8_t usart) {
  if (usart < cNumberOfUsarts) {
    return &_usart[usart];
  }
  return nullptr;
}

void setBlueLed(const uint32_t intensity) {
  if (intensity <= RgbLed::cLedMaxIntensity) {
    timer_set_oc_value(cLedPwmTimer, cLed2PwmOc, intensity);
  } else {
    timer_set_oc_value(cLedPwmTimer, cLed2PwmOc, RgbLed::cLedMaxIntensity);
  }
}

void setGreenLed(const uint32_t intensity) {
  if (intensity <= RgbLed::cLedMaxIntensity) {
    timer_set_oc_value(cLedPwmTimer, cLed1PwmOc, intensity);
  } else {
    timer_set_oc_value(cLedPwmTimer, cLed1PwmOc, RgbLed::cLedMaxIntensity);
  }
}

void setRedLed(const uint32_t intensity) {
  if (intensity <= RgbLed::cLedMaxIntensity) {
    timer_set_oc_value(cLedPwmTimer, cLed0PwmOc, intensity);
  } else {
    timer_set_oc_value(cLedPwmTimer, cLed0PwmOc, RgbLed::cLedMaxIntensity);
  }
}

void setStatusLed(const RgbLed &led) {
  setBlueLed(led.getBlue());
  setGreenLed(led.getGreen());
  setRedLed(led.getRed());
}

void delay(const uint32_t length) {
  _delayCounter = 0;
  while (_delayCounter < length) {
  }
}

uint32_t bcdToUint32(uint32_t bcdValue) {
  uint32_t multiplier = 1, result = 0;

  while (bcdValue != 0) {
    result += multiplier * (bcdValue & 0xf);
    multiplier *= 10;
    bcdValue = bcdValue >> 4;
  }
  return result;
}

uint32_t uint32ToBcd(uint32_t uint32Value) {
  uint32_t result = 0;
  uint8_t shift = 0;

  if (uint32Value > 99999999) {
    uint32Value = 0;
  }

  while (uint32Value != 0) {
    result += (uint32Value % 10) << shift;
    uint32Value = uint32Value / 10;
    shift += 4;
  }
  return result;
}

void dmaCh1Isr() {}

void dmaCh2to3Isr() {
  // Handle each SPI DMA channel individually as its transfer completes
  if (DMA1_ISR & DMA_ISR_TCIF2) {
    DMA1_IFCR |= DMA_IFCR_CTCIF2;

    dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL2);

    spi_disable_rx_dma(SPI1);

    dma_disable_channel(DMA1, DMA_CHANNEL2);
  }

  if (DMA1_ISR & DMA_ISR_TCIF3) {
    DMA1_IFCR |= DMA_IFCR_CTCIF3;

    dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL3);

    spi_disable_tx_dma(SPI1);

    dma_disable_channel(DMA1, DMA_CHANNEL3);
  }

  // Only process SPI completion when BOTH DMA channels are idle.
  // TX DMA (ch3) typically completes before RX DMA (ch2), causing split
  // ISR invocations. Without this guard, dmaComplete() would run when
  // only one channel has finished, potentially processing the transfer
  // prematurely and corrupting a subsequent chained transfer.
  if (!(DMA_CCR(DMA1, DMA_CHANNEL2) & DMA_CCR_EN) && !(DMA_CCR(DMA1, DMA_CHANNEL3) & DMA_CCR_EN)) {
    _spi1Master.dmaComplete();
  }
}

void dmaCh4to7Isr() {
  if (DMA1_ISR & DMA_ISR_TCIF4) {
    DMA1_IFCR |= DMA_IFCR_CTCIF4;

    dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL4);

    // Use compile-time dispatch based on DMA channel assignment
    if constexpr (cUsart1TxDmaChannel == DMA_CHANNEL4) {
      usart_disable_tx_dma(USART1);
    } else if constexpr (cUsart2TxDmaChannel == DMA_CHANNEL4) {
      usart_disable_tx_dma(USART2);
    }

    dma_disable_channel(DMA1, DMA_CHANNEL4);
  }

  if (DMA1_ISR & DMA_ISR_TCIF5) {
    DMA1_IFCR |= DMA_IFCR_CTCIF5;

    dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL5);

    // Use compile-time dispatch based on DMA channel assignment
    if constexpr (cUsart1RxDmaChannel == DMA_CHANNEL5) {
      usart_disable_rx_dma(USART1);
    } else if constexpr (cUsart2RxDmaChannel == DMA_CHANNEL5) {
      usart_disable_rx_dma(USART2);
      Dmx512Rx::rxCompleteIsr();
    }

    dma_disable_channel(DMA1, DMA_CHANNEL5);
  }

  if (DMA1_ISR & DMA_ISR_TCIF6) {
    DMA1_IFCR |= DMA_IFCR_CTCIF6;

    dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL6);

    // Use compile-time dispatch based on DMA channel assignment
    if constexpr (cUsart2RxDmaChannel == DMA_CHANNEL6) {
      usart_disable_rx_dma(USART2);
      Dmx512Rx::rxCompleteIsr();
    } else if constexpr (cI2c1TxDmaChannel == DMA_CHANNEL6) {
      i2c_disable_txdma(I2C1);
      _i2cState = I2cState::I2cIdle;
      if (_i2cNumberRx > 0) {
        i2cReceive(_i2cAddr, _i2cBufferRx, _i2cNumberRx, true);
      }
    }

    dma_disable_channel(DMA1, DMA_CHANNEL6);
  }

  if (DMA1_ISR & DMA_ISR_TCIF7) {
    DMA1_IFCR |= DMA_IFCR_CTCIF7;

    dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL7);

    // Use compile-time dispatch based on DMA channel assignment
    if constexpr (cUsart2TxDmaChannel == DMA_CHANNEL7) {
      usart_disable_tx_dma(USART2);
    } else if constexpr (cI2c1RxDmaChannel == DMA_CHANNEL7) {
      i2c_disable_rxdma(I2C1);
      _i2cState = I2cState::I2cIdle;
      _i2cBufferRx = nullptr;
      _i2cNumberRx = 0;
    }

    dma_disable_channel(DMA1, DMA_CHANNEL7);
  }
}

void exti415Isr() {
  if (exti_get_flag_status(EXTI13) != false) {
    // Delay RTC read to allow external RTC to update its registers after PPS edge
    // This prevents reading stale values when PPS fires
    _ppsDelayCounter = cPpsRtcReadDelay;

    _incrementOnTimeSecondsCounter();

    _peripheralRefreshTrigger = PeripheralRefreshTrigger::PpsExti;

    exti_reset_request(EXTI13);
  }

  if (exti_get_flag_status(EXTI15) != false) {
    InfraredRemote::tick();

    exti_reset_request(EXTI15);
  }
}

void systickIsr() {
  _delayCounter++;  // used by delay()

  // Handle PPS delay countdown - when it reaches 1, trigger the RTC refresh
  if (_ppsDelayCounter > 0) {
    _ppsDelayCounter--;
    if (_ppsDelayCounter == 0) {
      _refreshRTCNow = true;
      _refreshTempNow = true;
    }
  }
  // we'll do this if PPS isn't working
  else if (_peripheralRefreshTrigger == PeripheralRefreshTrigger::SysTick) {
    _refreshRTCNow = true;
    _refreshTempNow = true;

    _incrementOnTimeSecondsCounter();
  }
  if (_refreshRTCNow || _rtcReadPending || _refreshTempNow || _tempReadPending) {
    _refreshPeripherals();
  }

  // Light sensor sampling (500ms interval = 2 Hz)
  if (_adcSampleTimerLight++ >= cAdcSampleIntervalLight) {
    _adcSampleTimerLight = 0;

    // IIR filter with upscaling for precision: filtered = filtered - (filtered >> shift) + (sample << (upscale -
    // shift)) The upscaling maintains precision in integer math (avoids losing LSBs)
    _adcFilteredLight = _adcFilteredLight - (_adcFilteredLight >> cAdcLightFilterShift) +
                        (((uint32_t) _bufferADC[cAdcPhototransistor] << cAdcFilterUpscale) >> cAdcLightFilterShift);

    // Update cached light level (downscale back to actual ADC range)
    _cachedLightLevel = _adcFilteredLight >> cAdcFilterUpscale;

    _adcValuesUpdated = true;
  }

  // Temperature and voltage sampling (2000ms interval = 0.5 Hz - they change very slowly)
  if (_adcSampleTimerSlow++ >= cAdcSampleIntervalSlow) {
    _adcSampleTimerSlow = 0;

    // IIR filter for temperature (with upscaling)
    _adcFilteredTemp = _adcFilteredTemp - (_adcFilteredTemp >> cAdcTempVoltageFilterShift) +
                       (((uint32_t) _bufferADC[cAdcTemperature] << cAdcFilterUpscale) >> cAdcTempVoltageFilterShift);

    // IIR filter for VddA voltage (with upscaling)
    _adcFilteredVoltageVddA = _adcFilteredVoltageVddA - (_adcFilteredVoltageVddA >> cAdcTempVoltageFilterShift) +
                              (((uint32_t) _bufferADC[cAdcVref] << cAdcFilterUpscale) >> cAdcTempVoltageFilterShift);

    // IIR filter for battery voltage (with upscaling)
    _adcFilteredVoltageBatt = _adcFilteredVoltageBatt - (_adcFilteredVoltageBatt >> cAdcTempVoltageFilterShift) +
                              (((uint32_t) _bufferADC[cAdcVbat] << cAdcFilterUpscale) >> cAdcTempVoltageFilterShift);

    _adcValuesUpdated = true;
  }

  // update the tone timer and turn off the tone if the timer has expired
  if (_toneTimer > 0) {
    if (--_toneTimer == 0) {
      if (_toneTimerNext > 0) {
        tone(_toneFrequencyNext, _toneTimerNext);
        _toneFrequencyNext = 0;
        _toneTimerNext = 0;
      } else {
        timer_set_oc_value(cBuzzerTimer, cBuzzerTimerOC, 0);
      }
    }
  }

  if (++_batteryMeasuringCounter >= cBatteryMeasuringTimeout) {
    _batteryMeasuringCounter = 0;
    adc_disable_vbat_sensor();
  }
}

void tim2Isr() {
  TIM2_SR &= ~TIM_SR_UIF;  // Always clear flag; we shouldn't be here if the flag isn't set
}

void tim7Isr() {
  TIM7_SR &= ~TIM_SR_UIF;  // Always clear flag; we shouldn't be here if the flag isn't set
}

void tim15Isr() {}

void tim16Isr() {}

void tscIsr() {
  // process acquisition, advance to next channels, and trigger another acquisition
  if (TSC_ISR & TSC_ISR_EOAF) {
    // Read hardware counter values from both groups
    uint16_t rawValue[2];
    rawValue[1] = TSC_IOGxCR(6);  // Group 6
    rawValue[0] = TSC_IOGxCR(3);  // Group 3

    // Process both channels acquired in this cycle (one from each group)
    for (uint8_t groupIdx = 0; groupIdx < 2; groupIdx++) {
      uint8_t channelIdx = _tscChannelCounter + (groupIdx * cTscChannelsPerGroup);

      if (channelIdx >= cTscChannelCount) {
        continue;
      }

      uint16_t raw = rawValue[groupIdx];

      // Step 1: IIR Low-Pass Filter (exponentially weighted moving average)
      // filtered = filtered - (filtered >> shift) + (raw >> shift)
      // This implements: filtered = (filtered * (N-1) + raw) / N where N = 2^shift
      _tscFiltered[channelIdx] =
          _tscFiltered[channelIdx] - (_tscFiltered[channelIdx] >> cTscFilterShift) + (raw >> cTscFilterShift);

      // Step 2: Calculate delta from baseline
      // Touch causes capacitance increase which decreases the counter value
      // So: delta = baseline - filtered (positive delta = touch detected)
      int16_t delta = (int16_t) _tscBaseline[channelIdx] - (int16_t) _tscFiltered[channelIdx];

      // Step 3: Touch detection with hysteresis
      bool isTouched = (_tscTouchState & (1 << channelIdx)) != 0;

      if (!isTouched && delta > cTscTouchThreshold) {
        // Touch detected - set the touch state bit
        _tscTouchState |= (1 << channelIdx);
        _buttonStates |= (1 << channelIdx);
      } else if (isTouched && delta < cTscReleaseThreshold) {
        // Touch released - clear the touch state bit
        _tscTouchState &= ~(1 << channelIdx);
        _buttonStates &= ~(1 << channelIdx);
      }

      // Step 4: Adaptive baseline tracking (only update when not touched)
      // This allows the baseline to track environmental changes (temperature, humidity)
      // but prevents it from tracking sustained touches
      if (!isTouched) {
        // Asymmetric tracking: slow rise, faster fall
        if (_tscFiltered[channelIdx] > _tscBaseline[channelIdx]) {
          // Baseline rises slowly (tracks long-term drift upward)
          uint16_t rise = (_tscFiltered[channelIdx] - _tscBaseline[channelIdx]) >> cTscBaselineShift;
          _tscBaseline[channelIdx] += rise;
        } else {
          // Baseline falls faster (quick recovery after finger lift)
          uint16_t fall = (_tscBaseline[channelIdx] - _tscFiltered[channelIdx]) >> (cTscBaselineShift - 2);
          _tscBaseline[channelIdx] -= fall;
        }
      }
    }

    // Advance to next channel pair
    if (++_tscChannelCounter >= cTscChannelsPerGroup) {
      _tscChannelCounter = 0;
    }

    // Select the next pair of channels to sample
    TSC_IOCCR = cTscChannelControlBits[_tscChannelCounter];

    // Clear end-of-acquisition flag
    TSC_ICR = TSC_ICR_EOAIC;

    // Kick off the next acquisition cycle
    TSC_CR |= TSC_CR_START;
  }

  // Deal with max count error if it happened
  if (TSC_ISR & TSC_ISR_MCEF) {
    TSC_ICR = TSC_ICR_MCEIC;
  }
}

void usart1Isr() {
  // brute-force approace to USART errors here... :/
  if (_usart[0].hasErrors() || _usart[0].rxReady() || (USART_ISR(_usart[0].peripheral()) & USART_ISR_ABRE)) {
    _usart[0].clearErrors();
    // Clear the RXNE and ABRE flags -- they may be set with the flags above
    USART_RQR(_usart[0].peripheral()) = USART_RQR_ABKRQ | USART_RQR_RXFRQ;
  }
}

void usart2Isr() {
  if (_usart[1].hasErrors() || _usart[1].rxReady()) {
    _usart[1].clearErrors();
  }
}

void usart3_4Isr() {
  // Clear error flags on USART3 (serial remote RX)
  // Also flush RXNE if still set (safety net: SerialRemote::rxIsr normally
  // reads the byte, but if it hasn't been initialized yet, RXNE would remain
  // asserted and the ISR would loop indefinitely via Cortex-M0 tail-chaining)
  if (_usart[2].hasErrors() || _usart[2].rxReady()) {
    _usart[2].clearErrors();
    USART_RQR(_usart[2].peripheral()) = USART_RQR_RXFRQ;
  }
  // Clear error flags on USART4 (serial remote TX)
  if (_usart[3].hasErrors()) {
    _usart[3].clearErrors();
  }
}

}  // namespace kbxTubeClock::Hardware
