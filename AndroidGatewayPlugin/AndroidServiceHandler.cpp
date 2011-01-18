#include "AndroidServiceHandler.h"
#include "protocol/AmmoMessages.pb.h"

#include <iostream>

#include <log4cxx/logger.h>
#include <log4cxx/ndc.h>

#include "ace/OS_NS_errno.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

extern LoggerPtr logger;

extern std::string gatewayAddress;
extern int gatewayPort;

AndroidServiceHandler::AndroidServiceHandler() : gatewayConnector(NULL) {
  std::stringstream id;
  id << this;
  deviceId = id.str();
}

int AndroidServiceHandler::open(void *ptr) {
  if(super::open(ptr) == -1) {
    return -1;
    
  }
  state = READING_SIZE;
  dataSize = 0;
  checksum = 0;
  collectedData = NULL;
  position = 0;
  
  gatewayConnector = new GatewayConnector(this);
}

int AndroidServiceHandler::handle_input(ACE_HANDLE fd) {
  NDC::push("HandleInput");
  NDC::push(deviceId);
  //LOG4CXX_TRACE(logger, "In handle_input");
  int count = 0;
  
  if(state == READING_SIZE) {
    count = this->peer().recv_n(&dataSize, sizeof(dataSize));
    //LOG4CXX_TRACE(logger, "SIZE Read " << count << " bytes");
  } else if(state == READING_CHECKSUM) {
    count = this->peer().recv_n(&checksum, sizeof(checksum));
    //LOG4CXX_TRACE(logger, "SUM Read " << count << " bytes");
  } else if(state == READING_DATA) {
    count = this->peer().recv(collectedData + position, dataSize - position);
    //LOG4CXX_TRACE(logger, "DATA Read " << count << " bytes");
  } else {
    LOG4CXX_ERROR(logger, "Invalid state!");
  }
  
  
  
  if(count > 0) {
    if(state == READING_SIZE) {
      collectedData = new char[dataSize];
      position = 0;
      //LOG4CXX_TRACE(logger, "Got data size (" << dataSize << ")");
      state = READING_CHECKSUM;
    } else if(state == READING_CHECKSUM) {
      //LOG4CXX_TRACE(logger, "Got data checksum (" << checksum << ")");
      state = READING_DATA;
    } else if(state == READING_DATA) {
      //LOG4CXX_TRACE(logger, "Got some data...");
      position += count;
      if(position == dataSize) {
        //LOG4CXX_TRACE(logger, "Got all the data... processing");
        processData(collectedData, dataSize, checksum);
        //LOG4CXX_TRACE(logger, "Processsing complete.  Deleting buffer.");
        delete[] collectedData;
        collectedData = NULL;
        dataSize = 0;
        position = 0;
        state = READING_SIZE;
      }
    }
  } else if(count == 0) {
    LOG4CXX_INFO(logger, "Connection closed.");
    return -1;
  } else if(count == -1 && ACE_OS::last_error () != EWOULDBLOCK) {
    LOG4CXX_ERROR(logger, "Socket error occurred. (" << ACE_OS::last_error() << ")");
    return -1;
  }
  //LOG4CXX_TRACE(logger, "Leaving handle_input()");
  NDC::pop();
  NDC::pop();
  return 0;
}

void AndroidServiceHandler::sendData(ammo::protocol::MessageWrapper &msg) {
  /*Fixme: potential deadlock here
  unsigned int messageSize = msg.ByteSize();
  char *messageToSend = new char[messageSize];
  msg.SerializeToArray(messageToSend, messageSize);
  unsigned int messageChecksum = ACE::crc32(messageToSend, messageSize);
  
  ACE_Message_Block *messageSizeBlock = new ACE_Message_Block(sizeof(messageSize));
  messageSizeBlock->copy((char *) &messageSize, sizeof(messageSize));
  this->putq(messageSizeBlock);
  
  ACE_Message_Block *messageChecksumBlock = new ACE_Message_Block(sizeof(messageChecksum));
  messageChecksumBlock->copy((char *) &messageChecksum, sizeof(messageChecksum));
  this->putq(messageChecksumBlock);
  
  ACE_Message_Block *messageToSendBlock = new ACE_Message_Block(messageSize);
  messageToSendBlock->copy(messageToSend, messageSize);
  this->putq(messageToSendBlock);
  
  this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);*/
  
  unsigned int messageSize = msg.ByteSize();
  char *messageToSend = new char[messageSize];
  if(msg.IsInitialized()) {
    msg.SerializeToArray(messageToSend, messageSize);
    unsigned int messageChecksum = ACE::crc32(messageToSend, messageSize);
    
    this->peer().send_n(&messageSize, sizeof(messageSize));
    this->peer().send_n(&messageChecksum, sizeof(messageChecksum));
    this->peer().send_n(messageToSend, messageSize);
  } else {
    LOG4CXX_ERROR(logger, "SEND ERROR:  Message is missing a required element.");
  }
}

int AndroidServiceHandler::processData(char *data, unsigned int messageSize, unsigned int messageChecksum) {
  //Validate checksum
  unsigned int calculatedChecksum = ACE::crc32(data, messageSize);
  if(calculatedChecksum != messageChecksum) {
    LOG4CXX_ERROR(logger, "Invalid checksum--  we've been sent bad data (perhaps a message size mismatch?)");
    return -1;
  }
  
  //checksum is valid; parse the data
  ammo::protocol::MessageWrapper msg;
  bool result = msg.ParseFromArray(data, messageSize);
  if(result == false) {
    LOG4CXX_ERROR(logger, "MessageWrapper could not be deserialized.");
    LOG4CXX_ERROR(logger, "Client must have sent something that isn't a protocol buffer (or the wrong type).");
    return -1;
  }
  LOG4CXX_TRACE(logger, "Message Received: " << msg.DebugString());
  
  if(msg.type() == ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_MESSAGE) {
    LOG4CXX_DEBUG(logger, "Received Authentication Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::AuthenticationMessage authMessage = msg.authentication_message();
      gatewayConnector->associateDevice(authMessage.device_id(), authMessage.user_id(), authMessage.user_key());
      deviceId = authMessage.device_id();
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_DATA_MESSAGE) {
    LOG4CXX_DEBUG(logger, "Received Data Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::DataMessage dataMessage = msg.data_message();
      gatewayConnector->pushData(dataMessage.uri(), dataMessage.mime_type(), dataMessage.data());
      ammo::protocol::MessageWrapper ackMsg;
      ammo::protocol::PushAcknowledgement *ack = ackMsg.mutable_push_acknowledgement();
      ack->set_uri(dataMessage.uri());
      ackMsg.set_type(ammo::protocol::MessageWrapper_MessageType_PUSH_ACKNOWLEDGEMENT);
      LOG4CXX_DEBUG(logger, "Sending push acknowledgement to connected device...");
      this->sendData(ackMsg);
      
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_SUBSCRIBE_MESSAGE) {
    LOG4CXX_DEBUG(logger, "Received Subscribe Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::SubscribeMessage subscribeMessage = msg.subscribe_message();
      gatewayConnector->registerDataInterest(subscribeMessage.mime_type(), this);
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_PULL_REQUEST) {
    LOG4CXX_DEBUG(logger, "Received Pull Request Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::PullRequest pullRequest = msg.pull_request();
      // register for pull response - 
      gatewayConnector->registerPullResponseInterest(pullRequest.mime_type(), this);
      // now send request
      gatewayConnector->pullRequest( pullRequest.request_uid(), pullRequest.plugin_id(), pullRequest.mime_type(), pullRequest.query(),
				     pullRequest.projection(), pullRequest.max_results(), pullRequest.start_from_count(), pullRequest.live_query() );

    }
  }
  

  return 0;
}

void AndroidServiceHandler::onConnect(GatewayConnector *sender) {
}

void AndroidServiceHandler::onDisconnect(GatewayConnector *sender) {
  
}

void AndroidServiceHandler::onDataReceived(GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data, std::string originUser) {
  NDC::push("PushCallback");
  NDC::push(deviceId);
  
  LOG4CXX_DEBUG(logger, "Sending subscribed data to device...");
  LOG4CXX_DEBUG(logger, "   URI: " << uri << ", Type: " << mimeType);
  
  std::string dataString(data.begin(), data.end());
  ammo::protocol::MessageWrapper msg;
  ammo::protocol::DataMessage *dataMsg = msg.mutable_data_message();
  dataMsg->set_uri(uri);
  dataMsg->set_mime_type(mimeType);
  dataMsg->set_data(dataString);
  
  msg.set_type(ammo::protocol::MessageWrapper_MessageType_DATA_MESSAGE);
  
  LOG4CXX_DEBUG(logger, "Sending Data Push message to connected device");
  this->sendData(msg);
  
  NDC::pop();
  NDC::pop();
}

void AndroidServiceHandler::onDataReceived(GatewayConnector *sender, std::string requestUid, std::string pluginId, std::string mimeType, std::string uri, std::vector<char> &data) {
  NDC::push("PullResponseCallback");
  NDC::push(deviceId);
  
  LOG4CXX_DEBUG(logger, "Sending pull response to device...");
  LOG4CXX_DEBUG(logger, "   URI: " << uri << ", Type: " << mimeType);
  
  std::string dataString(data.begin(), data.end());
  ammo::protocol::MessageWrapper msg;
  ammo::protocol::PullResponse *pullMsg = msg.mutable_pull_response();

  pullMsg->set_request_uid(requestUid);
  pullMsg->set_plugin_id(pluginId);
  pullMsg->set_mime_type(mimeType);
  pullMsg->set_uri(uri);
  pullMsg->set_data(dataString);
  
  msg.set_type(ammo::protocol::MessageWrapper_MessageType_PULL_RESPONSE);
  
  LOG4CXX_DEBUG(logger, "Sending Pull Response message to connected device");
  this->sendData(msg);
}



void AndroidServiceHandler::onAuthenticationResponse(GatewayConnector *sender, bool result) {
  NDC::push("AuthCallback");
  NDC::push(deviceId);
  
  LOG4CXX_DEBUG(logger, "Delegate: onAuthenticationResponse");
  ammo::protocol::MessageWrapper newMsg;
  newMsg.set_type(ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_RESULT);
  newMsg.mutable_authentication_result()->set_result(result ? ammo::protocol::AuthenticationResult_Status_SUCCESS : ammo::protocol::AuthenticationResult_Status_SUCCESS);
  this->sendData(newMsg);
}

AndroidServiceHandler::~AndroidServiceHandler() {
  if(gatewayConnector) {
    delete gatewayConnector;
  }
}
