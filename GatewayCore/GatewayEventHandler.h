/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef GATEWAY_EVENT_HANDLER_H
#define GATEWAY_EVENT_HANDLER_H

#include <string>
#include <set>
#include <queue>

#include "NetworkEventHandler.h"
#include "protocol/GatewayPrivateMessages.pb.h"
#include "Enumerations.h"

class GatewayEventHandler : public ammo::gateway::internal::NetworkEventHandler<ammo::gateway::protocol::GatewayWrapper, ammo::gateway::internal::SYNC_MULTITHREADED, 0xdeadbeef> {
public:
  virtual ~GatewayEventHandler();
  
  virtual void onConnect(std::string &peerAddress);
  virtual void onDisconnect();
  virtual int onMessageAvailable(ammo::gateway::protocol::GatewayWrapper *msg);
  virtual int onError(const char errorCode);
  
  bool sendPushedData(std::string uri, std::string mimeType, std::string encoding, const std::string &data, std::string originUser, std::string originDevice, MessageScope scope, bool ackDeviceDelivered, bool ackPluginDelivered, char priority);
  bool sendPullRequest(std::string requestUid, std::string pluginId, std::string mimeType, std::string query, std::string projection,
		       unsigned int maxResults, unsigned int startFromCount, bool liveQuery, char priority);

  bool sendPullResponse(std::string requestUid, std::string pluginId, std::string mimeType, std::string uid, std::string encoding, const std::string& data, char priority);
  
  bool sendPushAcknowledgement(std::string uid, std::string destinationDevice, std::string acknowledgingDevice, std::string destinationUser, std::string acknowledgingUser, bool deviceDelivered, bool pluginDelivered, PushStatus status);
  
private:
  std::string username;
  bool usernameAuthenticated;
  
  std::vector<std::string> registeredHandlers;
  std::vector<std::string> registeredPullRequestHandlers;
  std::set<std::string> registeredPullResponsePluginIds;
};

#endif
