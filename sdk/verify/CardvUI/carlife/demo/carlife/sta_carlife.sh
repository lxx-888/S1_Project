#!/bin/sh

case "$1" in
	start)
	echo "Launch Wifi module STA Mode..."
	SSID="`nvconf get 1 wireless.sta.ssid`"
	sed -i "s/^.*ssid.*$/    ssid=\"$SSID\"/" /customer/wifi/wpa_supplicant.conf
	PSK="`nvconf get 1 wireless.sta.wpa.psk`"
	sed -i "s/^.*psk.*$/    psk=\"$PSK\"/" /customer/wifi/wpa_supplicant.conf
	ifconfig wlan0 up
	wpa_supplicant -Dnl80211 -iwlan0 -c /customer/wifi/wpa_supplicant.conf -B
	;;
	stop)
	echo " Kill all process of STA Mode"
	busybox killall wpa_supplicant
	busybox killall goahead
	ifconfig wlan0 down
	;;
	*)
	echo "Usage: $0 {start|stop|restart}"
	exit 1
esac

