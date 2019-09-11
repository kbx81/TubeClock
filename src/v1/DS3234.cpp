//
// kbx81's tube clock DS3234 RTC class
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
#include <libopencm3/stm32/spi.h>

#include <cstdint>

#include "DateTime.h"
#include "DS3234.h"
#include "Hardware.h"
#include "SpiMaster.h"

#if HARDWARE_VERSION == 1
  #include "Hardware_v1.h"
#else
  #error HARDWARE_VERSION must be defined with a value of 1
#endif


namespace kbxTubeClock {

namespace DS3234 {

// DS3234 Registers
//
static const uint8_t cSecondsRegister                       = 0x00;
static const uint8_t cMinutesRegister                       = 0x01;
static const uint8_t cHoursRegister                         = 0x02;
static const uint8_t cDoWRegister                           = 0x03;
static const uint8_t cDateRegister                          = 0x04;
static const uint8_t cMonthRegister                         = 0x05;
static const uint8_t cYearRegister                          = 0x06;
static const uint8_t cAlarm1SecondsRegister                 = 0x07;
static const uint8_t cAlarm1MinutesRegister                 = 0x08;
static const uint8_t cAlarm1HoursRegister                   = 0x09;
static const uint8_t cAlarm1DoWDateRegister                 = 0x0a;
static const uint8_t cAlarm2MinutesRegister                 = 0x0b;
static const uint8_t cAlarm2HoursRegister                   = 0x0c;
static const uint8_t cAlarm2DoWDateRegister                 = 0x0d;
static const uint8_t cControlRegister                       = 0x0e;
static const uint8_t cStatusRegister                        = 0x0f;
static const uint8_t cAgingRegister                         = 0x10;
static const uint8_t cTemperatureMSBRegister                = 0x11;
static const uint8_t cTemperatureLSBRegister                = 0x12;
static const uint8_t cTemperatureConversionDisableRegister  = 0x13;
static const uint8_t cSramAddressRegister                   = 0x18;
static const uint8_t cSramDataRegister                      = 0x19;

// The number of registers in the chip (yeah, three are reserved...okokok...)
//
static const uint8_t cNumberOfRegisters = 25;

// The location of the register address byte in the buffer we send to the DS3234
//
static const uint8_t cAddressByte = 0x00;

// The location of the SRAM address byte in the buffer we send to the DS3234
//
static const uint8_t cSramAddressByte = 0x01;

// The location of the SRAM data in the buffer we send to the DS3234
//
static const uint8_t cSramData = 0x02;

// The maximum number of bytes handled with a single call to read/write
//
static const uint8_t cSramMaxRWSize = 20;

// A byte we use to test if the DS3234 is connected
//
static const uint8_t cTestByte = 0x5a;

// Setting this bit in the address byte makes the operation a write
//
static const uint8_t cWriteBit = 0x80;

// This bit indicates if the oscillator stopped, likely invalidating the time
//
static const uint8_t cOsfBit = (1 << 7);

// This bit indicates the century (in the month register)
//
static const uint8_t cCenturyBit = (1 << 7);


// The year base
//
static uint16_t _yearBase = 2000;

// SPI buffers
//
static uint8_t _spiRefreshBufferIn[cNumberOfRegisters];
static uint8_t _spiWorkingBufferIn[cNumberOfRegisters];
static uint8_t _spiWorkingBufferOut[cNumberOfRegisters];

// A full copy of DS3234 registers, refreshed by calling the refresh() function
// ...dirty black magic, but it works...
static uint8_t* _ds3234Registers  = _spiRefreshBufferIn + 1;
static uint8_t* _ds3234RegisterIn  = _spiWorkingBufferIn + 1;
static uint8_t* _ds3234RegisterOut = _spiWorkingBufferOut + 1;

// Pointer to SpiMaster, initialized by initialize()
//
static SpiMaster *_spiMaster = nullptr;

// Slave ID assigned by SpiMaster
//
static uint8_t _slaveId = SpiMaster::cNoSlave;


// Function to convert BCD format into binary format
//
static inline uint8_t convertBcdToBin(const uint8_t bcd)
{
    return (bcd & 0xf) + ((bcd >> 4) * 10);
}


// Function to convert binary to BCD format
//
static inline uint8_t convertBinToBcd(const uint8_t bin)
{
    return (bin % 10) + ((bin / 10) << 4);
}


void initialize()
{
  SpiMaster::SpiSlave mySlave = {
    .gpioPort       = Hardware::cNssPort,   // gpio port on which CS line lives
    .gpioPin        = Hardware::cNssRtcPin, // gpio pin on which CS line lives
    .strobeCs       = false,                // CS line is strobed upon xfer completion if true
    .polarity       = false,                // CS/CE polarity (true = active high)
    .misoPort       = Hardware::cSpi1Port,              // port on which slave inputs data
    .misoPin        = Hardware::cSpi1MisoPin,           // pin on which slave inputs data
    .br             = SPI_CR1_BAUDRATE_FPCLK_DIV_16,    // Baudrate
    .cpol           = SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,  // Clock polarity
    .cpha           = SPI_CR1_CPHA_CLK_TRANSITION_2,    // Clock Phase
    .lsbFirst       = SPI_CR1_MSBFIRST,     // Frame format -- lsb/msb first
    .dataSize       = SPI_CR2_DS_8BIT,      // Data size (4 to 16 bits, see RM)
    .memorySize     = DMA_CCR_MSIZE_8BIT,   // Memory word width (8, 16, 32 bit)
    .peripheralSize = DMA_CCR_PSIZE_8BIT    // Peripheral word width (8, 16, 32 bit)
  };

  _spiMaster = Hardware::getSpiMaster();

  _slaveId = _spiMaster->registerSlave(&mySlave);
}


bool isConnected()
{
  SpiMaster::SpiTransferReq* request = _spiMaster->getTransferRequestBuffer(_slaveId);

  if (request != nullptr)
  {
    // We confirm connectivity by attempting to set and then read back the SRAM
    //  address register...so start by writing at the SRAM Address register
    _spiWorkingBufferOut[cAddressByte] = cSramAddressRegister | cWriteBit;
    // Load the SRAM Address register with the test value
    _spiWorkingBufferOut[cSramAddressByte] = cTestByte;
    // Set the SRAM Address regsiter
    request->bufferIn = _spiWorkingBufferIn;
    request->bufferOut = _spiWorkingBufferOut;
    request->length = 2;
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;
    // We must wait for the transfer to complete before we touch any buffers again
    while (_spiMaster->transferComplete(_slaveId) == false);

    // This is the address we want to start reading from
    _spiWorkingBufferOut[cAddressByte] = cSramAddressRegister;
    // Try to read the byte back and check if it matches what we expect
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;
    // We must wait for the transfer to complete before we touch any buffers again
    while (_spiMaster->transferComplete(_slaveId) == false);

    // If we read back the byte we wrote, the RTC is very likely connected
    if (_ds3234RegisterIn[0] == cTestByte)
    {
      // Kick off a full register refresh
      while (refresh(true) != SpiMaster::SpiReqAck::SpiReqAckOk);

      return true;
    }
  }
  return false;
}


bool isRunning()
{
  // If the 7th bit is zero, the RTC is running
  return (_ds3234RegisterIn[cControlRegister] & cOsfBit) == 0;
}


bool isValid()
{
  // If the 7th bit (OSF) is zero, the oscillator has not stopped so the RTC is valid
  return (_ds3234RegisterIn[cStatusRegister] & cOsfBit) == 0;
}


DateTime getDateTime()
{
  // Convert the values into a date object and return it
  return DateTime(
      static_cast<uint16_t>(convertBcdToBin(_ds3234Registers[cYearRegister])) + ((_ds3234Registers[cMonthRegister] & cCenturyBit) != 0 ? (_yearBase + 100) : _yearBase),
      convertBcdToBin(_ds3234Registers[cMonthRegister] & 0x1f),
      convertBcdToBin(_ds3234Registers[cDateRegister] & 0x3f),
      convertBcdToBin(_ds3234Registers[cHoursRegister] & 0x3f),
      convertBcdToBin(_ds3234Registers[cMinutesRegister] & 0x7f),
      convertBcdToBin(_ds3234Registers[cSecondsRegister] & 0x7f));
}


uint16_t getTemperatureRegister()
{
  return (_ds3234RegisterIn[cTemperatureMSBRegister] << 8) | _ds3234RegisterIn[cTemperatureLSBRegister];
}


int16_t getTemperatureWholePart()
{
  return ((int8_t)_ds3234RegisterIn[cTemperatureMSBRegister]);
}


uint16_t getTemperatureFractionalPart()
{
  return (_ds3234RegisterIn[cTemperatureLSBRegister] >> 4);
}


void setBaseYear(uint16_t yearBase)
{
    _yearBase = yearBase;
}


void setDateTime(const DateTime &dateTime)
{
  SpiMaster::SpiTransferReq* request = _spiMaster->getTransferRequestBuffer(_slaveId);

  if (request != nullptr)
  {
    // Wait...just in case a transfer is already in progress
    while (_spiMaster->transferComplete(_slaveId) == false);
    // first, we must send the address of the register at which the write is to begin
    _spiWorkingBufferOut[cAddressByte] = cStatusRegister | cWriteBit;
    // clear the OSF bit 7, keep EN32kHz at its default
    _spiWorkingBufferOut[cAddressByte + 1] = 0x48;
    // Set the buffer up with our data stuff and trigger the transfer to write to
    //  the status register to clear the OSF
    request->bufferIn = _spiWorkingBufferIn;
    request->bufferOut = _spiWorkingBufferOut;
    request->length = 2;
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;
    // We must wait for the transfer to complete before we touch any buffers again
    while (_spiMaster->transferComplete(_slaveId) == false);

    // Again, we need to write the starting register address first...
    _spiWorkingBufferOut[cAddressByte] = cSecondsRegister | cWriteBit;

    _ds3234RegisterOut[cSecondsRegister] = dateTime.second(true);
    _ds3234RegisterOut[cMinutesRegister] = dateTime.minute(true);
    _ds3234RegisterOut[cHoursRegister] = dateTime.hour(true, false);
    _ds3234RegisterOut[cDoWRegister] = dateTime.dayOfWeek();
    _ds3234RegisterOut[cDateRegister] = dateTime.day(true);
    _ds3234RegisterOut[cMonthRegister] = dateTime.month(true);
    _ds3234RegisterOut[cYearRegister] = dateTime.yearShort(true);
    // Based on the buffer we set up above, we'll start at the seconds register
    //  and write up from there
    request->length = 8;
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;
    while (_spiMaster->transferComplete(_slaveId) == false);
  }
}


SpiMaster::SpiReqAck getRegister(const uint8_t registerAddress, uint8_t* const registerDataBuffer, const uint8_t numberOfBytes)
{
  SpiMaster::SpiTransferReq* request = _spiMaster->getTransferRequestBuffer(_slaveId);

  if (request != nullptr)
  {
    // This is the address we want to start reading from
    _spiWorkingBufferOut[cAddressByte] = registerAddress;

    request->bufferIn = registerDataBuffer;
    request->bufferOut = _spiWorkingBufferOut;
    request->length = numberOfBytes;
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;
    // We must wait for the transfer to complete before we touch any buffers again
    while (_spiMaster->transferComplete(_slaveId) == false);

    return request->state;
  }
  return SpiMaster::SpiReqAck::SpiReqAckError;
}


SpiMaster::SpiReqAck setRegister(const uint8_t registerAddress, uint8_t* const registerDataBuffer, const uint8_t numberOfBytes, const bool block)
{
  SpiMaster::SpiTransferReq* request = _spiMaster->getTransferRequestBuffer(_slaveId);

  if (request != nullptr)
  {
    // Start writing at the specified register
    _spiWorkingBufferOut[cAddressByte] = registerAddress | cWriteBit;

    for (uint8_t i = 0; (i < numberOfBytes) && (i < cNumberOfRegisters); i++)
    {
      _ds3234RegisterOut[i] = registerDataBuffer[i];
    }

    request->bufferIn = registerDataBuffer;
    request->bufferOut = _spiWorkingBufferOut;
    request->length = numberOfBytes + 1;
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;
    // We must wait for the transfer to complete before we touch any buffers again
    while ((_spiMaster->transferComplete(_slaveId) == false) && (block == true));

    return request->state;
  }
  return SpiMaster::SpiReqAck::SpiReqAckError;
}


SpiMaster::SpiReqAck readSram(const uint8_t sramStartAddress, uint8_t* const data, const uint8_t numberOfBytes)
{
  SpiMaster::SpiTransferReq* request = _spiMaster->getTransferRequestBuffer(_slaveId);

  if ((request != nullptr) && (numberOfBytes < cSramMaxRWSize))
  {
    // Start writing at the SRAM Address register
    _spiWorkingBufferOut[cAddressByte] = cSramAddressRegister | cWriteBit;
    // Set SRAM Address register to point to sramStartAddress
    _spiWorkingBufferOut[cSramAddressByte] = sramStartAddress;
    // Set the SRAM Address regsiter
    request->bufferIn = _spiWorkingBufferIn;
    request->bufferOut = _spiWorkingBufferOut;
    request->length = 2;
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;
    // We must wait for the transfer to complete before we touch any buffers again
    while (_spiMaster->transferComplete(_slaveId) == false);

    // This is the address we want to start reading from
    _spiWorkingBufferOut[cAddressByte] = cSramDataRegister;
    // Try to read the byte back and check if it matches what we expect
    request->length = numberOfBytes + 1;
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;
    // We must wait for the transfer to complete before we touch any buffers again
    while (_spiMaster->transferComplete(_slaveId) == false);
    // finally, copy the data read into the given buffer
    for (uint8_t i = 0; (i < numberOfBytes) && (i < cSramMaxRWSize); i++)
    {
      data[i] = _spiWorkingBufferIn[i + 1];
    }

    return request->state;
  }
  return SpiMaster::SpiReqAck::SpiReqAckError;
}


SpiMaster::SpiReqAck writeSram(const uint8_t sramStartAddress, uint8_t* const data, const uint8_t numberOfBytes, const bool block)
{
  SpiMaster::SpiTransferReq* request = _spiMaster->getTransferRequestBuffer(_slaveId);

  if ((request != nullptr) && (numberOfBytes < cSramMaxRWSize))
  {
    // Start writing at the SRAM Address register
    _spiWorkingBufferOut[cAddressByte] = cSramAddressRegister | cWriteBit;
    // Set SRAM Address register to point to sramStartAddress
    _spiWorkingBufferOut[cSramAddressByte] = sramStartAddress;
    // We want to write the data in the passed buffer
    for (uint8_t i = 0; (i < numberOfBytes) && (i < cSramMaxRWSize); i++)
    {
      _spiWorkingBufferOut[i + 2] = data[i];
    }

    // Set the SRAM Address regsiter
    request->bufferIn = _spiWorkingBufferIn;
    request->bufferOut = _spiWorkingBufferOut;
    request->length = numberOfBytes + 2;
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;
    // We must wait for the transfer to complete before we touch any buffers again
    while ((_spiMaster->transferComplete(_slaveId) == false) && (block == true));

    return request->state;
  }
  return SpiMaster::SpiReqAck::SpiReqAckError;
}


SpiMaster::SpiReqAck refresh(const bool block)
{
  SpiMaster::SpiTransferReq* request = _spiMaster->getTransferRequestBuffer(_slaveId);

  if (request != nullptr)
  {
    // Wait...just in case a transfer is already in progress
    while (_spiMaster->transferComplete(_slaveId) == false);
    // Address the seconds register, then (try to) read all the registers
    _spiWorkingBufferOut[cAddressByte] = cSecondsRegister;

    request->bufferIn = _spiRefreshBufferIn;
    request->bufferOut = _spiWorkingBufferOut;
    request->length = cNumberOfRegisters;
    request->state = SpiMaster::SpiReqAck::SpiReqAckQueued;

    while ((_spiMaster->transferComplete(_slaveId) == false) && (block == true));

    return SpiMaster::SpiReqAck::SpiReqAckOk;
  }
  return SpiMaster::SpiReqAck::SpiReqAckError;
}


}

}
