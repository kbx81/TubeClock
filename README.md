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

That said, the world probably doesn't need another tube clock. There are
 plenty of them--all different shapes and sizes--so why build another one?

Last year, I built this lovely
 [binary clock](https://github.com/kbx81/RGBBinaryClock). Some ten-plus years
 ago I had built some IN-18 nixie clocks. Earlier this year (2019) one of them
 became a little troublesome. I fixed it but, in the interim, became motivated
 to build my own. As I already had a wonderful foundation for some clock
 hardware, this seemed a natural progression. Overall I'm quite happy with the
 results if I may say so myself. :)

_Sooo...another nixie clock? What's special about it?_

First, I brought the touchkey design over from the binary clock. That seemed
 like a no-brainer. Still, it needed more...flair. Soooo...let's get the time
 from GPS -- either a soldered-on LIV3F module or a more friendly
 [module](https://www.adafruit.com/product/746) from
 [Adafruit](https://www.adafruit.com). How about an infrared remote control so
 you can turn off the display from across the room while you're watching a
 movie? Or rather, if you don't want to do that, it'll still do that cool
 display-dimming thing that I did with the
 [binary clock](https://github.com/kbx81/RGBBinaryClock). These tubes can be
 pretty bright in a dark room. :)

Also, just like the binary clock, it can do more than just tell the time --
 it'll tell you the date and temperature, too! There is even a timer/counter
 mode. What's more, you can choose the format for it all: the clock can display
 in a 12 or 24 hour format, the temperature can display in degrees Celsius or
 degrees Fahrenheit, and (most importantly) you are able to choose the formats
 you prefer and they can be changed easily at any time.

As mentioned above, it has a phototransistor which is used to determine the
 amount of ambient light around it and the display will dim smoothly as the
 light level around it diminishes. This is great if you want to keep it near you
 at night while you sleep.

A CR2032 coin-cell battery backup can be installed to keep the time valid in
 the event that the board loses power; there is also a super capacitor,
 eliminating the need for the battery; even so, both can be installed, adding
 flexibility to the build.

Finally--and one could argue that no clock is complete without one--it has an
 alarm! The alarm can be set to beep at any of the eight times the user sets.
 There is also an hourly chime that one can enable which will beep out each hour
 in binary using high and/or low pitch tones...so you can hear the time when
 you're in another room! The display can be configured to blink when an alarm
 occurs. Finally, unused microcontroller pins from the STM32 are brought out to
 a pin header, some pins of which can be used as inputs to trigger the alarm
 from an external device -- or connect other hardware of your own!

## Great, but what makes it tick?

The "brain" is an STM32F072 microcontroller. This MCU alone has everything
 that's necessary to have a functional time clock -- even a temperature sensor
 as a bonus. Still, it might not be quite as accurate as some of us would like.
 For those folks, there are footprints for some additional ICs to improve the
 accuracy of the time and/or temperature sensing:
* A Maxim DS3234
* A Maxim DS1722
* A LM74
* A LIV3F GPS module (and associated antenna front end with several 0402 parts!)
* A header for an Adafruit [module](https://www.adafruit.com/product/746)

Why footprints for both temperature sensors and the RTCs? The DS3234 is somewhat
 expensive and it's possible that one might want more accurate temperature
 sensing abilities but isn't as concerned with the accuracy of the clock. It
 should be noted that the DS3234 devices have temperature sensors built in and
 the application will use this sensor if a DS3234 is installed but one of the
 other external temperature sensors is not.

Beyond the MCU itself, the display boards have up to three (depending on the
 tubes/board) HV5622 high-voltage driver ICs on them to light up the beautiful
 tubes. The MCU uses its SPI1 to communicate with these drivers. There is a
 single RGB LED is used as a "status" LED and it is connected (through FET
 drivers) to GPIO pins on the MCU. These pins double as timer output channels,
 meaning they can also generate a PWM signal, enabling the dimming of the status
 LED elements, as well.

The beeper is connected (also through a FET driver) to yet another GPIO pin
 that doubles as a timer output channel; this enables the beeper to generate a
 wide range of tones or even play a tune!

The phototransistor is connected to the MCU's ADC channel ten.

Two USARTs are exposed via pin headers on the right side of the board: USART1
 is brought out on a standard six pin header as is commonly found on many
 devices; it is also connected to the optional GPS module (solder jumpers allow
 easy disconnection of the module should it be necessary for troubleshooting).
 USART2 is connected to an RS-485 line driver enabling communication on an
 RS-485 bus. Through this interface, the application is able to receive a
 DMX-512 signal so the tubes can each be individually controlled from an
 entertainment lighting console (or other application that speaks this
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
