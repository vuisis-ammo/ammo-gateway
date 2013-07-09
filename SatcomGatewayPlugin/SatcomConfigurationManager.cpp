#include "SatcomConfigurationManager.h"
#include "json/value.h"
#include "log.h"
#include <sstream>

static const char* CONFIG_FILE = "SerialPluginConfig.json";

SatcomConfigurationManager &SatcomConfigurationManager::getInstance()
{
  static SatcomConfigurationManager sharedInstance;
  
  return sharedInstance;
}

SatcomConfigurationManager::SatcomConfigurationManager() : ConfigurationManager(CONFIG_FILE)
{
  init();
  populate();
}

void SatcomConfigurationManager::init()
{
#ifdef WIN32
  listenPort = "COM1";
#else
  listenPort = "/dev/ttyUSB0";
#endif

  tokenTimeout = 5000;
  dataTimeout = 5000;
}

void SatcomConfigurationManager::decode(const Json::Value& root)
{
  CM_DecodeString ( root, "listenPort", listenPort );
  CM_DecodeInt    ( root, "tokenTimeout", tokenTimeout);
  CM_DecodeInt    ( root, "dataTimeout", dataTimeout);

  LOG_INFO("Serial Plugin Configuration: ");
  LOG_INFO("  Listen Port: " << listenPort);
  LOG_INFO("  Token Timeout: " << tokenTimeout);
  LOG_INFO("  Data Timeout: " << dataTimeout);
}