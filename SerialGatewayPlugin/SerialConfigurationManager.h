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

#ifndef SERIAL_CONFIGURATION_MANAGER_H
#define SERIAL_CONFIGURATION_MANAGER_H

#include <vector>
#include "ConfigurationManager.h"

class SerialConfigurationManager : public ConfigurationManager {
public:
  static SerialConfigurationManager* getInstance();
  
  std::vector<std::string> getListenPorts() { return listenPorts; }
  std::string getGpsPort() { return gpsPort; }

  bool getSendEnabled() { return sendEnabled; }

  int getBaudRate() { return baudRate; }
  int getSlotDuration() { return slotDuration; }
  int getSlotNumber() { return slotNumber; }
  int getNumberOfSlots() { return numberOfSlots; }
  int getTransmitDuration() { return transmitDuration; }
  int getGpsTimeOffset() { return gpsTimeOffset; }

  int getPliRelayPerCycle() { return pliRelayPerCycle; }

  bool getPliRelayEnabled() { return pliRelayEnabled; }
  int getPliSendFrequency() { return pliSendFrequency; }
  std::string getPliRelayNodeName() { return pliRelayNodeName; }
  
  int getRangeScale() { return rangeScale; }
  int getTimeScale() { return timeScale; }

protected:
  void init();
  void decode(const Json::Value& root);
  
private:
  SerialConfigurationManager();
  
  static SerialConfigurationManager *sharedInstance;
  std::vector<std::string> listenPorts;
  std::string gpsPort;

  bool sendEnabled;

  int baudRate;
  int slotDuration;
  int slotNumber;
  int numberOfSlots;
  int transmitDuration;

  int gpsTimeOffset;

  int pliRelayPerCycle;

  bool pliRelayEnabled;
  int pliSendFrequency;
  std::string pliRelayNodeName;
  
  int rangeScale;
  int timeScale;
};

#endif //SERIAL_CONFIGURATION_MANAGER_H
