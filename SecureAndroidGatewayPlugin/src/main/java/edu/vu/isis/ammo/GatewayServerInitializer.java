package edu.vu.isis.ammo;

import edu.vu.isis.ammo.core.pb.AmmoMessages;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelPipeline;
import io.netty.channel.socket.SocketChannel;
import io.netty.handler.codec.protobuf.ProtobufDecoder;
import io.netty.handler.ssl.SslHandler;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLEngine;

/**
 * Created with IntelliJ IDEA.
 * User: jwilliams
 * Date: 4/3/13
 * Time: 3:16 PM
 * To change this template use File | Settings | File Templates.
 */
public class GatewayServerInitializer extends ChannelInitializer<SocketChannel> {
    private static final Logger logger = LoggerFactory.getLogger(GatewayServerInitializer.class);

    final int MAXIMUM_FRAME_LENGTH = Integer.MAX_VALUE;

    @Override
    protected void initChannel(SocketChannel socketChannel) throws Exception {
        SecureGatewayPluginConfigurationManager config = SecureGatewayPluginConfigurationManager.getInstance();

        ChannelPipeline pipeline = socketChannel.pipeline();

        SSLContext sslContext = SSLContext.getDefault();
        SSLEngine sslEngine = sslContext.createSSLEngine();

        sslEngine.setUseClientMode(false);

        sslEngine.setWantClientAuth(config.isClientAuthEnabled());
        sslEngine.setNeedClientAuth(config.isClientAuthEnabled());

        logger.debug("SSL: Enabled protocols:");
        for(String protocol : sslEngine.getEnabledProtocols()) {
            logger.debug("    {}", protocol);
        }

        logger.debug("SSL: Enabled cipher suites:");
        for(String cipher : sslEngine.getEnabledCipherSuites()) {
            logger.debug("    {}", cipher);
        }

        /*logger.debug("SSL: Enabled protocols: {}", sslEngine.getEnabledProtocols().length);
        logger.debug("SSL: Enabled Cipher Suites: {}", sslEngine.getEnabledCipherSuites().length);*/

        pipeline.addLast("ssl", new SslHandler(sslEngine));

        pipeline.addLast("decoder", new AndroidMessageDecoder());
        pipeline.addLast("protobuf", new ProtobufDecoder(AmmoMessages.MessageWrapper.getDefaultInstance()));
        pipeline.addLast("handler", new AndroidMessageHandler());

        pipeline.addLast("encoder", new AndroidMessageEncoder());
    }
}
