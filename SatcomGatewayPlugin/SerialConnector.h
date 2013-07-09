#ifndef SERIAL_CONNECTOR_H
#define SERIAL_CONNECTOR_H

#include <stdint.h>

#include "ace/DEV_Connector.h"
#include "ace/TTY_IO.h"
#include "ace/Copy_Disabled.h"

const uint32_t MAGIC_NUMBER = 0xabad1dea;

struct SatcomHeader {
  uint32_t magicNumber;
  uint16_t size;
  uint16_t reserved;
  uint16_t payloadChecksum;
  uint16_t headerChecksum;
}



class SerialConnector : public ACE_Copy_Disabled {
public:
  SerialConnector();
  ~SerialConnector();
  
  void run();
  void stop();
  
private:
  bool running;
  
  ACE_TTY_IO serialDevice;
  ACE_DEV_Connector serialConnector;
}

#endif //SERIAL_CONNECTOR_H