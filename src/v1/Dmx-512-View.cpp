//
// kbx81's tube clock DMX-512 View class
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

#include "Application.h"
#include "Display.h"
#include "DisplayManager.h"
#include "Dmx-512-Packet.h"
#include "Dmx-512-Rx.h"
#include "Dmx-512-View.h"
#include "NixieGlyph.h"
#include "NixieTube.h"
#include "Settings.h"
#include "View.h"


namespace kbxTubeClock {


Dmx512View::Dmx512View()
  : _mode(Application::OperatingMode::OperatingModeDmx512Display)
{
}


void Dmx512View::enter()
{
  _mode = Application::getOperatingMode();

  // Application should handle this but we'll do it here, too, for consistency
  Application::setIntensityAutoAdjust(false);

  DisplayManager::setStatusLedAutoRefreshing(true);
}


bool Dmx512View::keyHandler(Keys::Key key)
{
  bool tick = false;

  if (key == Keys::Key::E)
  {
    Application::setOperatingMode(Application::OperatingMode::OperatingModeMainMenu);
    tick = true;
  }

  return tick;
}


void Dmx512View::loop()
{
  Dmx512Packet* packet = Dmx512Rx::getLastPacket();
  Settings *pSettings = Application::getSettingsPtr();
  uint16_t address = pSettings->getRawSetting(Settings::Setting::DmxAddress);

  if (Dmx512Rx::signalIsActive() == true)
  {
    if (packet->startCode() == 0)
    {
      if (pSettings->getSetting(Settings::Setting::SystemOptions, Settings::SystemOptionsBits::DmxExtended) == true)
      {
        // in this mode, fade duration (followed by tube values and intensities) starts at offset 4
        _dmxExtendedModeView(packet, address + 4);
      }
      else
      {
        // in this mode, tube values starts at offset 1
        _dmxStandardModeView(packet, address + 1);
      }
    }
  }
  else
  {
    DisplayManager::setDisplayBlanking(false);
    // Kick us back to the main menu if the signal was lost
    Application::setOperatingMode(Application::OperatingMode::OperatingModeMainMenu);
  }
}


void Dmx512View::_dmxExtendedModeView(Dmx512Packet* packet, uint16_t address)
{
  uint16_t duration = (packet->channel(address++) * cChannelMultiplier) + cChannelMultiplier;
  Display dmxDisplay;
  NixieTube currentTube(0, 0, duration);
  NixieGlyph currentGlyph(0, duration);

  // jump over the master intensity channel
  address++;

  // set tube values and intensities based on DMX-512 channel values in packet
  for (uint8_t t = 0; t < Display::cTubeCount; t++)
  {
    // we add cTubeCount + 1 to get the tube intensity channels
    uint32_t top = (packet->channel(address + t + Display::cTubeCount) * NixieGlyph::cGlyphMaximumIntensity);
    uint16_t level = static_cast<uint16_t>(top / 255);

    currentTube.setGlyph(packet->channel(address + t) / 25.6);
    currentTube.setIntensity(level);

    dmxDisplay.setTubeFromRaw(t, currentTube);
  }

  address += (Display::cTubeCount * 2); // jump over tube values and tube intensities

  // set dot/point intensities based on DMX-512 channel values in packet
  for (uint8_t d = 0; d < Display::cDotCount; d++)
  {
    // we add cTubeCount to get the tube intensity channels
    uint32_t top = (packet->channel(address++) * NixieGlyph::cGlyphMaximumIntensity);
    uint16_t level = static_cast<uint16_t>(top / 255);

    currentGlyph.setIntensity(level);

    dmxDisplay.setDot(d, currentGlyph);
  }

  DisplayManager::writeDisplay(dmxDisplay);
}


void Dmx512View::_dmxStandardModeView(Dmx512Packet* packet, uint16_t address)
{
  Display dmxDisplay;
  NixieTube currentTube;

  // set tube values and intensities based on DMX-512 channel values in packet
  for (uint8_t t = 0; t < Display::cTubeCount; t++)
  {
    currentTube.setGlyph(packet->channel(address + t) / 10);

    dmxDisplay.setTubeFromRaw(t, currentTube);
  }

  DisplayManager::writeDisplay(dmxDisplay);
}


}
