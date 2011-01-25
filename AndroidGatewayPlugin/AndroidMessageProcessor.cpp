#include "AndroidMessageProcessor.h"
#include "AndroidServiceHandler.h"

#include "log.h"

AndroidMessageProcessor::AndroidMessageProcessor(AndroidServiceHandler *serviceHandler) :
closed(false),
closeMutex(),
newMessageMutex(),
newMessageAvailable(newMessageMutex),
commsHandler(serviceHandler),
gatewayConnector(NULL)
{
  //need to initialize GatewayConnector in the main thread; the constructor always
  //happens in the main thread
  gatewayConnector = new GatewayConnector(this);
}

AndroidMessageProcessor::~AndroidMessageProcessor() {
  LOG_TRACE("In ~AndroidMessageProcessor()");
  if(gatewayConnector) {
    delete gatewayConnector;
  }
}

int AndroidMessageProcessor::open(void *args) {
  closed = false;
  return 0;
}

int AndroidMessageProcessor::close(unsigned long flags) {
  LOG_TRACE("Closing MessageProcessor (in AndroidMessageProcessor.close())");
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
    newMessageMutex.acquire();
    newMessageAvailable.wait();
    newMessageMutex.release();
    
    ammo::protocol::MessageWrapper *msg = commsHandler->getNextReceivedMessage();
    if(msg) {
      processMessage(*msg);
      delete msg;
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
  LOG_TRACE("Message Received: " << msg.DebugString());
  
  if(msg.type() == ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_MESSAGE) {
    LOG_DEBUG("Received Authentication Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::AuthenticationMessage authMessage = msg.authentication_message();
      gatewayConnector->associateDevice(authMessage.device_id(), authMessage.user_id(), authMessage.user_key());
      //deviceId = authMessage.device_id();
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_DATA_MESSAGE) {
    LOG_DEBUG("Received Data Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::DataMessage dataMessage = msg.data_message();
      gatewayConnector->pushData(dataMessage.uri(), dataMessage.mime_type(), dataMessage.data());
      ammo::protocol::MessageWrapper *ackMsg = new ammo::protocol::MessageWrapper();
      ammo::protocol::PushAcknowledgement *ack = ackMsg->mutable_push_acknowledgement();
      ack->set_uri(dataMessage.uri());
      ackMsg->set_type(ammo::protocol::MessageWrapper_MessageType_PUSH_ACKNOWLEDGEMENT);
      LOG_DEBUG("Sending push acknowledgement to connected device...");
      commsHandler->sendMessage(ackMsg);
      
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_SUBSCRIBE_MESSAGE) {
    LOG_DEBUG("Received Subscribe Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::SubscribeMessage subscribeMessage = msg.subscribe_message();
      gatewayConnector->registerDataInterest(subscribeMessage.mime_type(), this);
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_PULL_REQUEST) {
    LOG_DEBUG("Received Pull Request Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::PullRequest pullRequest = msg.pull_request();
      // register for pull response - 
      gatewayConnector->registerPullResponseInterest(pullRequest.mime_type(), this);
      // now send request
      gatewayConnector->pullRequest( pullRequest.request_uid(), pullRequest.plugin_id(), pullRequest.mime_type(), pullRequest.query(),
				     pullRequest.projection(), pullRequest.max_results(), pullRequest.start_from_count(), pullRequest.live_query() );

    }
  }
}


void AndroidMessageProcessor::onConnect(GatewayConnector *sender) {
}

void AndroidMessageProcessor::onDisconnect(GatewayConnector *sender) {
  
}

void AndroidMessageProcessor::onDataReceived(GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data, std::string originUser) {
  LOG_DEBUG("Sending subscribed data to device...");
  LOG_DEBUG("   URI: " << uri << ", Type: " << mimeType);
  
  std::string dataString(data.begin(), data.end());
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper;
  ammo::protocol::DataMessage *dataMsg = msg->mutable_data_message();
  dataMsg->set_uri(uri);
  dataMsg->set_mime_type(mimeType);
  dataMsg->set_data(dataString);
  
  msg->set_type(ammo::protocol::MessageWrapper_MessageType_DATA_MESSAGE);
  
  LOG_DEBUG("Sending Data Push message to connected device");
  commsHandler->sendMessage(msg);
}

void AndroidMessageProcessor::onDataReceived(GatewayConnector *sender, std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, std::vector<char> &data) {
  LOG_DEBUG("Sending pull response to device...");
  LOG_DEBUG("   URI: " << uri << ", Type: " << mimeType);
  
  std::string dataString(data.begin(), data.end());
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper();
  ammo::protocol::PullResponse *pullMsg = msg->mutable_pull_response();

  pullMsg->set_request_uid(requestUid);
  pullMsg->set_plugin_id(pluginId);
  pullMsg->set_mime_type(mimeType);
  pullMsg->set_uri(uri);
  pullMsg->set_data(dataString);
  
  msg->set_type(ammo::protocol::MessageWrapper_MessageType_PULL_RESPONSE);
  
  LOG_DEBUG("Sending Pull Response message to connected device");
  commsHandler->sendMessage(msg);
}



void AndroidMessageProcessor::onAuthenticationResponse(GatewayConnector *sender, bool result) {
  LOG_DEBUG("Delegate: onAuthenticationResponse");
  ammo::protocol::MessageWrapper *newMsg = new ammo::protocol::MessageWrapper();
  newMsg->set_type(ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_RESULT);
  newMsg->mutable_authentication_result()->set_result(result ? ammo::protocol::AuthenticationResult_Status_SUCCESS : ammo::protocol::AuthenticationResult_Status_SUCCESS);
  commsHandler->sendMessage(newMsg);
}