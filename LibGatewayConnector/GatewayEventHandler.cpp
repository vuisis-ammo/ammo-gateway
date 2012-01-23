#include <string>

#include "GatewayEventHandler.h"
#include "GatewayConnector.h"
#include "log.h"

using namespace std;
using namespace ammo::gateway::internal;

void GatewayEventHandler::onConnect(std::string &peerAddress) {
  LOG_TRACE("onConnect(" << peerAddress << ")");
}

void GatewayEventHandler::onDisconnect() {
  LOG_TRACE("onDisconnect()");
}

int GatewayEventHandler::onMessageAvailable(ammo::gateway::protocol::GatewayWrapper *msg) {
  LOG_TRACE("onMessageAvailable()");
  
  if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_RESULT) {
    LOG_DEBUG("Received Associate Result...");
    parent->onAssociateResultReceived(msg->associate_result());
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA) {
    LOG_DEBUG("Received Push Data...");
    parent->onPushDataReceived(msg->push_data());
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_REQUEST) {
    LOG_DEBUG("Received Pull Request...");
    parent->onPullRequestReceived(msg->pull_request());
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_RESPONSE) {
    LOG_DEBUG("Received Pull Response...");
    parent->onPullResponseReceived(msg->pull_response());
  }
  
  delete msg; //we're done with it, and we're responsible for deleting it
  
  return 0;
}

int GatewayEventHandler::onError(const char errorCode) {
  LOG_ERROR("onError(" << errorCode << ")");
  return 0;
}

void GatewayEventHandler::setParentConnector(ammo::gateway::GatewayConnector *parent) {
  this->parent = parent;
}
