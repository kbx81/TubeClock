# kbx's Tube Clock

_So...I built one (or someone gave me one), loaded the firmware and...now what?_

Congratulations! Read on...

## Getting started with your Tube Clock

After confirming everything is working, using the clock is pretty simple! Here's
 how it works:

When powered on, the clock is either set or it is not set (such as if the
 battery/capacitor drained or this is the first time you've powered it on).
* If the clock is set, it will resume displaying the time (and date and
  temperature, if the "Default to Toggle mode" option is enabled. More on this
  later on).
* If the clock is not set, display animations will run continuously (so the
  digits will just keep changing).

### It's working! Now what?

If you've powered it on and are looking at continuously rotating digits, you'll
 first need to set the time and date. To do so, start by touching the `E` touch
 key. This will exit the time/date/temperature view and enter into the main menu
 view. Alternatively, if your clock has a GPS module installed or connected,
 just wait for the GPS device to get a fix and your clock will set itself!

#### The Main Menu view

You can identify the main menu view as you'll see the right-most two digits lit
 with a single dot to their right (the lower of the two left colon indicator
 lamps) with the selected menu item number displayed on the left two tubes.
 Press the `U` key to increase or the `D` key to decrease the selected
 menu item number as indicated by the LEDs. To make your selection, touch the
 `E` key again to enter into the selected mode/menu item. (Perhaps you are
 starting to build the connection that `E` means "Enter"/"Exit".)

#### Setting the time and date

If, when you started, the time and date were not set, entering the main menu
 view will automatically select option 5, the "set time" function. If option 5
 is not selected, use the `U` and/or `D` keys to select it. Touch the `E` key to
 enter into the set time/date view to set the time.

Once you've entered into mode 5, you'll see the set time/date view. This is the
 standard view used to set times and dates. In this view, the touch keys work as
 follows:

* `U` increases the selected value
* `D` decreases the selected value
* `C` moves the selection left
* `B` moves the selection right
* `A` applies the date/time as set
* `E` exits to the main menu

Use the keys to adjust the values to the correct time and touch the `A` key to
 commit the time shown on the display to the clock's hardware. When you're done,
 touch the `E` key to exit back to the main menu.

**Important:** You must touch the `A` key to save the time/date as set! If you
 do not, the selected values will be lost upon exiting the set time/date view.

Once the time has been set, the date must be set. Upon exiting mode 5 (set
 time), mode 6 will be automatically selected if the date has not been set
 (that is, if the year is equal to zero, which...yeah...). In any event, mode 6
 is the "set date" mode and, once it is selected in the main menu, touch the `E`
 key again to enter that mode to set the date.

Setting the date works exactly the same way as setting the time does in terms of
 the user interface. Note that, by default, the display shows **year**,
 **month**, **day** (see footnote below). Use the keys as described above to
 make the appropriate selections and then press the `A` key to apply them. The
 `E` key exits back to the main menu view.

 #### I set my clock! What's next?

 Congratulations! You've now got a working, set clock! At this point, use the
  main menu to select either mode 1 (fixed time/date/temperature display) or
  mode 2 (auto-toggle time/date/temperature displays) to see the time, date,
  and/or temperature.

At this point, we're left with...

## Advanced topics and settings

Your Tube Clock can do more than just show you the time, date, and temperature.
 Let's dig in and take a closer look!

### Views

The Tube Clock has a number of configuration options that may be changed,
 allowing some customization of the clock's behavior. If you've just gone
 through the process of setting the time and date, you've already begun using
 some of its simple menu system and "views".

Configuration options/settings are changed by using a number of different views.
 These views provide a way to access the various settings so that they may be
 modified. Each setting is really nothing more than a number, but this number
 determines something: the volume of the beeper, the behavior of the colons when
 displaying the time, or the hour in which DST takes effect, as some examples.
 The various views simply display this information in a way that makes sense
 while providing a means by which to modify the settings.

Entering into any given mode activates a view. Each view presents information on
 the display a little differently. In addition, the touch keys are used somewhat
 differently in each view. Let's describe the views now, as, later on when we
 discuss the various modes, we'll refer back to the views to describe the
 expected behavior (what you'll see on the display, what the touch keys will do,
 etc.) upon entering into any given mode.

#### Main Menu View

If you've gone through the process to set the time and date, you've already seen
 this view. You can identify the main menu view as you'll see only the two
 left-most tubes lit along with the lower dot on the left colon. The touch keys
 are used as follows:
* `U` increases the selected mode number
* `D` decreases the selected mode number
* `C` enters Time/Date/Temperature view in toggle mode (aka mode 2)
* `B` enters Time/Date/Temperature view in fixed mode (aka mode 1)
* `E` enters into the selected mode (as indicated by the tubes)

#### Time/Date/Temperature View

This is the view that--you guessed it--displays the time, date, and temperature.
 This same view is used whether the clock is in mode 1 (fixed display) or mode 2
 (toggle/rotate display). This is the view in which your Tube Clock will spend
 the vast majority of its time. In this view, the touch keys are used as follows:
* `U` increases the display intensity (if automatic adjustments are disabled)
* `D` decreases the display intensity (if automatic adjustments are disabled)
* `C` displays the temperature
* `B` displays the date
* `A` toggles display of time (hours, minutes, seconds) and seconds-since-midnight
* `E` exits to the main menu

#### Timer/Counter View

This is the view that displays the timer/counter. In this view, the touch keys
 work as follows:
* When the timer/counter is stopped:
  * `U` increases the timer/counter value
  * `D` decreases the timer/counter value
* When the timer/counter is running:
  * `U` counts up at next start
  * `D` counts down at next start
* `C` resets the timer/counter
* `B` stops the timer/counter
* `A` starts the timer/counter
* `E` exits to the main menu

#### DMX-512 View

This view displays data being sent to the clock via DMX-512. If no DMX-512
 signal is present on the RS-485 serial interface, this view does nothing and
 will not activate. See the DMX-512 parameters document for more information.

#### Set Time/Date View

If you've already set your clock, you've seen this view; it was also described
 above. This is the view used to set times and dates. The existing values are
 shown when the view is entered. One pair of tubes will be "highlighted" at a
 time and the value on the highlighted block is what will be modified by
 touching the `U` and `D` touch keys. In this view, the touch keys work as
 follows:
* `U` increases the selected value
* `D` decreases the selected value
* `C` moves the selection left
* `B` moves the selection right
* `A` applies the date/time as set
* `E` exits to the main menu

#### Set Bits View

This view is used to modify settings that are either on/enabled/`1` or
 off/disabled/`0`. This view will display the setting number on the middle pair
 of tubes while its state (enabled/disabled) is shown on the right-most tube.
 The existing settings are shown when the view is entered. The touch keys can be
 used to toggle the various settings on and off as follows:
* `U` selects the next bit
* `D` selects the previous bit
* `C` sets/turns on the bit
* `B` clears/turns off the bit
* `A` applies (saves) the settings
* `E` exits to the main menu

#### Set Value View

This view allows adjusting of various settings values. The existing value is
 shown when the view is entered and it appears on the right side of the display.
 The touch keys work as follows:
* `U` increases the displayed value
* `D` decreases the displayed value
* `C` sets the displayed value to the maximum allowed value for the setting
* `B` sets the displayed value to the minimum allowed value for the setting
* `A` applies (saves) the selected value
* `E` exits to the main menu

#### System Status View

This view is unique and is explained in more detail later on below.

### Time Slots

The Tube Clock has the ability to store up to eight "times of significance" --
 you can think of it as an alarm clock with eight different alarm times that you
 can set. Moving forward, we'll refer to each of these times as a "time slot".

Each of these eight time slots can be configured to trigger any (combination) of
 three events:
1. An audible alarm
1. A blinking display
1. Turn the tubes on or off

The timer/counter and external alarm inputs can also trigger the audible alarm
 or display blinking.

The first is probably self-explanatory -- when the time matches the time that
 was set into the time slot, the alarm will sound. It's just like an alarm clock.

The second is similar to the audible alarm, but instead of beeping, it'll cause
 the display to switch to full brightness and blink at 1 Hz.

The third is a bit more unique -- it allows the tubes to be shut down for a time
 interval. As an example, this feature could be used to turn the tubes off
 during the day while no one is home to view the clock. In theory, this
 preserves the life of the tubes and will also save some energy.

### Other modes

At this point, you should have a feel for how to select a given mode by using
 the main menu view. (If not, scroll up and (re)read the Main Menu view section
 above.) On the back of the PCB you'll find a listing of all of the
 clock's various operating modes; the numbers seen in that list correspond
 directly to what you can select in the main menu view. Let's go through them
 now.

#### Modes 1 and 2: Time/Date/Temperature view

These modes were mentioned briefly above, but left out some detail.

The only difference between these two modes is that mode 2 will automatically
 rotate the display between the time, date, and temperature. That aside, they
 behave the same, as they both use the Time/Date/Temperature view as described
 above.

In mode 2, it is possible to adjust how long the time, date, and temperature
 each appear. The duration for each is configured by adjusting the values set in
 modes 20, 21, and 22. The values are in seconds and the defaults are 24 (time),
 3 (date), and 3 (temperature).

#### Mode 3: Timer/Counter

Mode three allows the use of the clock as a timer or counter. It can count up or
 down in seconds. See the Timer/Counter View section above.

An alarm can be triggered when the timer reaches either zero (if counting down)
 or the reset value (if counting up). The timer/counter reset value is
 determined by the value set in mode 7. This value is applied when the
 timer/counter is reset and the timer is set to count down; if set to count up,
 resetting the timer sets it to zero.

#### Mode 4: DMX-512 view

This mode displays data being sent to the clock via DMX-512. If no DMX-512
 signal is present on the RS-485 serial interface, this mode does nothing and
 cannot be selected. See the DMX-512 parameters document for more information.

#### Modes 5 and 6: Set time/date view

View: Set Time/Date View

These modes were described in the "Getting Started" and "Set Time/Date View"
 sections above.

#### Mode 7: Set Timer/Counter reset value

View: Set Value View

This mode allows adjusting of the reset value for the timer/counter. Adjust the
 value up or down as desired and touch the `A` key to save it; changes are lost
 if not saved.

**Important:** You must touch the `A` key to save the new value! If you do not,
 the new value will be lost upon exiting the set value view.

#### Mode 8: System Status View

This view is unique from all of the other views; its main purpose is to check
 various aspects of the clock's hardware such as the tube lifetime counter.

When entering this mode for the first time after the clock is powered on, it
 will display the time (normally in hours) that the tubes have been powered on.
 Pressing the `C` touchkey will reset this timer. *For the first hour, the
 tube-on time is shown in seconds.*

The `U` and `D` touchkeys may be used to display additional information. A
 two-digit value will appear on the right-most pair of tubes as well as the left
 colon:
 - Mode 1: Displays peripherals the application found. Tube 1 = DS1722, Tube 2 =
   LM74, Tube 3 = DS3234, Tube 4 = GPS module
 - Mode 2: Displays the number of satellites the GPS module is reporting.
 - Mode 3: CPU voltage (3300 = 3.3 volts)
 - Mode 4: Battery/super capacitor voltage (3000 = 3 volts)

Touch the `E` key to exit this view.

#### Mode 10: Set Options Bits

View: Set Bits View

This mode is where we configure a number of options that govern the clock's
 behavior. These are options that can be either on or off (enabled or disabled).
 As such, bits represent them nicely. Each bit's meaning is described on the
 back of the PCB, but we'll document them here, as well:

* Bit 0: 12-hour mode - if set, the time is displayed as a 12-hour clock
 (AM/PM), otherwise it will display as a 24-hour clock.
* Bit 1: Status LED as PM indicator - if set, the status LED will illuminate
 when it is PM, otherwise it will remain off.
* Bit 2: Hourly Chime - if set, at the top of every hour, the clock will
 indicate the current hour in binary audibly using a series of high (1) and low
 (0) beeps.
* Bit 3: DST Enabled - if set, the clock will adjust itself automatically for
 daylight savings time (DST) based on values configured in settings 24 through
 29 (see below).
* Bit 4: Display Fahrenheit - temperature is shown in degrees Fahrenheit if set,
 Celsius otherwise.
* Bit 5: Automatic Intensity Adjustment - if set, the display brightness will
 automatically adjust based on the ambient light level. If clear, manual
 adjustment is enabled when the time, date, or temperature is displayed.
* Bit 6: Startup To Toggle - if set, the clock defaults to mode 2 when powered
 on or when left idle for more than one minute. Otherwise, mode 1 is the default.
* Bit 7: DMX-512 Extended mode - if set, additional DMX-512 channels are used
 enabling enhanced control of the clock via the control protocol.
* Bit 8: MSDs Off - when set, Tubes that would otherwise be leading zeros are
 turned off in the time, date, temperature, and timer/counter views.
* Bit 9: Trigger Effect on Rotate - if set, the clock run an animation to
 prevent cathode poisoning each time the display rotates in mode 2.

#### Modes 11 through 13: Time slot/Alarm behavior bits

View: Set Bits View

Each of these three modes allows enabling or disabling of each of the three
 behaviors described in the "Time Slots" section above. Bits 0 through 7
 correspond to time slots 1 through 8, bit 8 corresponds to the timer/counter,
 and bits 9 and 10 correspond to the external alarm input pins (latching and
 momentary, respectively).

Mode 11 allows enabling or disabling the audible alarms/beeping, mode 12 allows
 enabling or disabling the display blinking, and mode 13 allows enabling or
 disabling the tubes between the selected slot and the next slot.

By default, the tubes remain on for all eight time slots while the audible
 alarms and display blinking are disabled for all eight time slots.

#### Modes 20 through 22: Time, Date, and Temperature display durations

View: Set Value View

As was described above, these three modes allow the adjustment of how long (in
 seconds) the time, date, and temperature will appear when the clock is
 operating in mode 2.

#### Mode 23: Fade duration

View: Set Value View

This mode allows the adjustment of the duration of the fading that occurs as
 digits change on the display. This effect is used in display modes 1, 2, and 3.
 The value set describes the fade duration in milliseconds and the default is 100.

#### Modes 24 through 29: Daylight Savings Time (DST) settings

View: Set Value View

These modes allow the configuration of exactly when clock adjustments occur due
 to DST beginning and ending. The default values correspond with DST changes
 that occur in the US.

Mode 24 allows setting the month in which DST begins.\
Mode 25 allows setting the day of week ordinal (first, second, third, fourth) in
 which DST begins.\
Mode 26 allows setting the month in which DST ends.\
Mode 27 allows setting the day of week ordinal (first, second, third, fourth) in
 which DST ends.\
Mode 28 allows setting the day of the week on which the DST time change occurs.\
Mode 29 allows setting the hour at which the DST time change occurs.

#### Mode 30: Effect duration

View: Set Value View

This mode allows the adjustment of the duration of animations that appear to
 prevent cathode poisoning. The value is a time in milliseconds and it defaults
 to 500.

#### Mode 31: Effect frequency

View: Set Value View

This mode allows the adjustment of the frequency of animations that appear to
prevent cathode poisoning. The value is a time in seconds and it defaults to 300.

#### Mode 32: Minimum Intensity

View: Set Value View

This mode allows the adjustment of the minimum intensity allowed when automatic
 intensity adjustments are enabled (see mode 10). It is intended to keep the
 display from dimming _too much_.

#### Mode 33: Beeper Volume

View: Set Value View

This mode allows adjusting of the beeper volume. Higher numbers mean a louder
 beeper. This setting affects all sounds the beeper produces. Easy enough, yeah?

#### Mode 34: Temperature Calibration

View: Set Value View

When using the STM32's internal temperature sensor, some calibration value is
 required as they're not all consistent. This mode allows the adjustment of this
 calibration value. Note that it is a **negative** value, so increasing the
 value will _decrease_ the temperature readout.

#### Mode 35: Display Hardware Refresh Interval

View: Set Value View

The value assigned in this mode is not currently utilized.

#### Mode 36: Date Format

View: Set Value View

This mode allows adjusting of the way the year, month, and day are arranged on
 the display when the date appears. The possible values are zero (the default),
 one, or two, and they correspond to the following arrangements:
* 0: Year, Month, Day
* 1: Day, Month, Year
* 2: Month, Day, Year

Note that this setting affects how the date is displayed both when viewing the
 date (mode 1 or 2) **and** when setting the date (in mode 6).

#### Mode 37: Time zone (time offset)

View: Set Value View

This mode allows setting of the local time zone. This value is only used when a
 GPS module is attached to the clock. It may be specified in 15-minute
 intervals. The upper left colon indicator lights to indicate a negative value.

#### Mode 38: Colon behavior

View: Set Value View

This mode allows adjusting of the colon behavior when the time is displayed. The
 behavior is defined by the values as follows:
 - 0 = On
 - 1 = Off
 - 2 = Blink
 - 3 = Blink Upper
 - 4 = Blink Lower
 - 5 = Alternate Upper/Lower

#### Mode 39: DMX-512 Address

View: Set Value View

This mode allows setting of the clock's DMX-512 address. If DMX-512 is not in
 use, this setting is irrelevant. See the DMX-512 parameters document for more
 information.

#### Modes 40 through 47: Set Slot Times

View: Set Time/Date View

These modes allow setting of the time for each of the eight time slots. See the
"Time Slots" section above for more information.

---

 **Footnote:** Yes, I said **year**, **month**, **day**, the way the date is
  _supposed_ to be written/displayed. ;) If you don't like it, there is a setting
  that can be modified to change it. You're welcome. :p
