# kbx's Tube Clock

Tube Clock firmware source files

## What's going on here?

Here you will find the C++ source code files for the Tube Clock. It is
 modularized so that its various parts can be broken out and potentially re-used
 for other projects. It's not perfect but it works well and the plan is to
 continue to update and improve it over time.

The firmware implements all of the clock's functionality from the simple display
 of the time and date through animation selection and management, alarms, and
 even DMX-512 control for the display tubes. It could be easily modified to
 provide a variety of other capabilities if desired.

## More specifics

The code is written for the STM32F072 microcontroller used on the Tube
 clock. With a little effort, it should be relatively easy to port it to another
 STM32Fx-series MCU. It relies on [libopencm3](http://libopencm3.org) as the
 underlying platform to interface with the MCU. If you wish to do some hacking
 with this platform, a nice guide can be found
 [here](https://github.com/libopencm3/libopencm3-examples#reuse) which explains
 how to clone libopencm3 for (re)use within your own (or this) project. Note
 that if you do so, it may be necessary to tweak some paths in the Makefile(s),
 but otherwise things should work without much trouble. You'll also (of course)
 need a (gcc) compiler to build the code base. (At the time of this writing,
 I've been building it with gcc on macOS 10.14 aka Mojave.)

The STM32F072 microcontroller is a rich device with a lot of peripherals. Many
 of these peripherals are leveraged in the code to enable it work as efficiently
 as possible. For posterity, here is a list of the peripherals used:

* ADC - the analog to digital converter, used for sensing light level from the
  attached phototransistor
* CRC - used to compute a checksum for settings data stored in FLASH
* DMA - direct memory access controller, used to speedily move data to/from
  other I/O peripherals such as I2C, SPI, and USARTs
* FLASH controller - erases and writes settings to a dedicated block of FLASH so
  settings are retained across power failures and do not depend on the battery
* GPIO - because how else do you toggle pins on/off or read input signals?
* IWDG - the watchdog timer makes sure the MCU gets reset in the event of some
  catastrophic software crash/failure
* NVIC - none of this would be possible without interrupts.
* RTC - the real time clock is used if the DS323x is not installed. Since it's a
  clock, you need a clock...sooooo...
* SPI - this is the interface (via DMA) to the tube driver ICs. It is also the
  interface to the DS3234 and other temperature sensors.
* Timers - Timers 1 and 3 are used to generate PWM for the buzzer and for the
  status LEDs, respectively. Timers 15 and 16 are used to supervise DMX-512
  communication. Timer 7 times pulses from the IR receiver and timer 2 generates
  an interrupt (at 5 KHz) to trigger a refresh of the tubes, emulating PWM and
  thus enabling the digit dimming/fading effect.
* TSC - the touch sensing controller is used to sense touches on the touch keys.
* USARTs - USART1 is a general serial interface and is used to communicate with
  an attached GPS module. USART2 is connected to an RS-485 transceiver. This
  firmware enables a DMX-512 signal to be received on USART2 so that the tubes
  can be individually controlled remotely.

## Important!
When compiling (with gcc), you'll need to add `-DHARDWARE_VERSION=X` to the
 command line, where `X` is the major version number of the hardware (PCB)
 you're targeting. At the time of this writing, there is only one major hardware
 version. Note that this switch has already been added to the included
 Makefile. Please be aware of this if you choose to build it yourself and/or do
 some hacking!

## I want to hack. Where do I start?

`TubeClock.cpp` is where it all begins. This connects the various
 interrupts to the appropriate functions for processing them, initializes the
 hardware, initializes the main application, then passes control to the main
 application loop. See `Application.cpp` and `Hardware.cpp` and work your way
 through the rest from there.

To build the code, there is a basic Makefile in the `src/v1` directory; simply
run `make` and it'll build what you need. Then, you can write the resulting
binary to the STM32 via an ST-Link with:

```shell
st-flash write TubeClock.bin 0x08000000
```

For more advanced debugging, you can use GDB. Run `st-util --n` followed by
`arm-none-eabi-gdb TubeClock.elf --init-eval-command="target remote 127.0.0.1:4242"`
to connect.

Please don't hesitate to reach out with any questions or comments -- it is nice
 to hear feedback. Happy hacking!
