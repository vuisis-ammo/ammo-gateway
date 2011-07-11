#include "AndroidMessageProcessor.h"
#include "AndroidServiceHandler.h"

#include "log.h"

using namespace ammo::gateway;

AndroidMessageProcessor::AndroidMessageProcessor(AndroidServiceHandler *serviceHandler) :
closed(false),
closeMutex(),
newMessageMutex(),
newMessageAvailable(newMessageMutex),
commsHandler(serviceHandler),
gatewayConnector(NULL),
deviceId(""),
deviceIdAuthenticated(false)
{
  //need to initialize GatewayConnector in the main thread; the constructor always
  //happens in the main thread
  gatewayConnector = new GatewayConnector(this);
}

AndroidMessageProcessor::~AndroidMessageProcessor() {
  LOG_TRACE(commsHandler << " In ~AndroidMessageProcessor()");
  if(gatewayConnector) {
    delete gatewayConnector;
  }


}

int AndroidMessageProcessor::open(void *args) {
  closed = false;
  return 0;
}

int AndroidMessageProcessor::close(unsigned long flags) {
  LOG_TRACE(commsHandler << " Closing MessageProcessor (in AndroidMessageProcessor.close())");
  closeMutex.acquire();
  closed = true;
  closeMutex.release();
  
  signalNewMessageAvailable();
  return 0;
}

bool AndroidMessageProcessor::isClosed() {
  volatile bool ret; //does this need to be volatile to keep the compiler from optimizing it out?

  closeMutex.acquire();
  ret = closed;
  closeMutex.release();
  
  return ret;
}

int AndroidMessageProcessor::svc() {
  while(!isClosed()) {    
    ammo::protocol::MessageWrapper *msg = NULL;
    
    do {
      msg = commsHandler->getNextReceivedMessage();
      if(msg) {
        processMessage(*msg);
        delete msg;
      }
    } while (msg != NULL);
    if(!isClosed()) {
      newMessageMutex.acquire();
      newMessageAvailable.wait();
      newMessageMutex.release();
    }
  }
  return 0;
}

void AndroidMessageProcessor::signalNewMessageAvailable() {
  newMessageMutex.acquire();
  newMessageAvailable.signal();
  newMessageMutex.release();
}

void AndroidMessageProcessor::processMessage(ammo::protocol::MessageWrapper &msg) {
  LOG_TRACE(commsHandler << " Message Received: " << msg.DebugString());
  
  if(msg.type() == ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_MESSAGE) {
    LOG_DEBUG(commsHandler << " Received Authentication Message...");
    ammo::protocol::AuthenticationMessage authMessage = msg.authentication_message();
    
    if(gatewayConnector != NULL) {
      gatewayConnector->sendAuthenticationMessage(authMessageTypeFromProtobuf(authMessage.type()), authMessage.message(), authMessage.device_id(), authMessage.user_id());
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_DATA_MESSAGE) {
    LOG_DEBUG(commsHandler << " Received Data Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::DataMessage dataMessage = msg.data_message();
      MessageScope scope;
      if(dataMessage.scope() == ammo::protocol::LOCAL) {
        scope = SCOPE_LOCAL;
      } else {
        scope = SCOPE_GLOBAL;
      }
      
      PushData pushData;
      pushData.uri = dataMessage.uri();
      pushData.mimeType = dataMessage.mime_type();
      pushData.data = dataMessage.data();
      pushData.scope = scope;
      gatewayConnector->pushData(pushData);
      ammo::protocol::MessageWrapper *ackMsg = new ammo::protocol::MessageWrapper();
      ammo::protocol::PushAcknowledgement *ack = ackMsg->mutable_push_acknowledgement();
      ack->set_uri(dataMessage.uri());
      ackMsg->set_type(ammo::protocol::MessageWrapper_MessageType_PUSH_ACKNOWLEDGEMENT);
      LOG_DEBUG(commsHandler << " Sending push acknowledgment to connected device...");
      commsHandler->sendMessage(ackMsg);
      
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_SUBSCRIBE_MESSAGE) {
    LOG_DEBUG(commsHandler << " Received Subscribe Message...");
    MessageScope scope;
    if(msg.subscribe_message().scope() == ammo::protocol::LOCAL) {
      scope = SCOPE_LOCAL;
    } else {
      scope = SCOPE_GLOBAL;
    }
    
    if(gatewayConnector != NULL) {
      ammo::protocol::SubscribeMessage subscribeMessage = msg.subscribe_message();
      
      gatewayConnector->registerDataInterest(subscribeMessage.mime_type(), this, scope);
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_UNSUBSCRIBE_MESSAGE) {
    LOG_DEBUG(commsHandler << " Received Unubscribe Message...");
    MessageScope scope;
    if(msg.unsubscribe_message().scope() == ammo::protocol::LOCAL) {
      scope = SCOPE_LOCAL;
    } else {
      scope = SCOPE_GLOBAL;
    }
    
    if(gatewayConnector != NULL) {
      ammo::protocol::UnsubscribeMessage unsubscribeMessage = msg.unsubscribe_message();
      
      gatewayConnector->unregisterDataInterest(unsubscribeMessage.mime_type(), scope);
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_PULL_REQUEST) {
    LOG_DEBUG(commsHandler << " Received Pull Request Message...");
    if(gatewayConnector != NULL && deviceIdAuthenticated) {
      ammo::protocol::PullRequest pullRequest = msg.pull_request();
      // register for pull response - 
      gatewayConnector->registerPullResponseInterest(pullRequest.mime_type(), this);
      // now send request
      PullRequest req;
      req.requestUid = pullRequest.request_uid();
      req.pluginId = this->deviceId;
      req.mimeType = pullRequest.mime_type();
      req.query = pullRequest.query();
      req.projection = pullRequest.projection();
      req.maxResults = pullRequest.max_results();
      req.startFromCount = pullRequest.start_from_count();
      req.liveQuery = pullRequest.live_query();
      gatewayConnector->pullRequest(req);
    } else {
      if(!deviceIdAuthenticated) {
        LOG_ERROR(commsHandler << " Attempted to send a pull request before authentication.");
      }
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_HEARTBEAT) {
    LOG_DEBUG(commsHandler << " Received Heartbeat from device...");
    ammo::protocol::Heartbeat heartbeat = msg.heartbeat();
    
    ammo::protocol::MessageWrapper *heartbeatAck = new ammo::protocol::MessageWrapper();
    ammo::protocol::Heartbeat *ack = heartbeatAck->mutable_heartbeat();
    ack->set_sequence_number(heartbeat.sequence_number());
    heartbeatAck->set_type(ammo::protocol::MessageWrapper_MessageType_HEARTBEAT);
    
    LOG_DEBUG(commsHandler << " Sending heartbeat acknowledgement to connected device...");
    commsHandler->sendMessage(heartbeatAck);
  }
}

void AndroidMessageProcessor::onConnect(GatewayConnector *sender) {
}

void AndroidMessageProcessor::onDisconnect(GatewayConnector *sender) {
  
}

void AndroidMessageProcessor::onPushDataReceived(GatewayConnector *sender, ammo::gateway::PushData &pushData) {
  LOG_DEBUG(commsHandler << " Sending subscribed data to device...");
  LOG_DEBUG(commsHandler << "    " << pushData);
  
  std::string dataString(pushData.data.begin(), pushData.data.end());
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper;
  ammo::protocol::DataMessage *dataMsg = msg->mutable_data_message();
  dataMsg->set_uri(pushData.uri);
  dataMsg->set_mime_type(pushData.mimeType);
  dataMsg->set_data(dataString);
  
  msg->set_type(ammo::protocol::MessageWrapper_MessageType_DATA_MESSAGE);
  
  LOG_DEBUG(commsHandler << " Sending Data Push message to connected device");
  commsHandler->sendMessage(msg);
}

void AndroidMessageProcessor::onPullResponseReceived(GatewayConnector *sender, ammo::gateway::PullResponse &response) {
  LOG_DEBUG(commsHandler << " Sending pull response to device...");
  LOG_DEBUG(commsHandler << "    URI: " << response.uri << ", Type: " << response.mimeType);

  std::string dataString(response.data.begin(), response.data.end());
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper();
  ammo::protocol::PullResponse *pullMsg = msg->mutable_pull_response();

  pullMsg->set_request_uid(response.requestUid);
  pullMsg->set_plugin_id(response.pluginId);
  pullMsg->set_mime_type(response.mimeType);
  pullMsg->set_uri(response.uri);
  pullMsg->set_data(dataString);
  
  msg->set_type(ammo::protocol::MessageWrapper_MessageType_PULL_RESPONSE);
  
  LOG_DEBUG(commsHandler << " Sending Pull Response message to connected device");
  commsHandler->sendMessage(msg);
}



void AndroidMessageProcessor::onAuthenticationResponse(GatewayConnector *sender, ammo::gateway::AuthenticationMessageType type, std::string message, std::string deviceId, std::string userId, bool authResult) {
  LOG_DEBUG(commsHandler << " Delegate: onAuthenticationResponse");
  if(authResult == true && type == ammo::gateway::SERVER_FINISH) {
    deviceIdAuthenticated = true;
  }
  
  ammo::protocol::MessageWrapper *newMsg = new ammo::protocol::MessageWrapper();
  newMsg->set_type(ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_MESSAGE);
  ammo::protocol::AuthenticationMessage *authMsg = newMsg->mutable_authentication_message();
  authMsg->set_type(authMessageTypeToProtobuf(type));
  authMsg->set_message(message);
  authMsg->set_device_id(deviceId);
  authMsg->set_user_id(userId);
  if(authResult == true) {
    authMsg->set_result(ammo::protocol::AuthenticationMessage_Status_SUCCESS);
  } else {
    authMsg->set_result(ammo::protocol::AuthenticationMessage_Status_FAILED);
  }
  
  commsHandler->sendMessage(newMsg);
}

ammo::protocol::AuthenticationMessage_Type AndroidMessageProcessor::authMessageTypeToProtobuf(ammo::gateway::AuthenticationMessageType type) {
  ammo::protocol::AuthenticationMessage_Type result = ammo::protocol::AuthenticationMessage_Type_STATUS;
  switch(type) {
  case ammo::gateway::CLIENT_NONCE:
    result = ammo::protocol::AuthenticationMessage_Type_CLIENT_NONCE;
    break;
  case ammo::gateway::SERVER_NONCE:
    result = ammo::protocol::AuthenticationMessage_Type_SERVER_NONCE;
    break;
  case ammo::gateway::CLIENT_KEYXCHANGE:
    result = ammo::protocol::AuthenticationMessage_Type_CLIENT_KEYXCHANGE;
    break;
  case ammo::gateway::CLIENT_PHNAUTH:
    result = ammo::protocol::AuthenticationMessage_Type_CLIENT_PHNAUTH;
    break;
  case ammo::gateway::CLIENT_FINISH:
    result = ammo::protocol::AuthenticationMessage_Type_CLIENT_FINISH;
    break;
  case ammo::gateway::SERVER_FINISH:
    result = ammo::protocol::AuthenticationMessage_Type_SERVER_FINISH;
    break;
  case ammo::gateway::STATUS:
    result = ammo::protocol::AuthenticationMessage_Type_STATUS;
    break;
  }
  return result;
}


ammo::gateway::AuthenticationMessageType AndroidMessageProcessor::authMessageTypeFromProtobuf(ammo::protocol::AuthenticationMessage_Type type) {
  ammo::gateway::AuthenticationMessageType result = ammo::gateway::STATUS;
  switch(type) {
  case ammo::protocol::AuthenticationMessage_Type_CLIENT_NONCE:
    result = ammo::gateway::CLIENT_NONCE;
    break;
  case ammo::protocol::AuthenticationMessage_Type_SERVER_NONCE:
    result = ammo::gateway::SERVER_NONCE;
    break;
  case ammo::protocol::AuthenticationMessage_Type_CLIENT_KEYXCHANGE:
    result = ammo::gateway::CLIENT_KEYXCHANGE;
    break;
  case ammo::protocol::AuthenticationMessage_Type_CLIENT_PHNAUTH:
    result = ammo::gateway::CLIENT_PHNAUTH;
    break;
  case ammo::protocol::AuthenticationMessage_Type_CLIENT_FINISH:
    result = ammo::gateway::CLIENT_FINISH;
    break;
  case ammo::protocol::AuthenticationMessage_Type_SERVER_FINISH:
    result = ammo::gateway::SERVER_FINISH;
    break;
  case ammo::protocol::AuthenticationMessage_Type_STATUS:
    result = ammo::gateway::STATUS;
    break;
  }
  return result;
}
