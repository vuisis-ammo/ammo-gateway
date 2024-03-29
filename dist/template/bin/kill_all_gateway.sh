#!/bin/bash

WAIT_TIME=10

echo "Killing all gateway processes..."

killall GatewayCore
killall AndroidGatewayPlugin
killall LdapGatewayPlugin
killall DataStoreGatewayPlugin
pkill -f edu.vu.isis.ammo.rmcastplugin.RMCastPlugin
pkill -f edu.vu.isis.ammo.mcastplugin.MCastPlugin

echo "Waiting $WAIT_TIME seconds to see if processes will exit cleanly..."
sleep $WAIT_TIME

echo "Sending KILL to all gateway processes..."
killall -9 GatewayCore
killall -9 AndroidGatewayPlugin
killall -9 LdapGatewayPlugin
killall -9 DataStoreGatewayPlugin
pkill -9 -f edu.vu.isis.ammo.rmcastplugin.RMCastPlugin
pkill -9 -f edu.vu.isis.ammo.mcastplugin.MCastPlugin

exit 0
