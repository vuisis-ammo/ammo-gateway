#ifndef GATEWAY_SERVICE_HANDLER_H
#define GATEWAY_SERVICE_HANDLER_H

#include "ace/Svc_Handler.h"
#include "ace/SOCK_Stream.h"
#include "protocol/GatewayPrivateMessages.pb.h"
#include <vector>
#include <queue>
#include <set>
#include "Enumerations.h"
#include "GWSecurityMgr.h"
#include "GatewaySecHandler.h"

class GatewayServiceHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>, public GatewaySecHandler {
public:
  //GatewayServiceHandler(ACE_Thread_Manager *tm, ACE_Message_Queue<ACE_NULL_SYNCH> *mq, ACE_Reactor *reactor);
  virtual ~GatewayServiceHandler();
  int open(void *ptr = 0);
  int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
  int handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE);
  
  void sendData(ammo::gateway::protocol::GatewayWrapper *msg);
  ammo::gateway::protocol::GatewayWrapper *getNextMessageToSend();
  
  int processData(char *collectedData, unsigned int dataSize, unsigned int checksum);
  
  bool sendPushedData(std::string uri, std::string mimeType, const std::string &data, std::string originUser, MessageScope scope);
  bool sendPullRequest(std::string requestUid, std::string pluginId, std::string mimeType, std::string query, std::string projection,
		       unsigned int maxResults, unsigned int startFromCount, bool liveQuery);
  bool sendPullResponse(std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, const std::string& data);

  virtual void sendMessage(AuthMessage& msg);
  virtual void authenticationComplete();
  
  friend std::ostream& operator<< (std::ostream& out, const GatewayServiceHandler& handler);
  friend std::ostream& operator<< (std::ostream& out, const GatewayServiceHandler* handler);
  
protected:
  typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;
  
  typedef enum {
    READING_SIZE = 0,
    READING_CHECKSUM = 1,
    READING_DATA = 2
  } ReaderState;
  
  ReaderState state;
  unsigned int dataSize;
  unsigned int checksum;
  char *collectedData;
  unsigned int position;
  
  char *dataToSend;
  unsigned int sendPosition;
  unsigned int sendBufferSize;
  
  GWSecurityMgr *securityManager;
  
  std::string username;
  bool usernameAuthenticated;
  
  std::vector<std::string> registeredHandlers;
  std::vector<std::string> registeredPullRequestHandlers;
  
  std::queue<ammo::gateway::protocol::GatewayWrapper *> sendQueue;
  std::set<std::string> registeredPullResponsePluginIds;
  
  ammo::gateway::protocol::AuthenticationMessage_Type authMessageTypeToProtobuf(AuthMessage::Type type);
  AuthMessage::Type authMessageTypeFromProtobuf(ammo::gateway::protocol::AuthenticationMessage_Type);
  
};

#endif        //  #ifndef GATEWAY_SERVICE_HANDLER_H

