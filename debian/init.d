#! /bin/sh
#
# ammo-gateway	/etc/init.d/ startup and shutdown script.
#
# Version:	1.0  03-May-2011  johnwilliams@isis.vanderbilt.edu
#

PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin
DESC="AMMO Gateway"

# Include ammo-gateway defaults if available
if [ -f /etc/default/ammo-gateway ] ; then
    . /etc/default/ammo-gateway
fi

set -e

isProcessRunning()
{
    pgrepable=`echo $1 | cut -c1-15`
    if [ ! "`pgrep $pgrepable | wc -l`" -eq "0" ]
    then
        echo $1 is running
    else
        echo $1 is NOT running
    fi
}

startGateway()
{
    /usr/bin/launch_ammo_gateway_headless.sh
}

stopGateway()
{
    /usr/bin/kill_all_gateway.sh
}

case "$1" in
  start)
        echo "Starting $DESC: "
        startGateway
        ;;
  stop)
        echo "Stopping $DESC: "
        stopGateway
        ;;
  restart)
    echo "Restarting $DESC: "
        stopGateway
        startGateway
        ;;
  status)
    isProcessRunning GatewayCore
    isProcessRunning AndroidGatewayPlugin
    isProcessRunning LdapGatewayPlugin
    isProcessRunning LocationStoreGatewayPlugin
    ;;
  *)
    N=/etc/init.d/$NAME
    echo "Usage: $N {start|stop|restart|status}" >&2
    exit 1
    ;;
esac

exit 0