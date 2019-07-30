//
// kbx81's tube clock GpsReceiver Library
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
#include <string.h>

#include <libopencm3/stm32/usart.h>

#include "Hardware.h"
#include "GpsReceiver.h"

#if HARDWARE_VERSION == 1
  #include "Hardware_v1.h"
#else
  #error HARDWARE_VERSION must be defined with a value of 1
#endif


namespace kbxTubeClock {

namespace GpsReceiver
{

  /// @brief GPS Receiver state machine states
  ///
  enum GpsRxState : uint8_t {
    GpsRxIdle            = 0,
    GpsRxGettingHeader   = 1,
    GpsRxNmeaHeaderValid = 2,
    GpsRxTimeReceived    = 3,
    GpsRxTimeCompleted   = 4,
    GpsRxWaitForDate     = 5,
    GpsRxDate            = 6,
    GpsRxNumSatellites   = 7
  };

  /// @brief GPS Receiver sentence type
  ///
  enum GpsRxType : uint8_t {
    GpsRMC = 0,
    GpsGSV = 1
  };


// the header on the string we expect to parse
// $GPRMC,000012.800,V,,,,,0.00,0.00,060180,,,N*49
// $GPRMC,230912.000,A,4055.5342,N,08341.7690,W,1.24,299.67,070519,,,D*71
//
static const char cGpsParseStrGSV[] = "$GPGSV,";
static const char cGpsParseStrRMC[] = "$GPRMC,";

// the length of the string we expect to parse
//
static const uint8_t cGpsNmeaHeaderLength = 7;

// the length of the date and time strings we expect to receive
//
static const uint8_t cGpsDateTimeLength = 6;

// the length of the number of satellites string we expect to receive
//
static const uint8_t cGpsNumSatellitesLength = 2;

// the number of commas seen before we receive the date
//
static const uint8_t cGpsPreDateCommaCount = 7;

// the number of commas seen before we receive the number of satellites
//
static const uint8_t cGpsPreNumSatellitesCommaCount = 2;

// tracks the receiver state machine's state
//
static GpsRxState _rxState = GpsRxState::GpsRxIdle;

// tracks the sentence type the receiver is sending
//
static GpsRxType _rxType = GpsRxType::GpsRMC;

// a counter for incoming characters
//
static uint8_t _rxCharCount = 0;

// indicates the validity of the incoming data
//
static char _rxValidChar = 0;

// buffer for sentence header
//
static char _rxHeaderChar[] = "$GPRMC,";

// date and time incoming character buffers
//
static char _rxDateChar[6];
static char _rxTimeChar[6];

// number of satellite character buffer
//
static char _rxNumSatellitesChar[2];

// number of  minutes added to GPS time to compute local time
//
static int16_t _timeZoneOffsetInMinutes = 0;

// the current date and time received from the GPS
//
static DateTime _gpsTime;

// the current number of satellites seen by the GPS
//
static uint8_t _numSatellites = 0;


/// @brief Updates _numSatellites based on the receive buffer
///
void _rxGsvComplete()
{
  _numSatellites = ((_rxNumSatellitesChar[0] - '0') * 10) + (_rxNumSatellitesChar[1] - '0');
}


/// @brief Updates our DateTime object based on the receive buffers
///
void _rxRmcComplete()
{
  uint16_t year = 2000;
  uint8_t month = 0, day = 0, hour = 0, minute = 0, second = 0;

  day   = ((_rxDateChar[0] - '0') * 10) + (_rxDateChar[1] - '0');
  month = ((_rxDateChar[2] - '0') * 10) + (_rxDateChar[3] - '0');
  year += ((_rxDateChar[4] - '0') * 10) + (_rxDateChar[5] - '0');

  hour   = ((_rxTimeChar[0] - '0') * 10) + (_rxTimeChar[1] - '0');
  minute = ((_rxTimeChar[2] - '0') * 10) + (_rxTimeChar[3] - '0');
  second = ((_rxTimeChar[4] - '0') * 10) + (_rxTimeChar[5] - '0');

  _gpsTime.setDate(year, month, day);
  _gpsTime.setTime(hour, minute, second);
}


bool isConnected()
{
  return ((_rxValidChar == 'A') || (_rxValidChar == 'V'));
}

bool isValid()
{
  return (_rxValidChar == 'A');
}


DateTime getDateTime()
{
  return _gpsTime;
}


DateTime getLocalDateTime()
{
  return _gpsTime.addSeconds(_timeZoneOffsetInMinutes * 60);
}


uint8_t getSatellitesInView()
{
  return _numSatellites;
}


void setTimeZone(const int16_t offsetInMinutes)
{
  _timeZoneOffsetInMinutes = offsetInMinutes;
}


void rxIsr()
{
  uint8_t rxChar = USART1_RDR;

  switch (_rxState)
  {
    // at this point, we've seen the "$GPRMC," characters and can expect the time or a sentence count next
    case GpsRxState::GpsRxNmeaHeaderValid:
      if (_rxType == GpsRxType::GpsRMC)
      {
        _rxTimeChar[_rxCharCount] = rxChar;

        if (++_rxCharCount >= cGpsDateTimeLength)
        {
          _rxState = GpsRxState::GpsRxTimeReceived;
          _rxCharCount = 0;
        }
      }
      else if (_rxType == GpsRxType::GpsGSV)
      {
        if (rxChar == ',')
        {
          if (++_rxCharCount >= cGpsPreNumSatellitesCommaCount)
          {
            _rxState = GpsRxState::GpsRxNumSatellites;
            _rxCharCount = 0;
          }
        }
      }
      break;

    // the time has been received, here we'll just eat a comma and move along
    case GpsRxState::GpsRxTimeReceived:
      if (rxChar == ',')
      {
        _rxState = GpsRxState::GpsRxTimeCompleted;
      }
      break;

    // now we should see either an 'A' or a 'V' indicating validity
    case GpsRxState::GpsRxTimeCompleted:
      if ((rxChar == 'A') || (rxChar == 'V'))
      {
        _rxValidChar = rxChar;
        _rxState = GpsRxState::GpsRxWaitForDate;
      }
      else
      {
        _rxState = GpsRxState::GpsRxIdle;
      }
      break;

    // the validity has been received, now we'll eat more commas
    case GpsRxState::GpsRxWaitForDate:
      if (rxChar == ',')
      {
        if (++_rxCharCount >= cGpsPreDateCommaCount)
        {
          _rxState = GpsRxState::GpsRxDate;
          _rxCharCount = 0;
        }
      }
      break;

    // finally we get the date
    case GpsRxState::GpsRxDate:
      _rxDateChar[_rxCharCount] = rxChar;

      if (++_rxCharCount >= cGpsDateTimeLength)
      {
        _rxState = GpsRxState::GpsRxIdle;
        _rxCharCount = 0;
        _rxRmcComplete();
      }
      break;

    // read in the two-digit number of satellites
    case GpsRxState::GpsRxNumSatellites:
      _rxNumSatellitesChar[_rxCharCount] = rxChar;

      if (++_rxCharCount >= cGpsNumSatellitesLength)
      {
        _rxState = GpsRxState::GpsRxIdle;
        _rxCharCount = 0;
        _rxGsvComplete();
      }
      break;

    // the first big step...read in up to cGpsNmeaHeaderLength header characters
    case GpsRxState::GpsRxGettingHeader:
      _rxHeaderChar[_rxCharCount] = rxChar;
      // if we've received enough characters, compare the strings for validity
      if (++_rxCharCount >= cGpsNmeaHeaderLength)
      {
        if (strcmp(cGpsParseStrGSV, _rxHeaderChar) == 0)
        {
          _rxState = GpsRxState::GpsRxNmeaHeaderValid;
          _rxType = GpsRxType::GpsGSV;
          _rxCharCount = 0;
        }
        else if (strcmp(cGpsParseStrRMC, _rxHeaderChar) == 0)
        {
          _rxState = GpsRxState::GpsRxNmeaHeaderValid;
          _rxType = GpsRxType::GpsRMC;
          _rxCharCount = 0;
        }
        else  // nothing matched so just start over
        {
          _rxState = GpsRxState::GpsRxIdle;
          _rxCharCount = 0;
        }
      }
      break;

    // it starts here...we patiently wait for that money header tag! ('$')
    default:
      if (rxChar == '$')
      {
        _rxHeaderChar[_rxCharCount++] = rxChar;

        _rxState = GpsRxState::GpsRxGettingHeader;
      }
  }
}


}

}
