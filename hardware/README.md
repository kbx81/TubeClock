# kbx's Tube Clock

Tube Clock PCB project files for KiCad (v5)

## How do I get PCBs made for this?

Detailing the process is a bit outside of what I plan to discuss here. There are
 plenty of resources out around the web already that will do a better job of
 explaining this process than I can. That said, there's not much to it -- the
 files you need are here and all you need to do (more or less) is upload them
 to your PCB fabricator of choice and, after a few days, they'll ship them
 to you. I've used [Elecrow](http://elecrow.com) for the prototypes and I can
 assure you that they've turned out fantastic boards given the price.

As the vast majority of the components on this board are surface-mount parts,
 hot air is a fun approach for soldering it all together. If you go this route
 (and especially if you plan to build more than one of them), you'll likely want
 to have a stencil made to help with the placement of solder paste onto the
 board. I ordered a stencil with my board from [Elecrow](http://elecrow.com).

## If I get boards made, how do I build it?

All the components are listed in the KiCad project files here; one will also
 find bill of materials (BoM) files named `TubeClock.BoM.md` or `IN-1x.BoM.md`
 containing these lists. For "unusual" parts (things that aren't resistors or
 capacitors) I've noted a suggested part number that should serve as a point of
 reference to help you find your own.

Before you order parts, be sure to read through the build notes below!

Once you have your boards and parts, there are at least a couple of different
 ways you can build the clock base. Below are build notes.

## Build notes

### Before you begin...

...you need to determine:
1. What will your time source be -- the STM32's RTC, the DS3234, or a GPS
 module? The GPS will provide the best accuracy, followed by the DS3234. That
 said, these ICs will add approximately $10 to $20 USD to the cost of the
 device. If you're purchasing them, double-check those part numbers before
 ordering!
1. What temperature sensor will you use -- the STM32's internal sensor, the
 DS3234's sensor, or a dedicated sensor? The DS1722 or LM74 dedicated sensors
 will provide the best accuracy; however, if one of these ICs is not installed
 but a DS3234 *is* installed, the app will use the DS3234's sensor. Note that,
 while the PCBs have footprints for more than one temperature sensor, only
 **one** of them may be installed at a time. Do not install both!
1. Solder-bridge/jumper settings. These are explained below...but don't forget
 about these!

Once you've settled on the items above, read on (below) for more detail and
 specifics regarding the build.

### Build details

#### GPS:

The clock's application will accept and decode NMEA sentences from an attached
 GPS module. The PCBs will accept a plug-in module from
 [Adafruit](https://www.adafruit.com/product/746) or a Teseo-LIV3F module can
 be installed for a more permanent solution. Testing reveals that both of these
 modules work extremely well and will provide the most accurate time for the
 clock. Note that the LIV3F module will require some additional components to
 operate; several of these parts are 0402-sized (which means that they are very,
 very tiny!) and, as such, are challenging to work with. This approach is best
 for experienced builders with tooling available to work on such tiny parts.

#### External RTC:

The external RTC--the DS3234--adds a level of precision to the clock that is
 likely difficult to achieve with the STM32's internal RTC alone. These RTC ICs
 are designed to drift less than two minutes _per year_. It's possible that your
 STM32 hardware could end up being this accurate, but such accuracy is unlikely.
 Install this IC if you want a clock that is as accurate as is reasonably
 possible without depending on GPS. Note that this IC also has a built-in
 temperature sensor (that's part of how it maintains its precision) that the
 application will use if it is installed and the other external temperature
 sensor (below) is _not_ installed.

If you install a DS3234 on your clock, **do not install** these components:
 - `C23`
 - `R16`

#### External temperature sensors:
**U6 and U7**

A challenge with using the STM32's internal temperature sensor is that the
 sensor typically reads a few degrees higher than the temperature around the
 MCU itself. This is because the die, where the sensor is located, is usually
 just a little bit warmer then the package around it. An easy workaround is to
 just subtract a few degrees from the reading -- hence the calibration item
 you'll find in the application's menu structure. It works, but it requires
 someone to tweak it as each chip is likely to be a little different. Naturally,
 this takes away from its accuracy.

Installing an external temperature sensor is a great solution to this problem.
 The application will prefer an external sensor if one is installed; it first
 prefers one of the dedicated temperature sensors. If not installed, it will try
 to use the sensor integrated into the DS3234, should one be installed. If these
 all fail, it will use the STM32's internal sensor as a last resort.

**IMPORTANT NOTE: the PCBs (all hardware versions) have footprints for two
 external temperature sensor ICs. ONLY INSTALL ONE of them. DO NOT INSTALL
 BOTH.** If you install both, neither of them will work and you may cause damage
 to them or to other ICs on the board.

#### RS-485 transceiver:
**U8**

This part will enable the the second USART in the STM32 MCU to be used to
 communicate on an RS-485 bus. The original intention for this feature was to
 enable the device to receive a DMX-512 signal from an entertainment lighting
 console. With some development, it could also be used (for example) to
 synchronize a number of clocks with each other. This interface is more of a toy
 and has no impact on how the clock itself will function. You'll likely want to
 leave it off of the board unless you have a specific plan to use it for
 something.

#### Infrared (IR) receiver module:
**U4**

If the infrared remote control is not needed, this component may be left off of
 the board.

#### Buzzer/beeper components:
**BZ1, D6, Q6, R29**

If you don't want your clock to be able to make any noise at all, it is safe to
 leave these parts off of your board. It'll be sad, but it'll work just fine.

#### Status LED components:
**LED1, LED2, Q3 through Q5, R23 through R28**

The status LED is not typically used during normal operation, however it can be
 enabled as an AM/PM indicator if/when the clock is set in 12-hour mode. It can
 also provide troubleshooting information if your clock doesn't seem to be
 working. That said, it's not necessary unless you like LEDs as much as I do.

#### Reset/Boot buttons:

The reset and boot buttons are of course not required, but it is convenient if
 you plan to do any development with this platform on your own.

### Other requirements

#### 32.768 kHz crystal installation and connection:

A footprint for a 32.768 kHz crystal exists on the PCB. It is used to drive the
 STM32's internal RTC mechanism. This crystal is particularly important if the
 DS3234 IC is **not** installed; in this case, it must be connected to the MCU
 by creating a solder bridge across the appropriate two pads on `SB2`. Failure
 to do so--and if the DS3234 is not installed--means the clock won't run
 (although things otherwise may seem to work).

If the DS3234 **is** installed, `SB2` may be bridged so as to connect the
 DS323x's 32.768 kHz signal output to the STM32's OSC32 input. This is not
 required but it is recommended so that this MCU input isn't left floating. In
 addition, if `SB2` is bridged in this manner, the 32.768 kHz crystal (and its
 drive capacitors) should **not** be installed.

#### Phototransistor series resistors:
 **Q2, RV2, R7 and R10**

The phototransistor enables the application to determine the amount of ambient
 light around the clock so it can adjust the overall display intensity
 accordingly. There is a variable resistor in series with a fixed resistor which
 allows some manual adjustment over the sensitivity of this mechanism. There is
 also a single fixed resistor in parallel with this configuration. It is not
 necessary to install all of these, however it won't hurt anything if you do.
 If the specified phototransistor is used, a single 100K to 150K resistor
 installed for `R10` will probably work well.

#### Status LED:

If installing a status LED, install only LED1 **or** LED2. Do not install both.

### Solder Bridge Jumpers

#### Pulse-Per-Second (PPS) input selection
**SB1**

This jumper determines which peripheral will provide the pulse-per-second input
 to the processor (and application). This enables more precise refreshing of the
 display and allows the application to operate more efficiently. If neither of
 these peripherals is installed, this jumper is not used.

#### 32.768 kHz Oscillator jumper
**SB2**

This jumper determines if the 32.768 kHz oscillator or the DS3234's 32.768 kHz
 output pin is connected to the STM32's OSC32_IN pin. If a DS3234 is **not**
 installed on the board, it must be bridged so that the crystal's pin is
 connected to the OSC32_IN pin. Otherwise, it's best to bridge it to the
 DS3234's 32.768 kHz output pin. Labels on the PCB indicate which position is
 which.

#### USART 1/USB VBUS jumpers
**SB3, SB4**

These jumpers are on version 4+ boards and allow some configuration of which
 USART1 pin on the MCU is connected to the six-pin serial header. Because the
 processor expects USB VBUS on PA9, if you plan to use the USB port, SB4 should
 be bridged and SB3 should be bridged to put PB6 on the on the six-pin header.
 If you do not want/need to use the USB port and wish to flash the MCU via the
 six-pin header, SB4 must be left **open** and SB3 should be bridged to put PA9
 on the six-pin header.

#### Reset Supervisor jumper
**SB5**

 This jumper determines if the DS3234's reset supervisor output pin is connected
  to the STM32's NRST (not-reset) pin. This should be connected if the DS3234 is
  installed, however it sometimes causes issues with flashing the MCU. If you
  installed a DS3234 and are having difficulty flashing the MCU, try
  disconnecting this jumper until the flashing process is complete. Don't forget
  to reconnect it, though!

#### RS-485 Terminator jumper
**SB6**

Bridge this jumper to connect the terminating resistor across the RS-485
 transceiver's input/output pins. This is useful if the board is at the end of
 a RS-485 link.

#### GPS receive and transmit lines
**SB7, SB8**

Bridge these jumpers to connect the GPS module's receive (Rx) and transmit (Tx)
 lines to the STM32 processor's USART1 interface. These jumpers are provided as
 a convenient way to disconnect the serial data lines in the event that USART1
 must be used for programming or debugging after the board has been built,
 particularly with the LIV3F module.

## Before powering it on for the first time...

WAIT! you didn't plug it in yet, did you? Let's do some pre-flight checks to get
 ready to bring up your board!

It's important that we go over the board with a DMM (or at least a continuity
 tester) and check for any nasty shorts. Solder bridges and poor part placement
 are the typical causes of this and it's not uncommon to have them, particularly
 with so many surface-mount parts. We don't want our brand-new clock to let out
 the magic smoke before we even get to use it, do we? :)

Generally, it will suffice to check a few key areas for shorts. Since these are
 surface-mount ICs, it's not uncommon to have solder bridges between some of the
 pins after you've soldered it all together. Don't worry, we can get away
 without checking _all_ of them, but we _do_ need to check a few. Let's make a
 list so we can check them off:
* **Check for shorts between the output of the LDO and ground.** Put one of your
 DMM probes on the power connector's ground pin and the other on the LDO's
 heatsink (the top fin soldered to the board -- this is its 3.3-volt output). If
 you see they're shorted, you'll have to go hunting for the short. Start by
 checking the power pins around the STM32 processor. All of its Vcc and ground
 pins are right next to each other so this is the best place to look first.
* **Check for shorts on the display pin header.** Note that there are multiple
 ground pinns on this header so they will naturally appear to be shorted. It is
 important to check for shorts between the pins that supply the various voltages
 to the display board (3.3, 12, and 170 volts). Shorts between any of these pins
 will be disastrous!
* **Check that the solder bridge "jumpers" are set correctly.** You didn't
 forget to bridge these as appropriate, did you? :) See the "Jumpers" section
 above for information on how they should be set.

Once you've confirmed everything above looks good, it should be reasonably safe
 to proceed to the next step -- flashing the firmware onto the STM32 processor!
 After we do this, we'll be able to more effectively identify any other
 soldering-related issues that could be affecting the clock's operation.

After you power on your board for the first time--and if nothing blows
 up--you'll need to flash the firmware onto the STM32 MCU. The board has USB
 interface that may be used for this purpose, in addition to the SWD connection
 (P1) or the serial interface on J5. Either way, you'll need an appropriate
 adapter and/or cable to do so. You can find the source, compiled binary files
 and flashing instructions in the `src` folder one level up in the repo's
 hierarchy. With the right tools in hand, this can be done in a couple of
 minutes.

If you're having trouble flashing the MCU, please see the next section.

## Troubleshooting

What's that? You couldn't flash the MCU? Or maybe it isn't working perfectly
 after you flashed the MCU? Uh oh...

As I've built...a few...of these to date, here is a list of common issues I've
 stumbled across as well as some notes on how to fix them.

If you're having trouble flashing the MCU, check these items first:
* If a DS3234 is installed, is the reset supervisor jumper bridged? If so, this
 may be holding the STM32 in a reset state too long and your programmer is
 giving up. Open this jumper and try again.
* If there is no DS3234 installed and/or the jumper is already open, check the
 data lines between the port you're using (six-pin serial, USB, or SWD) and the
 processor. Perhaps there is another nasty solder bridge somewhere that's
 shorting one or more of the lines to Vcc, ground, or another adjacent pin. If
 you're using the USB interface on a version 4+ board, take a close look at D5
 and the physical USB port itself, as well.

If you flashed the MCU and it's now doing...something...but it doesn't seem
 right, here are some more common symptoms and how to fix them:

* **Status LED blinks orange/off but the main display is blank or flickers
 erratically.** This is indicative of some soldering problem on the SPI1 pins
 of the MCU or on one or more of the HV5622 drivers. Take a close look at these
 pins and rework them as necessary.
* **One or more of the touch keys don't work.** Check the 1K resistors that sit
 between the touch keys and the MCU as well as the 47 nF sense capacitors.
 Again, also check the relevant pins on the MCU itself for shorts to adjacent
 pins (this is the most common issue).
* **It keeps making a ticking sound.** There's a problem with one or more of the
 touch keys -- see the item above.
* **The status LED alternates red and yellow.** You did not install a DS3234 (or
 maybe you did and it's not working) and the STM32's RTC/LSE oscillator did not
 start within the expected amount of time. Check that `SB2` is bridged correctly
 and that the 32.768 kHz crystal and its bypass capacitors are installed and
 soldered correctly. Also check that the respective pins on the STM32 processor
 aren't shorted to adjacent pins (or to each other).

## Known Issues and Noteworthy Items

Heat from the boost converter (the part that generates the high voltage for the
 tubes) or GPS module can skew the temperature reading from the sensor on the
 board. This becomes more noticeable as the tubes become brighter. The PCBs
 include footprints for additional 2.54mm pin headers which could be used for
 mounting the temperature sensor(s) off of the main board.

## Legal stuff and License

The circuit schematics and PCBs found here are licensed under the
 [Creative Commons Attribution-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-sa/4.0/).

![Creative Commons License badge](https://i.creativecommons.org/l/by-sa/4.0/88x31.png)

_Happy building!_
