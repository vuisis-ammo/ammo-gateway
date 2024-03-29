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

/**
* @mainpage
* 
* This library provides the interface from a gateway plugin to the core gateway.
* Client plugins should create at least one GatewayConnector object, then
* subclass GatewayConnectorDelegate and any listener classes (i.e. 
* DataPushReceiverListener) that they need.
*/

#ifndef GATEWAY_CONNECTOR_H
#define GATEWAY_CONNECTOR_H

#include <string>
#include <vector>
#include <map>

#include "NetworkConnector.h"
#include "NetworkEnumerations.h"

#include "LibGatewayConnector_Export.h"
#include "protocol/GatewayPrivateMessages.pb.h"

#include "Enumerations.h"

namespace ammo {
  namespace gateway {
    class GatewayConnectorDelegate;
    class DataPushReceiverListener;
    class PullRequestReceiverListener;
    class PullResponseReceiverListener;
    namespace internal {
      class GatewayConfigurationManager;
      class GatewayEventHandler;
    };
    namespace protocol {
      class AssociateResult;
      class PushData;
      class PullRequest;
      class PullResponse;
    };
    
    struct LibGatewayConnector_Export AcknowledgementThresholds {
    public:
      AcknowledgementThresholds() : deviceDelivered(false), pluginDelivered(false) {
        //do nothing
      };
      
      bool deviceDelivered;
      bool pluginDelivered;
    };
    
    /**
     * A data object.
     */
    class LibGatewayConnector_Export PushData {
    public:
      PushData();
      std::string uri;                  ///< The URI of this piece of data.  This URI should be a universally
                                        ///  unique identifier for the object being pushed (no two pieces of
                                        ///  data should have the same URI).
      std::string mimeType;             ///< The MIME type of this piece of data.  This MIME type is used to
                                        ///  determine which other gateway plugins will receive this pushed
                                        ///  data.
      std::string encoding;             ///< The encoding of the data in this message (optional; defaults
                                        ///  to "json" if not specified).
      std::string data;                 ///< The data to be pushed to the gateway.  Represented as a C++ string,
                                        ///  but can contain any arbitrary binary data (the gateway will 
                                        ///  accept strings with bytes invalid in C-strings such as embedded
                                        ///  nulls).
      std::string originUsername;       ///< The username of the user who generated this data.  May be
                                        ///  overwritten by the gateway in some cases.  Optional.
      std::string originDevice;         ///< A unique identifier for this plugin.  Used for acknowledgement
                                        ///  routing; must be unique or acknowledgements will be misrouted.
      ammo::gateway::MessageScope scope;///< The scope of this object (determines how many gateways to send
                                        ///  this object to in a multiple gateway configuration).  Optional,
                                        ///  will default to SCOPE_GLOBAL.
      char priority;                    ///< The priority of this object.  Objects with a higher priority
                                        ///  will be pushed to the device before objects with a lower priority,
                                        ///  if messages are queued.
      ammo::gateway::AcknowledgementThresholds ackThresholds;
      
      friend std::ostream& operator<<(std::ostream &os, const ammo::gateway::PushData &pushData) {
        os << "URI: " << pushData.uri << " type: " << pushData.mimeType;
        return os;
      }
    };
    
    class LibGatewayConnector_Export PushAcknowledgement {
    public:
      PushAcknowledgement();
      std::string uid;                  ///< The unique identifier of the message being acknowledged.  In the
                                        ///  current version of the API, this field corresponds to the "uri"
                                        ///  parameter in PushData.
      std::string destinationDevice;    ///< The device to send the acknowledgement to.
      std::string acknowledgingDevice;  ///< The device sending the acknowledgement.
      std::string destinationUser;      ///< The username of the user to send the acknowledgement to.
      std::string acknowledgingUser;    ///< The username of the user sending the acknowledgement.
                                        
      bool deviceDelivered;             ///< If true, indicates that this acknowledgement was sent in response
                                        ///  to delivery to a device.
      bool pluginDelivered;             ///< If true, indicates that this acknowledgement was sent in response
                                        ///  to delivery to a device.
                                        
      ammo::gateway::PushStatus status; ///< The status of the message being acknowledged.
    };
    
    /**
     * A request for data from a plugin.
     */
    class LibGatewayConnector_Export PullRequest {
    public:
      PullRequest();
      std::string requestUid;            ///< A unique identifier for this pull request.  It will be returned
                                         ///  with each item returned by the pull request.
      std::string pluginId;              ///< The unique identifier for this plugin or connected device.  For
                                         ///  devices, this should be the same as the device ID used by
                                         ///  associateDevice.
      std::string mimeType;              ///< The data type to request.  Used to determine which plugin or
                                         ///  plugins a request is routed to.
      std::string query;                 ///< The query for this pull request.  Its format is defined by the
                                         ///  plugin handling the request.
      std::string projection;            ///< An application-specific transformation to be applied to the results.  Optional.
      unsigned int maxResults;           ///< The maxiumum number of results to return from this pull request.  Optional.
      unsigned int startFromCount;       ///< An offset specifying the result to begin returning from (when a
                                         ///  subset of results is returned).  Optional.
      bool liveQuery;                    ///< Specifies a live query--  results are returned continously as they
                                         ///  become available.  The exact behavior of this option is defined
                                         ///  by the plugin handling the request.  Optional.
      ammo::gateway::MessageScope scope; ///< The scope of this object (determines how many gateways to send
                                         ///  this object to in a multiple gateway configuration).  Optional,
                                         ///  will default to SCOPE_LOCAL.
	    char priority;                     ///< Priority of this pull request.  Requests with higher priority
	                                       ///  values will be sent first if multiple messages are queued.
      
      friend std::ostream& operator<<(std::ostream &os, const ammo::gateway::PullRequest &pullReq) {
        os << "Pull << " << pullReq.requestUid << " from " << pullReq.pluginId << " for type " << pullReq.mimeType << " query: " << pullReq.query;
        return os;
      }
    };
    
    /**
     * A response to a pull request.
     */
    class LibGatewayConnector_Export PullResponse {
    public:
      PullResponse();
      std::string requestUid; ///< The unique identifier for this pull request, as specified
                              ///  in the original request.
      std::string pluginId;   ///< The unique identifier of the device to send the response to.
                              ///  Must match the identifier from the initial request or data
                              ///  will not be routed correctly.
      std::string mimeType;   ///< The data type of the data in this response.
      std::string uri;        ///< The URI of the data in this response.
      std::string encoding;   ///< The encoding of the data in this response (optional; defaults
                              ///  to "json" if not specified).
      std::string data;       ///< The data to be sent to the requestor.
	    char priority;           ///< Priority of this pull response.  Responses with higher priority
	                            ///  values will be sent first if multiple messages are queued.
	                            ///  Typically, this will match the priority of the pull request which
	                            ///  initiated this response.
      
      /**
       * Convenience method which creates a PullResponse, prepopulating elements
       * which must match the PullRequest which generated the response.  
       * Specifically, this method sets the requestUid, pluginId, and mimeType 
       * to match the values from request.
       * 
       * @param request The PullRequest which the PullResponse should be
       *                generated from.
       * @return A PullResponse, with mandatory fields preset to the correct
       *         values.
       */
      static PullResponse createFromPullRequest(PullRequest &request);
      
      friend std::ostream& operator<<(std::ostream &os, const ammo::gateway::PullResponse &resp) {
        os << "Response to " << resp.pluginId << " for request " << resp.requestUid << " for type " << resp.requestUid;
        return os;
      }
    };
    
    /**
    * This class is used to connect a gateway plugin to the core gateway.  Each 
    * plugin should use at least one instance of this class; a plugin may create
    * more than one (establish more than one connection to the core gateway) if
    * needed (see the AndroidGatewayPlugin, which creates a new GatewayConnector
    * for each connected device; this aids in tracking requests and subscriptions
    * for multiple devices).
    */
    class LibGatewayConnector_Export GatewayConnector {
    public:
      /**
      * Creates a new GatewayConnector with the given GatewayConnectorDelegate and
      * establishes a connection to the gateway core.
      *
      * @param delegate A GatewayConnectorDelegate object to be used by this
      *                 GatewayConnector instance.  May be NULL (no delegate methods
      *                 will be called).
      */
      GatewayConnector(GatewayConnectorDelegate *delegate);
      
      /**
      * Creates a new GatewayConnector with the given GatewayConnectorDelegate and
      * establishes a connection to the gateway core.
      *
      * @param delegate A GatewayConnectorDelegate object to be used by this
      *                 GatewayConnector instance.  May be NULL (no delegate methods
      *                 will be called).
      * @param configfile A path to the gateway config file.
      */
      GatewayConnector(GatewayConnectorDelegate *delegate, std::string configfile);
    
      /**
      * Destroys a GatewayConnector.
      */
      ~GatewayConnector();
      
      //General connection negotiation and bookkeeping
      /**
      * Associates a device with this GatewayConnector in the gateway core.  This
      * method should only be needed by device connector plugins (e.g. the Android
      * Gateway Plugin).
      * 
      * @todo Authentication is not yet implemented in the gateway--  this method
      *       will send an authentication request, but the gateway will always
      *       will always return 'success' (and this method will always return
      *       true).  We should actually perform authentication here (pending more
      *       information from the security people).
      * 
      * @param device The unique ID of the device connecting to the gateway
      * @param user The unique ID of the user associated with the device connecting
      *             to the gateway.
      * @param key The authentication key/password for this user and device. Content
      *            of this parameter is still to be determined.
      * 
      * @return true if authentication was successful; false if authentication
      *         failed.
      */
      bool associateDevice(std::string device, std::string user, std::string key);
      
      //--Data-Push support methods--
    
      //Sender-side
      /**
       * Pushes a piece of data (with a particular URI and type) to the gateway.
       * 
       * Data pushed with this method can be received by listeners registered with
       * registerDataInterest.  All listeners registered for the type specified by
       * mimeType will receive this piece of data.
       * 
       * @param pushData The data to be pushed to the gateway.  uri and mimeType
       *                 must be set, or this call must fail (other parameters
       *                 are optional, and will use sane defaults).
       * 
       * @return true if the operation succeeded; false if the operation failed.
       */
      bool pushData(ammo::gateway::PushData &pushData);
      
      bool pushAcknowledgement(ammo::gateway::PushAcknowledgement &ack);
    
      /**
       * Requests data from a gateway plugin or device (which claims it can handle a
       * pull request of a particular type).
       *
       * @param request The pull request to send to the gateway.  This call will fail
       *                if requestUid, pluginId, and mimeType are not set; other fields are
       *                optional and will use default values if not explicitly set.
       *
       * @return true if the operation succeeded; false if the operation failed.
       */
      bool pullRequest(PullRequest &request);
    
      /** 
       * Sends a response to a pull request.  Used by plugins with a registered
       * pull request handler.
       *
       * @param response The pull response to send back to the requesting plugin.
       *                 The requestUid, pluginId, and mimeType fields must be
       *                 set, and must match the corresponding fields from the
       *                 pull request which triggered this response.
       *
       * @return true if the operation succeeded; false if the operation failed.
       */
      bool pullResponse(PullResponse &response);
    
    
      //Receiver-side
      /**
      * Registers interest in pushed data of type mime_type.  Data will be pushed
      * to the DataPushReceiverListener specified by listener.
      *
      * @param mime_type The type of data to listen for.
      * @param listener  The DataPushReceiverListener object which will be called
      *                  when data is available.
      * @param scope     The scope of this registration.  SCOPE_LOCAL will only
      *                  deliver data originating from the connected gateway;
      *                  SCOPE_GLOBAL will deliver data originating from all
      *                  connected gateways (subject to each data object's scope
      *                  parameter).
      *
      * @return true if the operation succeeded; false if the operation failed.
      */
      bool registerDataInterest(std::string mime_type, DataPushReceiverListener *listener, MessageScope scope = SCOPE_GLOBAL);
      
      /**
      * Unregisters interest in pushed data of type mime_type.  Will unregister all
      * listeners associated with this plugin if this plugin has registered multiple
      * listeners for the same type.
      * 
      * @param mime_type The type of data to unregister interest in.
      * @param scope     The scope of the registration to unregister interest
      *                  in; should be the same as the original registration.
      * 
      * @return true if the operation succeeded; false if the operation failed.
      */
      bool unregisterDataInterest(std::string mime_type, MessageScope scope = SCOPE_GLOBAL);
      
      /**
      * Registers a pull request handler for the specified data type.
      * 
      * @param mime_type The type of data to listen for pull requests for.
      * @param listener  The PullRequestReceiverListener object which will be called
      *                  when a new pull request is received.
      *
      * @return true if the operation succeeded; false if the operation failed.
      */
      bool registerPullInterest(std::string mime_type, PullRequestReceiverListener *listener, MessageScope scope = SCOPE_LOCAL);
      
      /**
      * Unregisters interest in pull requests for the specified data type.  Will
      * unregister all listeners associated with this plugin if this plugin has 
      * registered multiple listeners for the same type.
      * 
      * @param mime_type The type of data to unregister interest in.
      * 
      * @return true if the operation succeeded; false if the operation failed.
      */
      bool unregisterPullInterest(std::string mime_type, MessageScope scope = SCOPE_LOCAL);
      
      /**
      * Registers a listener to be called when data is received as a response from a
      * pull request.  Should be called in conjunction with pullRequest when
      * initiating a pull request; does not have to be called more than once if more
      * than one pull request will be made for the same mime type.
      * 
      * @param mime_type The type of data to listen for pull responses for.
      * @param listener  The PullResponseReceiverListener object which will be
      *                  called when a new pull response is received.
      * 
      * @return true if the operation succeeded; false if the operation failed.
      */
      bool registerPullResponseInterest(std::string mime_type, PullResponseReceiverListener *listener);
      
      /**
      * Unregisters interest in pull responses for the specified data type.  Will
      * unresgister all listeners associated with this plugin if this plugin has
      * registered multiple listeners for the same data type.
      * 
      * @param mime_type The type of data to unregister interest in.
      * 
      * @return true if the operation succeeded; false if the operation failed.
      */
      bool unregisterPullResponseInterest(std::string mime_type);
      
    private:
      void init(GatewayConnectorDelegate *delegate, ammo::gateway::internal::GatewayConfigurationManager *config); 
      
      void onConnectReceived();
      void onDisconnectReceived();
      void onAssociateResultReceived(const ammo::gateway::protocol::AssociateResult &msg);
      void onPushDataReceived(const ammo::gateway::protocol::PushData &msg, char messagePriority);
      void onPushAcknowledgementReceived(const ammo::gateway::protocol::PushAcknowledgement &msg);
      void onPullRequestReceived(const ammo::gateway::protocol::PullRequest &msg, char messagePriority);
      void onPullResponseReceived(const ammo::gateway::protocol::PullResponse &msg, char messagePriority);
      
      GatewayConnectorDelegate *delegate;
      std::map<std::string, DataPushReceiverListener *> receiverListeners;
      std::map<std::string, PullRequestReceiverListener *> pullRequestListeners;
      std::map<std::string, PullResponseReceiverListener *> pullResponseListeners;
    
      ammo::gateway::internal::NetworkConnector<ammo::gateway::protocol::GatewayWrapper, ammo::gateway::internal::GatewayEventHandler, ammo::gateway::internal::SYNC_MULTITHREADED, 0xdeadbeef> *connector;
      ammo::gateway::internal::GatewayEventHandler *handler;
      
      bool connected;
      
      friend class ammo::gateway::internal::GatewayEventHandler;
    };
    
    /**
    * The delegate object for a GatewayConnector.  Plugins implement the methods in
    * this class to implement behaviors associated with lifecycle events of the
    * gateway connector, such as connection and disconnection.
    */
    class LibGatewayConnector_Export GatewayConnectorDelegate {
    public:
      /**
      * Called when the GatewayConnector connects to the gateway core.
      * 
      * @param sender The GatewayConnector object that connected to the gateway.
      */
      virtual void onConnect(GatewayConnector *sender) = 0;
      
      /**
      * Called when the GatewayConnector disconnects from the gateway core.
      * 
      * @param sender The GatewayConnector object that disconnect from the gateway.
      */
      virtual void onDisconnect(GatewayConnector *sender) = 0;
      
      /**
      * Optional delegate method called after an authentication message is processed
      * by the gateway.  GatewayConnectorDelegate subclasses do not have to
      * implement this method if it's not needed (associateDevice is not being 
      * called); the default implementation does nothing.
      * 
      * @param sender The GatewayConnector which received the authentication result.
      * @param result true if authentication succeeded; false if authentication
      *               failed.
      */
      virtual void onAuthenticationResponse(GatewayConnector *sender, bool result);

      virtual void onPushAcknowledgementReceived(GatewayConnector *sender, const ammo::gateway::PushAcknowledgement &ack);
    };
    
    /**
    * Listener class for pushed data.  Plugins subclass this class to receive data
    * pushed with pushData.
    */
    class LibGatewayConnector_Export DataPushReceiverListener {
    public:
      virtual void onPushDataReceived(GatewayConnector *sender, ammo::gateway::PushData &pushData) = 0;
    };
    
    /**
    * Listener class for pull requests.  Plugins subclass this class to receive
    * pull requests sent with pullRequest.
    */
    class LibGatewayConnector_Export PullRequestReceiverListener {
    public:
      /**
      * Called when a pull request is received by the gateway for the registered
      * data type.  A plugin's implementation of this method should call
      * pullResponse at least once to send the requested data. 
      */
      virtual void onPullRequestReceived(GatewayConnector *sender, ammo::gateway::PullRequest &pullReq) = 0;
    };
    
    /**
    * Listener class for pull responses.  Plugins subclass this class to receive
    * responses from pull requests.
    */
    class LibGatewayConnector_Export PullResponseReceiverListener {
    public:
      virtual void onPullResponseReceived(GatewayConnector *sender, PullResponse &response) = 0;
    };

  }
}




#endif        //  #ifndef GATEWAY_CONNECTOR_H

