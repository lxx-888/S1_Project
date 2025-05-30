#!/bin/sh

workmode=$1
killwlan(){
    local NAME=$1
    echo $NAME
    ID=$(ps -ef | grep "$NAME" | grep -v "grep" | busybox awk '{print $1}')
    for id in $ID; do
        kill $id
        echo "killed $id"
    done
}

if [ ${workmode} = "ap" ]; then
    killwlan hostapd
    killwlan dnsmasq
else
    killwlan wpa_supplicant
    killwlan udhcpc
fi
