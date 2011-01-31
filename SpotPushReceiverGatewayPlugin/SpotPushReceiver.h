#ifndef SPOT_PUSH_RECEIVER_H
#define SPOT_PUSH_RECEIVER_H

#include "GatewayConnector.h"



class SpotPushReceiver : public DataPushReceiverListener, public GatewayConnectorDelegate {
public:
  //GatewayConnectorDelegate methods
  virtual void onConnect(GatewayConnector *sender);
  virtual void onDisconnect(GatewayConnector *sender);
  
  //DataPushReceiverListener methods
  virtual void onDataReceived(GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data, std::string originUser);


  //PullRequestReceiverListener methods
  virtual void onDataReceived(GatewayConnector *sender, 
			      std::string requestUid, std::string pluginId,
			      std::string mimeType, std::string query,
			      std::string projection, unsigned int maxResults,
			      unsigned int startFromCount, bool liveQuery);

};

class SpotReport {
public:
  std::string content_guid;
  long report_time;
  std::string reporting_unit;
  int size;
  std::string activity;
  std::string location_utm;
  std::string enemy_unit;
  long observation_time;
  std::string unit;
  std::string equipment;
  std::string assessment;
  std::string narrative;
  std::string authentication;

};


#endif        //  #ifndef SPOT_PUSH_RECEIVER_H

