#!/bin/sh

case "$1" in
	start)
	echo "Launch Wifi module AP Mode..."
	ifconfig wlan0 192.168.1.1 up

	#IPADDR="`nvconf get 1 wireless.ap.ipaddr`"
    #SUBNETMASK="`nvconf get 1 wireless.ap.subnetmask`"
    #ifconfig wlan0 up
    #ifconfig wlan0 $IPADDR netmask $SUBNETMASK
    #sleep 1
    #/usr/sbin/udhcpd -S /customer/wifi/udhcpd-ap.conf &
        
	WIFIMAC="`cat /sys/class/net/wlan0/address | /usr/bin/awk -F ":" '{print $4$5$6}`"
    bSSID=`cat /customer/wifi/webserver/www/cgi-bin/net_config.bin|grep wireless.ap.ssid|sed 's/^[[:print:]]\{17\}//'|sed 's/[[:print:]]\{2\}$//' `
    SSID=`nvconf get 1 wireless.ap.ssid`
    if [ "$bSSID" = "$SSID" ]; then 
            NSSID=${SSID}_${WIFIMAC}
            sed -i "s/^ssid.*$/ssid=$NSSID/" /customer/carlife/hostapd.conf
            nvconf set 1 wireless.ap.ssid $NSSID
            nvconf set 1 devinfo.macaddr `cat /sys/class/net/wlan0/address`
    else
            sed -i "s/^ssid.*$/ssid=$SSID/" /customer/carlife/hostapd.conf
    fi
        
	PSK="`nvconf get 1 wireless.ap.wpa.psk`"
	sed -i "s/^wpa_passphrase.*$/wpa_passphrase=$PSK/" /customer/carlife/hostapd.conf
	hostapd -B /customer/carlife/hostapd.conf
	udhcpd /customer/carlife/udhcpd-ap.conf
	;;
	stop)
	echo " Kill all process of AP Mode"
	busybox killall hostapd
	busybox killall udhcpd
	ifconfig wlan0 down
	;;
	*)
	echo "Usage: $0 {start|stop|restart}"
	exit 1
esac

