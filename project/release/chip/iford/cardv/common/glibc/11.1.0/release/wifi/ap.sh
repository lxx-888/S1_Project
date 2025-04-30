#!/bin/sh

case "$1" in
  start)
      echo "Launch Wifi module AP Mode ..."

    mkdir -p /tmp/wifi/run
        chmod 777 /tmp/wifi/run
        mkdir -p /var/wifi/misc/
        mkdir -p /var/lib/misc/
        mkdir -p /var/run/hostapd/

    if [ "$WIFI_MODULE" == "bcmdhd" ]; then
        HOSTAPD="hostapd_ap6256"
    else
        HOSTAPD="hostapd"     
    fi

    if [ "`lsmod|grep $WIFI_MODULE`" == "" ]; then
        if [ "$WIFI_MODULE" == "bcmdhd" ]; then
            echo "wifi module is ap6256"
            HOSTAPD="hostapd_ap6256"
            insmod /customer/wifi/lib/bcmdhd.ko firmware_path=/customer/wifi/lib/fw_bcm43456c5_ag_apsta.bin nvram_path=/customer/wifi/lib/nvram_ap6256.txt
        else
            echo "wifi modu1e is $WIFI_MODULE.ko"
            HOSTAPD="hostapd"
            insmod /customer/wifi/$WIFI_MODULE.ko
        fi
    fi

    sleep 1

    if [ $? -eq 1 ]; then
        exit 1
    fi

    IPADDR="`nvconf get 1 wireless.ap.ipaddr`"
    SUBNETMASK="`nvconf get 1 wireless.ap.subnetmask`"
    #sleep 1
    ifconfig wlan0 up
    ifconfig wlan0 $IPADDR netmask $SUBNETMASK
    sleep 1
    /usr/sbin/udhcpd -S /customer/wifi/udhcpd-ap.conf &

    WIFIMAC="`cat /sys/class/net/wlan0/address | /usr/bin/awk -F ":" '{print $4$5$6}`"
    bSSID=`cat /customer/wifi/webserver/www/cgi-bin/net_config.bin|grep wireless.ap.ssid|sed 's/^[[:print:]]\{17\}//'|sed 's/[[:print:]]\{2\}$//' `
    SSID=`nvconf get 1 wireless.ap.ssid`
    if [ "$bSSID" = "$SSID" ]; then 
        if [[ $SSID != *"$WIFIMAC"* ]]; then
                    NSSID=${SSID}_${WIFIMAC}
                fi
        sed -i "s/^ssid.*$/ssid=$NSSID/" /customer/wifi/$HOSTAPD.conf
        nvconf set 1 wireless.ap.ssid $NSSID
        nvconf set 1 devinfo.macaddr `cat /sys/class/net/wlan0/address`
    else
        sed -i "s/^ssid.*$/ssid=$SSID/" /customer/wifi/$HOSTAPD.conf
    fi
    PSK="`nvconf get 1 wireless.ap.wpa.psk`"
    sed -i "s/^wpa_passphrase.*$/wpa_passphrase=$PSK/" /customer/wifi/$HOSTAPD.conf
    hostapd /customer/wifi/$HOSTAPD.conf -B                                    
    run_goahead.sh
;;
  stop)
    echo " Kill all process of AP Mode"
    busybox killall udhcpd
    busybox killall hostapd
    busybox killall goahead
    ifconfig wlan0 down
    #rmmod $WIFI_MODULE
;;
  restart)
    echo " restart AP Mode"
    busybox killall udhcpd
    busybox killall hostapd
    busybox killall goahead
    ifconfig wlan0 down
    sleep 1
    mkdir -p /tmp/wifi/run
        chmod 777 /tmp/wifi/run
        mkdir -p /var/wifi/misc/
        mkdir -p /var/lib/misc/
        mkdir -p /var/run/hostapd/
    export LD_LIBRARY_PATH=${LD_LIBRARAY_PATH}:/customer/wifi

    HOSTAPD="hostapd"     
    IPADDR="`nvconf get 1 wireless.ap.ipaddr`"
    SUBNETMASK="`nvconf get 1 wireless.ap.subnetmask`"
    sleep 1
    ifconfig wlan0 up
    ifconfig wlan0 $IPADDR netmask $SUBNETMASK
    sleep 1
    /usr/sbin/udhcpd -S /customer/wifi/udhcpd-ap.conf &
    
    WIFIMAC="`cat /sys/class/net/wlan0/address | /usr/bin/awk -F ":" '{print $4$5$6}`"
    bSSID=`cat /customer/wifi/net_config.bin|grep wireless.ap.ssid|sed 's/^[[:print:]]\{17\}//'|sed 's/[[:print:]]\{2\}$//' `
    SSID=`nvconf get 1 wireless.ap.ssid`
    if [ "$bSSID" = "$SSID" ]; then 
        if [[ $SSID != *"$WIFIMAC"* ]]; then
                    NSSID=${SSID}_${WIFIMAC}
                fi
        sed -i "s/^ssid.*$/ssid=$NSSID/" /customer/wifi/$HOSTAPD.conf
        nvconf set 1 wireless.ap.ssid $NSSID
        nvconf set 1 devinfo.macaddr `cat /sys/class/net/wlan0/address`
    else
        sed -i "s/^ssid.*$/ssid=$SSID/" /customer/wifi/$HOSTAPD.conf
    fi
    PSK="`nvconf get 1 wireless.ap.wpa.psk`"
    sed -i "s/^wpa_passphrase.*$/wpa_passphrase=$PSK/" /customer/wifi/$HOSTAPD.conf
    hostapd /customer/wifi/$HOSTAPD.conf -B
    run_goahead.sh

;;
  *)
    echo "Usage: $0 {start|stop|restart}"
    exit 1
esac

exit $?

