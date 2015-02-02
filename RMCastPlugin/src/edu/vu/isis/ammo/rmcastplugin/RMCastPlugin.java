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
package edu.vu.isis.ammo.rmcastplugin;

import edu.vu.isis.ammo.gateway.*;
import edu.vu.isis.ammo.core.pb.AmmoMessages;
import com.google.protobuf.ByteString;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.HashMap;

/**
 * class RMCastPlugin
 **/ 


public class RMCastPlugin 
{
    static ReliableMulticastConnector mMsgRmcastConnector = null;
    static GatewayConnector mMsgGatewayConnector = null;
    static PluginServiceHandler mMsgServiceHandler = null;

    static ReliableMulticastConnector mMediaRmcastConnector = null;
    static GatewayConnector mMediaGatewayConnector = null;
    static PluginServiceHandler mMediaServiceHandler = null;

    private static final Logger logger = LoggerFactory.getLogger(PluginServiceHandler.class);

    public static void main(String[] args)
    {
	// message handler ....
	mMsgServiceHandler = new PluginServiceHandler(false);

	mMsgGatewayConnector = new GatewayConnector( mMsgServiceHandler );
	mMsgServiceHandler.setGatewayConnector( mMsgGatewayConnector );

	mMsgRmcastConnector = new ReliableMulticastConnector(mMsgServiceHandler, "", 1234, "rmcast", "udp.xml", "AmmoGroup");
	mMsgServiceHandler.setRmcastConnector( mMsgRmcastConnector );
	
	// media handler 
	mMediaServiceHandler = new PluginServiceHandler(true);

	mMediaGatewayConnector = new GatewayConnector( mMediaServiceHandler );
	mMediaServiceHandler.setGatewayConnector( mMediaGatewayConnector );

	mMediaRmcastConnector = new ReliableMulticastConnector(mMediaServiceHandler, "", 1234, "mediaRmcast", "udpMedia.xml", "AmmoMedia");
	mMediaServiceHandler.setRmcastConnector( mMediaRmcastConnector );


	while(true) {
	    try {
		Thread.sleep(5);	// idle wait ...
	    } catch (InterruptedException ex) {
		logger.info("Main sleep interrupted ... exitting ... ");
		break;
	    }
	}
    }

}
