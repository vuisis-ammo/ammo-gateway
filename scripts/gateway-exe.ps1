# PowerShell script for building the AMMO Gateway and installer

. .\scripts\environ.ps1

$env:GATEWAY_VERSION=$(get_version_full)
$env:GATEWAY_ROOT=$pwd.path

Write-Host Building Gateway Version: $env:GATEWAY_VERSION
Write-Host GATEWAY_ROOT: $env:GATEWAY_ROOT

cd $env:GATEWAY_ROOT
ant configure
ant build
& "C:\Program Files\NSIS\makensis.exe" /DVERSION=$env:GATEWAY_VERSION /DACE_ROOT=$env:ACE_ROOT /DPROTOBUF_ROOT=$env:PROTOBUF_ROOT /DQTDIR="C:\Qt\4.8.3" ammo-gateway.nsi

mv "ammo-gateway-$(get_version_full).exe" "$(get_version_branch)-ammo-gateway-$(get_version_full).exe"

