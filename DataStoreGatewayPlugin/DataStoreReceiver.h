#ifndef DATA_STORE_RECEIVER_H
#define DATA_STORE_RECEIVER_H

#include "GatewayConnector.h"

#include "DataStoreDispatcher.h"

class sqlite3;

class DataStoreReceiver : public ammo::gateway::DataPushReceiverListener,
					                public ammo::gateway::GatewayConnectorDelegate,
                          public ammo::gateway::PullRequestReceiverListener

{
public:
  DataStoreReceiver (void);
  ~DataStoreReceiver (void);
	
  // GatewayConnectorDelegate methods
  virtual void onConnect (ammo::gateway::GatewayConnector *sender);
  virtual void onDisconnect (ammo::gateway::GatewayConnector *sender);
  
  // DataPushReceiverListener methods
  virtual void onPushDataReceived (ammo::gateway::GatewayConnector *sender,
							                     ammo::gateway::PushData &pushData);
	
  // PullRequestReceiverListener methods
  virtual void onPullRequestReceived (ammo::gateway::GatewayConnector *sender,
                                      ammo::gateway::PullRequest &pullReq);
                               
  void db_filepath (const std::string &path);
  bool init (void);

private:
  // Dispatcher for pushes and pulls.
  DataStoreDispatcher dispatcher_;
  
  // Pointer to open database.
  sqlite3 *db_;
  
  // Set by the config manager.
  std::string db_filepath_;
};

#endif        //  #ifndef DATA_STORE_RECEIVER_H
