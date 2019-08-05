# Tube Clock Logic Board GPS Build Guide

## Why GPS on a clock?

While explaining how GPS works is a bit outside of the scope of this document,
 it should suffice to say that, due to how it works, the GPS is an extremely
 accurate source of time. See this
 [brief article](http://www.physics.org/article-questions.asp?id=55) for a bit
 more detail. If nothing else, it allows your clock to set itself!

## Build Options

The tube clock logic board and application have the ability to decode NMEA
 messages sent into the USART1 serial interface. There are three possible ways
 to accomplish this:
1. Connect an external GPS module/device to the six-pin serial interface. Note
   that the USART1 receive line on the STM32's PA10 pin is a 5-volt tolerant
   pin. Therefore, the connected GPS device may output a 5-volt maximum signal.
   **Do not connect a device that outputs an RS-232-level signal!**
1. Connect the [Adafruit Ultimate GPS Module](https://www.adafruit.com/product/746)
   to the pin header on `J7`. This is arguably the easiest path.
1. Install the Teseo LIV3F module on the `U10` footprint.

## Which GPS option is right for me?

For most builders, the determining factor will be their ability to work with
 extremely tiny parts (the ones you need a microscope for). If this isn't for
 you, the [Adafruit Ultimate GPS Module](https://www.adafruit.com/product/746)
 is likely the best approach.

For builders with appropriate tooling, experience, and willingness, (or perhaps
 you're feeling a little crazy and just want to try your hand at these super
 small parts) the on-board LIV3F module is a less expensive approach. There are
 two possible build configurations when using the LIV3F module:
1. Active antenna only: this requires four 0402-sized surface mount parts to be
   soldered onto the underside of the logic board -- one capacitor, one
   inductor, and two zero-ohm jumpers. As these are not polarity-sensitive
   parts, placing them and soldering them is a bit easier. Still, the process is
   more tedious than it is with the larger parts on the top side of the board.
1. Active or passive antenna support: this requires a full suite of components
   on the underside of the board, including a LNA and a SAW filter. The LNA and
   SAW filter are more challenging to place as they have multiple pins and must
   be oriented correctly. A microscope of some sort will be required to identify
   the pin-1 marking on the LNA. Ideally testing equipment is also available to
   confirm its operation after soldering, although continuity testing is likely
   sufficient.

## What parts are necessary on the BoM for my chosen GPS configuration?

### For the Adafruit Ultimate GPS module

None of the parts flagged as "GPS" on the BoM are necessary. Only the pin header
 for `J7` is required so that the module may be plugged in. These parts are:
 `J8`, `C33` through `C43`, `L2`, `L3`, `U9`, `U10`, `U11`, `FL1`, and `R30`
 through `R33`. Do not install these if using this module.

### For the LIV3F module with active antenna support only

Only the following parts are required: `J8`, `C34`, `C42`, `C43`, `L2`, `U10`,
 `U11`, and `R30` through `R33`.

### For the LIV3F module with active or passive antenna support

The following parts are all required: `J8`, `C33` through `C43`, `L2`, `L3`,
 `U9`, `U10`, `U11`, `FL1`, and `R30` through `R33`.

## Anything else I should know?

A word of caution for those who wish to use the LIV3F module with active and
 passive antenna support: The LNA and SAW filter have pins that are *under* the
 device (somewhat similar to a BGA-style package). They simply cannot be
 soldered with a soldering pencil. Soldering them will require either a hot-air
 soldering station or a reflow oven.

While the tiny parts are challenging to work with, it's quite satisfying to see
 them operating after completing the build. They're so tiny! The end result is
 quite pleasing and worth the effort, if you dare!
