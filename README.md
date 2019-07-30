# kbx's Nixie Clock

Nixie Clock project files - PCB and source

## Another nixie tube clock? Why?

I have to start by giving credit where credit is due. The
 [Boldport Club](http://boldport.club), over the last couple of years, has
 motivated me to dwell back into the world of electronics, at least as a
 hobbyist. Both the kits themselves and the other members in the club are
 amazing and helped inspire me to make something...so I began working on this
 project. Some inspiration is taken from
 [Touchy, BPC Project #7](https://www.boldport.com/products/touchy); this kit
 enabled me to learn more about and actually experiment with capacitive touch.
 It was clear early on that this would be a nice touch (pun intended) to add to
 this project; visually, it is cleaner, it reduces the number of parts and, as a
 result, it reduces the cost.

That said, the world probably doesn't need another (binary) clock. There are
 plenty of them--all different shapes and sizes--so why build another one?

This project was started with two main goals in mind: I wanted a "simple"
 project I could use to help me get back into the world of electronics. The
 second goal I had in mind was to build up my comfort level with C and C++ -- I
 knew the language(s) but was never terribly comfortable with them. Building
 this helped me to achieve both of those goals and then some -- and I am quite
 happy with the result!

_But why a clock?_

It's fun to make LEDs blink. It's fun to play with microcontrollers. It's fun
 to build stuff. But a blinking LED or two or three isn't very meaningful. Nor
 is a nice microcontroller that just toggles a GPIO pin. If you're going to
 build something, why not make it at least modestly useful? And why not build it
 in a unique way that's neat to look at and/or watch?

The idea for this came from an old electronics kit I put together probably
 around twenty or so years ago (and it still works today!). I liked the kit but
 it was somehow lacking. Probably the most obvious problem was that, in a dark
 bedroom, the display was too bright. It was a simple, 12-hour-only binary
 clock, built on 4000-series CMOS logic. Now, there's nothing wrong with that,
 but...it's 2018 now. Let's take it up a notch or ten.

## How do you take a binary clock up a notch (or ten)?

First, you can add some color. A lot of color. In fact, you can pick the colors
 you want it to use...up to eight times over. This binary clock has the ability
 to gradually shift colors over the course of each day. Bits can even fade in
 and out as they change. It's all about the eye candy in this regard.

Second, it can do more than just tell the time -- it'll tell you the date and
 temperature, too! There is even a timer/counter mode. What's more, you can
 choose the format for it all: the clock can display in a 12 or 24 hour format,
 the temperature can display in degrees Celsius or degrees Fahrenheit, and (most
 importantly) it can display all of these values in either binary-coded decimal
 (BCD) or good old-fashioned binary. You get to choose the formats you prefer
 and they can be changed easily at any time.

Next, the display brightness issue had to be addressed -- it has a
 phototransistor which is used to determine the amount of ambient light around
 it and the display will dim as the light level around it diminishes. This is
 great if you want to keep it near you at night while you sleep.

Another cool bit is that it uses capacitive sense touch keys for buttons; the
 kit it is based on had mechanical, fixed-function buttons. If you haven't seen
 or used capsense technology before, you'll find it's pretty cool and adds a
 little extra uniqueness to the clock, too.

A CR2032 coin-cell battery back-up can be installed to keep the time valid in
 the event that the board loses power. Version 4 of the hardware also includes a
 super capacitor, eliminating the need for the battery; even so, both can be
 installed, adding flexibility to the build.

Finally--and one could argue that no clock is complete without one--it has an
 alarm! The alarm can be set to beep at any of the eight times the user sets.
 There is also an hourly chime that one can enable which will beep out each hour
 in binary using high and/or low pitch tones...so you can hear the time when
 you're in another room! The display can be configured to blink when an alarm
 occurs. Version 4 of the clock also includes extra pin headers, some pins of
 which can be used as inputs to trigger the alarm from an external device.

## Great, but what makes it tick?

The "brain" is an STM32F072 microcontroller. This MCU alone has everything
 that's necessary to have a functional time clock -- even a temperature sensor
 as a bonus. Still, it might not be quite as accurate as some of us would like.
 For those folks, there are footprints for some additional ICs to improve the
 accuracy of the time and/or temperature sensing.

Version 2 boards have footprints for I2C devices:
* A Maxim DS3231
* An LM75 (or compatible)
* A Microchip MCP9808

Version 3+ boards have footprints for SPI devices:
* A Maxim DS3234
* An LM74
* A Maxim DS1722

Why footprints for both temperature sensors and the RTCs? The DS323x is somewhat
 expensive and it's possible that one might want more accurate temperature
 sensing abilities but isn't as concerned with the accuracy of the clock. It
 should be noted that the DS323x devices have temperature sensors built in and
 the application will use this sensor if a DS323x is installed but one of the
 other external temperature sensors is not.

Beyond the MCU itself, the board has 25 RGB LEDs on it; 24 of them form the main
 display and they are connected to TLC5947 (pre-v4) or TLC5951 (v4+)
 constant-current PWM drivers. The MCU uses its SPI1 to communicate with these
 drivers. The remaining RGB LED is used as a "status" LED and it is connected
 (through FET drivers) to GPIO pins on the MCU. These pins double as timer
 output channels, meaning they can also generate a PWM signal, enabling the
 dimming of the status LED elements, as well.

The beeper is connected (also through a FET driver) to yet another GPIO pin
 that doubles as a timer output channel; this enables the beeper to generate a
 wide range of tones or even play a tune!

The phototransistor is connected to the MCU's ADC channel zero.

Two USARTs are exposed via pin headers on the right side of the board: USART1
 is brought out on a standard six pin header as is commonly found on many
 devices while USART2 is connected to an RS-485 line driver enabling
 communication on an RS-485 bus. Through this interface, the application is able
 to receive a DMX-512 signal so the LEDs can each be individually controlled
 from an entertainment lighting console (or other application that speaks this
 protocol), enabling another whole realm of possibilities...

## How do I get or build one?

In this repository you'll find everything needed to put one together. It is
 divided into two major parts: hardware and software (source). The `hardware`
 directory contains the [KiCad](http://kicad.org) project files used to create
 the printed circuit board. The `src` directory contains the source code needed
 to compile and run the application on the microcontroller. It is built on top
 of [libopencm3](http://libopencm3.org). Finally, then `bin` directory contains
 compiled binary files you may flash directly onto the microcontroller...great
 for folks who want to solder something together but don't want to be bothered
 with compiling code!

Additional details regarding the hardware and software can be found in the
 `README.md` files located in each respective directory.

 **That's all for now...thanks for visiting!**
