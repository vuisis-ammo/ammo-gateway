/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "SerialConfigurationManager.h"
#include "json/value.h"
#include "log.h"
#include <sstream>

static const char* CONFIG_FILE = "SerialPluginConfig.json";

SerialConfigurationManager* SerialConfigurationManager::sharedInstance = NULL;

SerialConfigurationManager* SerialConfigurationManager::getInstance()
{
  if (!sharedInstance) {
    sharedInstance = new SerialConfigurationManager();
	sharedInstance->populate();
  }

  return sharedInstance;
}

SerialConfigurationManager::SerialConfigurationManager() : ConfigurationManager(CONFIG_FILE)
{
  init();
}

void SerialConfigurationManager::init()
{
  std::string listenPort;

#ifdef WIN32
  listenPort = "COM1";
#else
  listenPort = "/dev/ttyUSB0";
#endif

  listenPorts.clear();
  listenPorts.push_back(listenPort);

#ifdef WIN32
  gpsPort = "COM2";
#else
  gpsPort = "/dev/ttyUSB1";
#endif

  sendEnabled = false;

  baudRate = 9600;

  slotDuration = 750;
  slotNumber = 1;
  numberOfSlots = 16;
  transmitDuration = 500;
  gpsTimeOffset = 0;

  pliRelayPerCycle = 4;

  pliRelayEnabled = true;
  pliSendFrequency = 2; //every other slot
  pliRelayNodeName = "base";
  
  rangeScale = 4;
  timeScale = 4;
}

void SerialConfigurationManager::decode(const Json::Value& root)
{
  std::string listenPort;

  listenPorts.clear();

  CM_DecodeString ( root, "listenPort", listenPort );
  listenPorts.push_back(listenPort);
  CM_DecodeString ( root, "gpsPort", gpsPort);
  CM_DecodeBool   ( root, "sendEnabled", sendEnabled);
  CM_DecodeInt    ( root, "baudRate", baudRate);
  CM_DecodeInt    ( root, "slotDuration", slotDuration);
  CM_DecodeInt    ( root, "slotNumber", slotNumber);
  CM_DecodeInt    ( root, "numberOfSlots", numberOfSlots);
  CM_DecodeInt    ( root, "transmitDuration", transmitDuration);
  CM_DecodeInt    ( root, "gpsTimeOffset", gpsTimeOffset);

  CM_DecodeInt    ( root, "pliRelayPerCycle", pliRelayPerCycle);

  CM_DecodeBool   ( root, "pliRelayEnabled", pliRelayEnabled);
  CM_DecodeInt    ( root, "pliSendFrequency", pliSendFrequency);
  CM_DecodeString ( root, "pliRelayNodeName", pliRelayNodeName);
  CM_DecodeInt    ( root, "rangeScale", rangeScale);
  CM_DecodeInt    ( root, "timeScale", timeScale);

  bool findMorePorts = true;
  for (int i = 1; findMorePorts; i++) {
    std::stringstream ss;
    ss << "listenPort" << i;
    std::string portKey = ss.str();

	if(!root[portKey].isString()) {
      findMorePorts = false;
      continue;
	}

    listenPort = root[portKey].asString();
	listenPorts.push_back(listenPort);
  }

  LOG_INFO("Serial Plugin Configuration: ");
  LOG_INFO("  Listen Port: " << listenPorts[0]);
  for (int i = 1; i < listenPorts.size(); i++) {
    std::stringstream ss;
    ss << "listenPort" << i;
    std::string portKey = ss.str();
    LOG_INFO("  Listen Port " << i << ": " << listenPorts[i]);
  }
  LOG_INFO("  GPS Port: " << gpsPort);
  LOG_INFO("  Send Enabled: " << sendEnabled);
  LOG_INFO("  Baud Rate: " << baudRate);
  LOG_INFO("  Slot Duration: " << slotDuration);
  LOG_INFO("  Slot Number: " << slotNumber);
  LOG_INFO("  Number Of Slots: " << numberOfSlots);
  LOG_INFO("  Transmit Duration: " << transmitDuration);
  LOG_INFO("  GPS Time Offset: " << gpsTimeOffset);
  LOG_INFO("  Pli Relay Per Cycle: " << pliRelayPerCycle);
  LOG_INFO("  Pli Relay Enabled: " << pliRelayEnabled);
  LOG_INFO("  Pli Send Frequency: " << pliSendFrequency);
  LOG_INFO("  Pli Relay Node Name: " << pliRelayNodeName);
  LOG_INFO("  Range Scale: " << rangeScale);
  LOG_INFO("  Time Scale: " << timeScale);
}
