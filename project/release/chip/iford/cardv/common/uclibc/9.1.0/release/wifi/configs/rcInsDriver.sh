#!/bin/sh
echo "Launch device driver in rcInsDriver.sh..."
sleep 1

NETEN=`nvconf get 0 Camera.Menu.WiFi`
APSTA=`nvconf get 1 wireless.apstaswitch`

looptimes=0
while true
do
	if [ $NETEN = "OFF" ]; then
		echo "wifi disable by default."
		break
	fi

	if [ -e /sys/bus/usb/devices/1-1 ] \
		|| [ -e /sys/bus/sdio/devices/mmc1:0001:1 ];then
		if [ $? -eq 0 ]; then
			if [ $NETEN = "ON" ]; then
				usleep 500000
				if [ $APSTA = "STA" ]; then
					sta.sh start &
				else
					ap.sh start &
				fi
				echo rtsp 1 > /tmp/cardv_fifo
			fi
			break
		fi
	else
		usleep 500000
		echo "wait for usb wifi attach,delay 0.5sec"
	fi
	
	#wait a moment.
	looptimes=`busybox expr $looptimes + 1`
	if test $looptimes -ge 4
	then
		echo "can't find usb wifi module!"
		break
	fi
done

exit $?
