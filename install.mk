#
# make install target for Makefile - append me to Makefile
#

###############################################################################
# Configuration
###############################################################################

# Installation directory for Gateway
INSTALL_DIR_BASE=/usr
VERSION=`cat .build-version | sed 's/release-//' | cut -d- -f 1,2`

###############################################################################
# Installation rules
###############################################################################

INSTALL_DIR=$(DESTDIR)/$(INSTALL_DIR_BASE)

.PHONY: install
install: all
	echo "Creating directories ..."
	mkdir -p $(INSTALL_DIR)
	mkdir -p $(INSTALL_DIR)/bin
	mkdir -p $(INSTALL_DIR)/lib
	mkdir -p $(INSTALL_DIR)/include/ammo-gateway
	mkdir -p $(INSTALL_DIR)/include/ammo-json
	mkdir -p $(DESTDIR)/etc/init.d
	mkdir -p $(DESTDIR)/etc/ammo-gateway
	mkdir -p $(DESTDIR)/etc/ammo-gateway/keys
	mkdir -p $(DESTDIR)/etc/ammo-gateway/jgroups
	mkdir -p $(DESTDIR)/usr/share/java
	mkdir -p $(DESTDIR)/var/log/ammo-gateway
	mkdir -p $(DESTDIR)/var/db/ammo-gateway
	mkdir -p $(DESTDIR)/var/run/ammo-gateway
	echo "Installing binaries ..."
	install -m 755 build/bin/AndroidGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/AtsGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/GatewayCore $(INSTALL_DIR)/bin
	install -m 755 build/bin/GatewayUsbTransfer $(INSTALL_DIR)/bin
	install -m 755 build/bin/LdapGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/DataStoreGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/SerialGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/SamplePushReceiverGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/SamplePushTestDriverPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/SpotPushReceiverGatewayPlugin $(INSTALL_DIR)/bin
	echo "Installing headers ..."
	install -m 755 LibGatewayConnector/GatewayConnector.h $(INSTALL_DIR)/include/ammo-gateway
	install -m 755 LibGatewayConnector/Enumerations.h $(INSTALL_DIR)/include/ammo-gateway	
	install -m 755 LibGatewayConnector/LibGatewayConnector_Export.h $(INSTALL_DIR)/include/ammo-gateway	
	install -m 755 common/log.h $(INSTALL_DIR)/include/ammo-gateway	
	install -m 755 common/version.h $(INSTALL_DIR)/include/ammo-gateway	
	echo "Installing JSON headers..."
	install -m 755 LibJSON/json/autolink.h $(INSTALL_DIR)/include/ammo-json
	install -m 755 LibJSON/json/config.h $(INSTALL_DIR)/include/ammo-json
	install -m 755 LibJSON/json/features.h $(INSTALL_DIR)/include/ammo-json
	install -m 755 LibJSON/json/forwards.h $(INSTALL_DIR)/include/ammo-json
	install -m 755 LibJSON/json/json.h $(INSTALL_DIR)/include/ammo-json
	install -m 755 LibJSON/json/reader.h $(INSTALL_DIR)/include/ammo-json
	install -m 755 LibJSON/json/value.h $(INSTALL_DIR)/include/ammo-json
	install -m 755 LibJSON/json/writer.h $(INSTALL_DIR)/include/ammo-json
	echo "Installing libs ..."
	install -m 644 build/lib/libgatewayconnector.so.$(VERSION) $(INSTALL_DIR)/lib
	ln -s $(INSTALL_DIR_BASE)/lib/libgatewayconnector.so.$(VERSION) $(INSTALL_DIR)/lib/libgatewayconnector.so
	install -m 644 build/lib/libgatewaydatastore.so.$(VERSION) $(INSTALL_DIR)/lib
	ln -s $(INSTALL_DIR_BASE)/lib/libgatewaydatastore.so.$(VERSION) $(INSTALL_DIR)/lib/libgatewaydatastore.so
	install -m 644 build/lib/libgeotrans-mgrs.so.$(VERSION) $(INSTALL_DIR)/lib
	ln -s $(INSTALL_DIR_BASE)/lib/libgeotrans-mgrs.so.$(VERSION) $(INSTALL_DIR)/lib/libgeotrans-mgrs.so
	install -m 644 build/lib/libjson.so.$(VERSION) $(INSTALL_DIR)/lib
	ln -s $(INSTALL_DIR_BASE)/lib/libjson.so.$(VERSION) $(INSTALL_DIR)/lib/libjson.so
	echo "Installing jars ..."
	install -m 644 JavaGatewayConnector/dist/lib/gatewaypluginapi.jar $(DESTDIR)/usr/share/java
	install -m 644 MCastPlugin/dist/lib/mcastplugin.jar $(DESTDIR)/usr/share/java
	install -m 644 RMCastPlugin/dist/lib/rmcastplugin.jar $(DESTDIR)/usr/share/java
	install -m 644 RMCastPlugin/libs/jgroups-gw.jar $(DESTDIR)/usr/share/java
	install -m 644 RMCastPlugin/libs/json-20090211.jar $(DESTDIR)/usr/share/java
	install -m 644 RMCastPlugin/libs/protobuf-java-2.4.1.jar $(DESTDIR)/usr/share/java
	install -m 644 RMCastPlugin/libs/slf4j-api-1.6.4.jar $(DESTDIR)/usr/share/java
	install -m 644 RMCastPlugin/libs/slf4j-simple-1.6.4.jar $(DESTDIR)/usr/share/java
	install -m 644 RMCastPlugin/libs/logback-core-1.0.11.jar $(DESTDIR)/usr/share/java
	install -m 644 RMCastPlugin/libs/logback-classic-1.0.11.jar $(DESTDIR)/usr/share/java
	install -m 644 RMCastPlugin/libs/logback-access-1.0.11.jar $(DESTDIR)/usr/share/java
	echo "Installing scripts ..."
	install -m 755 debian/init.d $(DESTDIR)/etc/init.d/ammo-gateway
	install -m 755 dist/template/bin/launch_ammo_gateway_headless.sh $(INSTALL_DIR)/bin
	install -m 755 dist/template/bin/launch_ammo_gateway.sh $(INSTALL_DIR)/bin
	install -m 755 dist/template/bin/kill_all_gateway.sh $(INSTALL_DIR)/bin
	install -m 755 MCastPlugin/mcastplugin.sh $(INSTALL_DIR)/bin
	install -m 755 RMCastPlugin/rmcastplugin.sh $(INSTALL_DIR)/bin
	echo "Installing config files ..."
	install -m 644 build/etc/AtsPluginConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/GatewayConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/LdapPluginConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/LoggingConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/DataStorePluginConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/SerialPluginConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/MCastPluginConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/RMCastPluginConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 755 RMCastPlugin/jgroups/udp.xml $(DESTDIR)/etc/ammo-gateway/jgroups
	install -m 755 RMCastPlugin/jgroups/udpMedia.xml $(DESTDIR)/etc/ammo-gateway/jgroups

