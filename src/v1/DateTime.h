//
// kbx81's tube clock DateTime class
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
#pragma once


#include <cstdint>


namespace kbxTubeClock {


/// A rock solid date/time class for a Gregorian calendar.
///
/// This class was specifically made for the Ardurino environment. It works
/// well with 8bit and 32bit microcontrollers. Conversion to seconds and
/// back is slow, because it uses an algorithmic method instead of a
/// mathematical one. The class also allocates around 200 bytes of flash
/// memory for date/time formats. If you do not need the toString() method,
/// you can free this memory removing these strings.
///
/// Supports years starting from 2000, but toSecsSince2000() is limited
/// by the size of the 32bit integer which works up to 2136-02-07 06:28:15.
/// If you use dates after this point, the result of the seconds conversion
/// is undefined.
///
/// Leap years are correctly handled. The class is fully tested with all
/// possible values up to 2136-02-07 06:28:15.
///
/// Note: For the formatting I follow the ISO 8601 standard with some common
/// derivations for better readability. This whole class has ISO standards
/// in mind, therefore I use everywhere the logical ordering of dates:
/// year, month, day, hour, minute, second or for the date day, month and year.
/// If you live in one of the 6 countries on the world with special ordering
/// of date values: US, the Federated States of Micronesia, Philippines, Palau,
/// Canada and Belize, feel free to adjust the code to your needs. Be aware
/// that only these 6 countries out of ~195 write the month before the day.
///
class DateTime {

public:
  /// Create the first possible date/time which is 2000-01-01 00:00:00.
  ///
  /// You can check for this special date/time using the isFirst()
  /// method.
  ///
  DateTime();

  /// Create a new date/time with the given values.
  /// All values are constrained to valid values, day of week is calculated.
  ///
  /// @param year The year from 2000-9999.
  /// @param month The month from 1=January to 12=December.
  /// @param day The day of the month from 1 to 31.
  /// @param hour The hour from 0 to 23.
  /// @param minute The minute from 0 to 59.
  /// @param second The second from 0 to 59.
  ///
  DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour = 0, uint8_t minute = 0, uint8_t second = 0);

  /// dtor
  ///
  // ~DateTime();

public:
  /// Compare this date/time to another value
  ///
  bool operator==(const DateTime &other) const;

  /// Compare this date/time to another value
  ///
  bool operator!=(const DateTime &other) const;

  /// Compare this date/time to another value
  ///
  bool operator<(const DateTime &other) const;

  /// Compare this date/time to another value
  ///
  bool operator<=(const DateTime &other) const;

  /// Compare this date/time to another value
  ///
  bool operator>(const DateTime &other) const;

  /// Compare this date/time to another value
  ///
  bool operator>=(const DateTime &other) const;

public:
  /// Set the date.
  /// Valid values 2000-9999.
  ///
  void setDate(uint16_t year, uint16_t month, uint16_t day);

  /// Set the time
  ///
  void setTime(uint8_t hour, uint8_t minute, uint8_t second);

  /// Get the year.
  /// Value from 2000-9999.
  ///
  uint16_t year(const bool bcd = false) const;

  /// Get the year (two-digit).
  /// Value from 00-99.
  ///
  uint8_t yearShort(const bool bcd = false) const;

  /// Get the month.
  /// Value from 1=January to 12=December.
  ///
  uint8_t month(const bool bcd = false) const;

  /// Get the day.
  /// Value from 1 to 31.
  ///
  uint8_t day(const bool bcd = false) const;

  /// Get the day of the week.
  /// Value from 0=Sunday, 1=Monday...6=Saturday
  ///
  uint8_t dayOfWeek() const;

  /// Set the day of the week.
  /// Value from 0=Sunday, 1=Monday...6=Saturday
  ///
  void setDayOfWeek(uint8_t dayOfWeek);

  /// Get the hour.
  /// If format == true, value from 1 to 12, else value 0 to 23
  ///
  uint8_t hour(const bool bcd = false, const bool format12Hour = false) const;

  /// Get the minute.
  /// Value from 0-59.
  ///
  uint8_t minute(const bool bcd = false) const;

  /// Get the second.
  /// Value from 0-59.
  ///
  uint8_t second(const bool bcd = false) const;

  /// Returns true if _hour > 12
  ///
  bool isPM() const;

  /// Get a new date/time with the given number of seconds added.
  /// This is a slow operation.
  ///
  DateTime addSeconds(int32_t seconds) const;

  /// Get a new date/time with the given number of days added.
  /// This is a slow operation.
  ///
  DateTime addDays(int32_t days) const;

  /// Returns the number of days in the current month and year
  ///
  uint8_t daysThisMonth() const;

  /// Returns the number of seconds since midnight.
  ///
  uint32_t secondsSinceMidnight(const bool bcd = false);

  /// Get the number of seconds to the other date/time.
  /// This is a slow operation. It works only correctly with differences
  /// up to 62 years because of the limitation of the 32bit value.
  ///
  int32_t secondsTo(const DateTime &other) const;

  /// Convert the date/time into seconds since 2000-01-01 00:00:00.
  ///
  /// If you need a unix timestamp, add 0x386D4380 to this value.
  ///
  uint32_t toSecondsSince2000() const;

  /// Check if this is the first possible date/time 2000-01-01 00:00:00.
  ///
  bool isFirst() const;

public:
  /// Create a new date/time object from the given unix time.
  ///
  /// If you convert from a unix timestamp, subtract 0x386D4380 from this value.
  ///
  static DateTime fromSecondsSince2000(uint32_t seconds);

  /// Create a new completely unchecked date time object from
  /// the given values. You have to make sure all values are in the
  /// correct ranges.
  ///
  static DateTime fromUncheckedValues(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint8_t dayOfWeek);

private:
  /// Create a new unchecked date/time object.
  ///
  DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint8_t dayOfWeek);

private:
  uint16_t _year; ///< The year
  uint8_t _month; ///< The month 1-12
  uint8_t _day; ///< The day 1-31
  uint8_t _hour; ///< The hour 0-23
  uint8_t _minute; ///< The minute 0-59
  uint8_t _second; ///< The second 0-59
  uint8_t _dayOfWeek; ///< The day of the week 0-6
};


}
