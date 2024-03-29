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
package edu.vu.isis.ammo.mcastplugin;

import edu.vu.isis.ammo.gateway.*;
import edu.vu.isis.ammo.core.pb.AmmoMessages;
import com.google.protobuf.ByteString;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

/**
 * class PluginServiceHandler
 * - transcode AmmoMessages received from JGroups into GatewayMessages
 * - transcode GatewayMessages received from Gateway into AmmoMessages
 **/ 


class PluginServiceHandler implements
			       GatewayConnectorDelegate,
			       DataPushReceiverListener,
			       PullRequestReceiverListener,
			       PullResponseReceiverListener
{
    private MulticastConnector mMcastConnector = null;
    private GatewayConnector mGatewayConnector = null;

    private HashMap<String,Integer> subscriptions = null;

    private static final Logger logger = LoggerFactory.getLogger(PluginServiceHandler.class);

    public PluginServiceHandler()
    {
	subscriptions = new HashMap<String,Integer>();
	// populate the subscriptions map with mimetypes read from config file
	PluginConfigurationManager pConfig = PluginConfigurationManager.getInstance();
	List<String> mimeTypes = pConfig.getMimeTypes();
	for( String mimeType : mimeTypes ) {
	    subscriptions.put(mimeType, 1);
	}
    }

    public void setGatewayConnector(GatewayConnector gatewayConnector)
    {
	mGatewayConnector = gatewayConnector;
    }

    public void setMcastConnector(MulticastConnector mcastConnector)
    {
	mMcastConnector = mcastConnector;
    }


    public void onConnect() { }	// from MCastConnector

    public void onDisconnect() { } // from MCastConnector

    public void onAmmoMessageReceived( AmmoMessages.MessageWrapper message ) // from MCastConnector
    {
	if (message.getType() == AmmoMessages.MessageWrapper.MessageType.DATA_MESSAGE) { // data message convert to push message
	    PushData pushData = new PushData();
	    AmmoMessages.DataMessage dataMessage = message.getDataMessage();
	    pushData.uri =  dataMessage.getUri();
	    pushData.mimeType = dataMessage.getMimeType();
	    pushData.encoding = dataMessage.getEncoding();
	    pushData.originUserName = dataMessage.getUserId();
	    pushData.data = dataMessage.getData().toByteArray();
	    pushData.scope = (AmmoMessages.MessageScope.GLOBAL == dataMessage.getScope()) ?
		MessageScope.SCOPE_GLOBAL :
		MessageScope.SCOPE_LOCAL;
	    mGatewayConnector.pushData(pushData);
	} else 	if (false) { // TBD SKN (message.getType() == AmmoMessages.MessageWrapper.MessageType.SUBSCRIBE_MESSAGE) {
	    // subscribe message check the sub map to see if we are not already subscribed to this type
	    AmmoMessages.SubscribeMessage subscribeMessage = message.getSubscribeMessage();
	    
	    String mimeType = subscribeMessage.getMimeType();
	    // @todo: handle queries
	    // 	    subData.query = subscribeMessage.getQuery();
	    // @todo: handle scope changes across different subscriptions of the same type
	    MessageScope scope = (AmmoMessages.MessageScope.GLOBAL == subscribeMessage.getScope()) ?
	    	MessageScope.SCOPE_GLOBAL :
	    	MessageScope.SCOPE_LOCAL;

	    if ( subscriptions.containsKey(mimeType) ) {
		int count = subscriptions.get(mimeType);
		subscriptions.put(mimeType, count+1); // increment refcount
	    } else {
		subscriptions.put(mimeType, 1);
		mGatewayConnector.registerDataInterest( mimeType, this, scope); // create a subscription with Gateway
	    }
	} else 	if (false) { // (message.getType() == AmmoMessages.MessageWrapper.MessageType.UNSUBSCRIBE_MESSAGE) {
	    // subscribe message check the sub map to see if we are not already subscribed to this type
	    AmmoMessages.UnsubscribeMessage unsubscribeMessage = message.getUnsubscribeMessage();
	    
	    String mimeType = unsubscribeMessage.getMimeType();
	    // @todo: handle scope changes across different subscriptions of the same type
	    MessageScope scope = (AmmoMessages.MessageScope.GLOBAL == unsubscribeMessage.getScope()) ?
	    	MessageScope.SCOPE_GLOBAL :
	    	MessageScope.SCOPE_LOCAL;

	    if ( subscriptions.containsKey(mimeType) ) {
		int count = subscriptions.get(mimeType);
		if (count == 1) {
		    subscriptions.remove(mimeType);
		    mGatewayConnector.unregisterDataInterest(mimeType, scope);
		} else
		    subscriptions.put(mimeType, count-1); // decrement refcount
	    }

	    
	}

    }

    private class SubscribeData
    {
	public MessageScope scope;
	public int refCount;
    }


    @Override
	public void onConnect(GatewayConnector sender) {
	// resend all the subscriptions
	Iterator ki = subscriptions.keySet().iterator();
	while( ki.hasNext() ) {
	    String mimeType = (String)ki.next();
	    sender.registerDataInterest( mimeType, this, MessageScope.SCOPE_GLOBAL ); // TBD - handle the scope properly
	}
	
    }

    @Override
	public void onDisconnect(GatewayConnector sender) {
    }

    @Override
	public void onAuthenticationResponse(GatewayConnector sender, boolean result) {
    }
    
    @Override
    public void onPushAcknowledgementReceived(GatewayConnector sender, PushAcknowledgement ack) {
      
    }


    @Override
	public void onPushDataReceived(GatewayConnector sender, PushData pushData)
    {
	
	AmmoMessages.MessageWrapper.Builder msg =
	    AmmoMessages.MessageWrapper.newBuilder();
	AmmoMessages.DataMessage.Builder pushMsg =
	    AmmoMessages.DataMessage.newBuilder();

	pushMsg.setUri( pushData.uri );
	pushMsg.setMimeType( pushData.mimeType );
	pushMsg.setEncoding( pushData.encoding );
	pushMsg.setUserId( pushData.originUserName );
	pushMsg.setData( ByteString.copyFrom(pushData.data) );
	pushMsg.setScope( pushData.scope == MessageScope.SCOPE_GLOBAL ? AmmoMessages.MessageScope.GLOBAL : AmmoMessages.MessageScope.LOCAL );

	msg.setType( AmmoMessages.MessageWrapper.MessageType.DATA_MESSAGE );
	msg.setDataMessage( pushMsg.build() );
	// send to mcastConnector
	mMcastConnector.sendMessage( msg.build() );
	
    }

    @Override
	public void onPullRequestReceived(GatewayConnector sender, PullRequest pullReq)
    {
	// currently unsupported ...
	
    }

    @Override
	public void onPullResponseReceived(GatewayConnector sender, PullResponse pullResp)
    {
	// currently unsupported ...
    }


}