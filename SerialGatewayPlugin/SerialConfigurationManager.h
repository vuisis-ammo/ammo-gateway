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

  int getSlotDuration() { return slotDuration; }
  int getSlotNumber() { return slotNumber; }
  int getNumberOfSlots() { return numberOfSlots; }
  int getTransmitDuration() { return transmitDuration; }
  int getGpsTimeOffset() { return gpsTimeOffset; }

protected:
  void init();
  void decode(const Json::Value& root);
  
private:
  SerialConfigurationManager();
  
  static SerialConfigurationManager *sharedInstance;
  std::vector<std::string> listenPorts;
  std::string gpsPort;

  bool sendEnabled;

  int slotDuration;
  int slotNumber;
  int numberOfSlots;
  int transmitDuration;

  int gpsTimeOffset;
};

#endif //SERIAL_CONFIGURATION_MANAGER_H
