#!/bin/sh

#export LD_LIBRARY_PATH=$PWD/lib:$LD_LIBRARY_PATH
#export PATH=$PWD/bin:$PATH
export WORKPATH=/customer/carlife


#### start bluetooth ####
gocsdk -E6 -T/dev/ttyS3 -B460800 &

ifconfig lo up

mkdir -p /var/wifi/misc
mkdir -p /var/run/hostapd

mdnsd
#lyLinkUI >/dev/null 2>&1 &
lyLinkApp >/dev/null 2>&1 &

insmod $WORKPATH/lib/8821cs.ko

#### start ap mode ####
# ifconfig wlan0 192.168.1.1 up
# hostapd -B $WORKPATH/hostapd.conf
# udhcpd $WORKPATH/udhcpd-ap.conf

#### start sta mode ####
# wpa_supplicant -B -Dnl80211 -iwlan0 -c/customer/wifi/wpa_supplicant.conf
# udhcpc -b -iwlan0 -s/customer/wifi/udhcpc.script

/customer/carlife/sta_carlife.sh start

udhcpc -iwlan0 -b -s/customer/carlife/udhcpc.script
