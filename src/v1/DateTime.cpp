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
#include <cstdint>
#include "DateTime.h"
#include "Hardware.h"


namespace kbxTubeClock {


namespace {


// The number of days per month.
static const uint8_t cDaysPerMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// The number of seconds per day.
static const uint32_t cSecondsPerDay = 86400;

// The number of seconds per hour.
static const uint16_t cSecondsPerHour = 3600;

// The number of seconds per minute.
static const uint16_t cSecondsPerMinute = 60;

// The number of days for a regular year.
static const uint32_t cDaysPerNormalYear = 365;


// Calculate the day of the week
// Using the formula from: http://www.tondering.dk/claus/cal/chrweek.php
static uint8_t calculateDayOfWeek(int16_t year, int16_t month, int16_t day)
{
  const int16_t a = ((14 - month) / 12);
  const int16_t y = year - a;
  const int16_t m = month + (12 * a) - 2;
  const int16_t d = (day + y + (y / 4) - (y / 100) + (y / 400) + ((31 * m) / 12)) % 7;
  return d;
}


static inline bool isLeapYear(uint16_t year)
{
  return ((year & 3) == 0 && year % 100 != 0) || (year % 400 == 0);
}


static inline uint8_t getMaxDayPerMonth(uint16_t year, uint8_t month)
{
  if (month == 2 && isLeapYear(year))
  {
    return 29;
  }
  return cDaysPerMonth[month];
}


static inline uint32_t getDaysForYear(uint16_t year)
{
  if (isLeapYear(year))
  {
    return cDaysPerNormalYear + 1;
  }
  else
  {
    return cDaysPerNormalYear;
  }
}


}


DateTime::DateTime()
  : _year(2000), _month(1), _day(1), _hour(0), _minute(0), _second(0), _dayOfWeek(6)
{
}


DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
  : _year(year), _month(month), _day(day), _hour(hour), _minute(minute), _second(second), _dayOfWeek(0)
{
  setDate(year, month, day);
  setTime(hour, minute, second);
}


DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint8_t dayOfWeek)
  : _year(year), _month(month), _day(day), _hour(hour), _minute(minute), _second(second), _dayOfWeek(dayOfWeek)
{
}


bool DateTime::operator==(const DateTime &other) const
{
  return _second == other._second &&
          _minute == other._minute &&
          _hour == other._hour &&
          _day == other._day &&
          _month == other._month &&
          _year == other._year;
}


bool DateTime::operator!=(const DateTime &other) const
{
  return !operator==(other);
}


bool DateTime::operator<(const DateTime &other) const
{
  if (_year != other._year)
  {
    return _year < other._year;
  }
  else if (_month != other._month)
  {
    return _month < other._month;
  }
  else if (_day != other._day)
  {
    return _day < other._day;
  }
  else if (_hour != other._hour)
  {
    return _hour < other._hour;
  }
  else if (_minute != other._minute)
  {
    return _minute < other._minute;
  }
  else
  {
    return _second < other._second;
  }
}


bool DateTime::operator<=(const DateTime &other) const
{
  return operator<(other) || operator==(other);
}


bool DateTime::operator>(const DateTime &other) const
{
  if (_year != other._year)
  {
    return _year > other._year;
  }
  else if (_month != other._month)
  {
    return _month > other._month;
  }
  else if (_day != other._day)
  {
    return _day > other._day;
  }
  else if (_hour != other._hour)
  {
    return _hour > other._hour;
  }
  else if (_minute != other._minute)
  {
    return _minute > other._minute;
  }
  else
  {
    return _second > other._second;
  }
}


bool DateTime::operator>=(const DateTime &other) const
{
  return operator>(other) || operator==(other);
}


void DateTime::setDate(uint16_t year, uint16_t month, uint16_t day)
{
  // force all values into valid ranges.
  if (year < 2000)
  {
    _year = 2000;
  }
  else if (year > 9999)
  {
    _year = 9999;
  }
  else
  {
    _year = year;
  }

  if (month < 1)
  {
    _month = 1;
  }
  else if (month > 12)
  {
    _month = 12;
  }
  else
  {
    _month = month;
  }

  const uint8_t maxDayPerMonth = getMaxDayPerMonth(_year, _month);

  if (day < 1)
  {
    _day = 1;
  }
  else if (day > maxDayPerMonth)
  {
    _day = maxDayPerMonth;
  }
  else
  {
    _day = day;
  }

  _dayOfWeek = calculateDayOfWeek(_year, _month, _day);
}


void DateTime::setTime(uint8_t hour, uint8_t minute, uint8_t second)
{
  if (hour > 23)
  {
    _hour = 23;
  }
  else
  {
    _hour = hour;
  }
  if (minute > 59)
  {
    _minute = 59;
  }
  else
  {
    _minute = minute;
  }
  if (second > 59)
  {
    _second = 59;
  }
  else
  {
    _second = second;
  }
}


uint16_t DateTime::year(const bool bcd) const
{
  if (bcd)
  {
    return Hardware::uint32ToBcd(_year);
  }

  return _year;
}


uint8_t DateTime::yearShort(const bool bcd) const
{
  if (bcd)
  {
    return Hardware::uint32ToBcd(_year - 2000);
  }

  return (_year - 2000);
}


uint8_t DateTime::month(const bool bcd) const
{
  if (bcd)
  {
    return Hardware::uint32ToBcd(_month);
  }

  return _month;
}


uint8_t DateTime::day(const bool bcd) const
{
  if (bcd)
  {
    return Hardware::uint32ToBcd(_day);
  }

  return _day;
}


uint8_t DateTime::dayOfWeek() const
{
  return _dayOfWeek;
}


uint8_t DateTime::hour(const bool bcd, const bool format12Hour) const
{
  if (format12Hour == true && (_hour > 12 || _hour == 0))
  {
    if (bcd && _hour > 0)
    {
      return Hardware::uint32ToBcd(_hour - 12);
    }
    else if (bcd && _hour == 0)
    {
      return 0x12;  // hour is zero so return twelve BCD-encoded
    }
    else if (!bcd && _hour > 0)
    {
      return _hour - 12;
    }
    else
    {
      return 12;  // plain old twelve
    }
  }
  else
  {
    if (bcd)
    {
      return Hardware::uint32ToBcd(_hour);
    }

    return _hour;
  }
}


uint8_t DateTime::minute(const bool bcd) const
{
  if (bcd)
  {
    return Hardware::uint32ToBcd(_minute);
  }

  return _minute;
}


uint8_t DateTime::second(const bool bcd) const
{
  if (bcd)
  {
    return Hardware::uint32ToBcd(_second);
  }

  return _second;
}


bool DateTime::isPM() const
{
  return (_hour >= 12);
}


DateTime DateTime::addSeconds(int32_t seconds) const
{
  return fromSecondsSince2000(toSecondsSince2000() + seconds);
}


DateTime DateTime::addDays(int32_t days) const
{
  return fromSecondsSince2000(toSecondsSince2000() + days * cSecondsPerDay);
}


uint8_t DateTime::daysThisMonth() const
{
  return getMaxDayPerMonth(_year, _month);
}


uint32_t DateTime::secondsSinceMidnight(const bool bcd)
{
  if (bcd)
  {
    return Hardware::uint32ToBcd(_second + (cSecondsPerMinute * _minute) + (cSecondsPerHour * _hour));
  }
  return _second + (cSecondsPerMinute * _minute) + (cSecondsPerHour * _hour);
}


int32_t DateTime::secondsTo(const DateTime &other) const
{
  return static_cast<int32_t>(other.toSecondsSince2000()) - static_cast<int32_t>(toSecondsSince2000());
}


uint32_t DateTime::toSecondsSince2000() const
{
  // This calculation will require some CPU cycles.
  // It's a programmatic solution, not a mathematical one.

  uint32_t seconds = 0;
  for (uint16_t year = 2000; year < _year; ++year)
  {
    seconds += (getDaysForYear(year) * static_cast<uint32_t>(cSecondsPerDay));
  }
  for (uint8_t month = 1; month < _month; ++month)
  {
    seconds += (static_cast<uint32_t>(getMaxDayPerMonth(_year, month)) * static_cast<uint32_t>(cSecondsPerDay));
  }
  seconds += static_cast<uint32_t>(_day - 1) * static_cast<uint32_t>(cSecondsPerDay);
  seconds += static_cast<uint32_t>(_hour) * static_cast<uint32_t>(cSecondsPerHour);
  seconds += static_cast<uint32_t>(_minute) * static_cast<uint32_t>(cSecondsPerMinute);
  seconds += static_cast<uint32_t>(_second);
  return seconds;
}


bool DateTime::isFirst() const
{
  return _year == 2000 && _month == 1 && _day == 1 && _hour == 0 && _minute == 0 && _second == 0;
}


DateTime DateTime::fromSecondsSince2000(uint32_t secondsSince2000)
{
  // This calculation will require some CPU cycles. It is a
  // programmatic solution, not a mathematical one. This function
  // is approximate 6 times slower than mathematical implementations.

  // Calculate the time
  uint32_t secondsSinceMidnight = secondsSince2000 % cSecondsPerDay;
  const uint8_t hours = secondsSinceMidnight / static_cast<uint32_t>(cSecondsPerHour);
  secondsSinceMidnight %= static_cast<uint32_t>(cSecondsPerHour);
  const uint8_t minutes = secondsSinceMidnight / static_cast<uint32_t>(cSecondsPerMinute);
  const uint8_t seconds = secondsSinceMidnight % static_cast<uint32_t>(cSecondsPerMinute);
  // Calculate the date
  uint32_t days = secondsSince2000 / static_cast<uint32_t>(cSecondsPerDay);
  const uint8_t dayOfWeek = (days + 6) % 7; // 2000-01-01 was Saturday (6)
  uint16_t year = 2000;
  uint32_t daysForThisSection = getDaysForYear(year);
  while (days >= daysForThisSection)
  {
    ++year;
    days -= daysForThisSection;
    daysForThisSection = getDaysForYear(year);
  }
  uint16_t month = 1;
  daysForThisSection = getMaxDayPerMonth(year, month);
  while (days >= daysForThisSection)
  {
    ++month;
    days -= daysForThisSection;
    daysForThisSection = getMaxDayPerMonth(year, month);
  }
  return DateTime(year, month, days+1, hours, minutes, seconds, dayOfWeek);
}


DateTime DateTime::fromUncheckedValues(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint8_t dayOfWeek)
{
  return DateTime(year, month, day, hour, minute, second, dayOfWeek);
}


}
