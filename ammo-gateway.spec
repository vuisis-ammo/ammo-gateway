Summary:        AMMO Gateway
Name:           ammo-gateway
Version:        AMMO_VERSION
Release:        AMMO_RELEASE%{dist}
License:        AMMO
Group:          Applications/Internet
Source:         %{name}-%{version}.tar.gz
URL:            http://ammo.isis.vanderbilt.edu
BuildRoot:      %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
Requires:       ace = 6.0.1, gsoap, protobuf
BuildRequires:  gcc-c++, ace-devel = 6.0.1, gsoap-devel, protobuf-compiler, protobuf-devel

%description
Android Middleware Server


%prep
%setup -q

%build
mwc.pl --type make Gateway.mwc
cat install.mk >> Makefile
make PROTOBUF_ROOT=/usr GATEWAY_ROOT=`pwd`

%install
make DESTDIR=%{buildroot} PROTOBUF_ROOT=/usr GATEWAY_ROOT=`pwd` install

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%clean
rm -rf %{buildroot}

%files
%defattr(-, root, root, -)
/etc/ammo-gateway/AtsPluginConfig.json
/etc/ammo-gateway/GatewayConfig.json
/etc/ammo-gateway/LdapPluginConfig.json
/etc/ammo-gateway/LocationStorePluginConfig.json
/etc/ammo-gateway/PassPluginConfig.json
/etc/ammo-gateway/TigrPluginConfig.json
/usr/bin/AndroidGatewayPlugin
/usr/bin/AtsGatewayPlugin
/usr/bin/GatewayCore
/usr/bin/GatewayUsbTransfer
/usr/bin/LdapGatewayPlugin
/usr/bin/LocationStoreGatewayPlugin
/usr/bin/PassGatewayPlugin
/usr/bin/SamplePushReceiverGatewayPlugin
/usr/bin/SamplePushTestDriverPlugin
/usr/bin/SpotPushReceiverGatewayPlugin
/usr/bin/TigrGatewayPlugin
/usr/bin/kill_all_gateway.sh
/usr/bin/launch_ammo_gateway.sh
/usr/bin/launch_ammo_gateway_headless.sh
/usr/lib/libgatewayconnector.so
/usr/lib/libgeotrans-mgrs.so
/usr/lib/libjson.so

%changelog
* BUILD_DATE John Williams <johnwilliams@isis.vanderbilt.edu> - AMMO_VERSION
- Autobuild Release for AMMO project
