#!/bin/sh

#nvconf update 0
#nvconf update 1
NETEN=`nvconf get 0 Camera.Menu.WiFi`
APSTA=`nvconf get 1 wireless.apstaswitch`

if [ "$NETEN" = "OFF" ]; then
	echo "wait for NET $APSTA ON."
	usleep 300000
	nvconf set 0 Camera.Menu.WiFi ON
	if [ "$APSTA" = "STA" ]; then
		sta.sh start &
	else
		ap.sh start &
	fi
	echo rtsp 1 > /tmp/cardv_fifo
elif [ "$NETEN" = "ON" ]; then
	echo "wait for NET $APSTA OFF."
	echo rtsp 0 > /tmp/cardv_fifo
	usleep 200000
	nvconf set 0 Camera.Menu.WiFi OFF
	if [ "$APSTA" = "STA" ]; then
		sta.sh stop
	else
		ap.sh stop
	fi
	busybox killall goahead
fi

