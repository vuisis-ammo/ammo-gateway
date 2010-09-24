#include "SamplePushReceiver.h"

#include <iostream>
#include <string>

using namespace std;

void SamplePushReceiver::onConnect(GatewayConnector *sender) {
  
}

void SamplePushReceiver::onDisconnect(GatewayConnector *sender) {
  
}

void SamplePushReceiver::onDataReceived(GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data) {
  cout << "Got data." << endl;
  cout << "  URI: " << uri << endl;
  cout << "  Mime type: " << mimeType << endl;
  cout << "  Data: " << string(data.begin(), data.end()) << endl;
}