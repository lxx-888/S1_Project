#!/bin/sh

NETEN=`nvconf get 0 Camera.Menu.WiFi`
APSTA=`nvconf get 1 wireless.apstaswitch`

if [ "`cat /tmp/rec_status`" = "0" ]; then 
	echo "reset default..."
	cp -f /customer/wifi/webserver/www/cgi-bin/cgi_config.bin /customer/wifi/
	cp -f /customer/wifi/webserver/www/cgi-bin/net_config.bin /customer/wifi/

	#reset boot env
#	CMD="nvconf get 0 Camera.Menu.LoopingVideo"
#	/customer/wifi/webserver/www/CGI_PROCESS.sh set LoopingVideo `$CMD`
#	CMD="nvconf get 0 Camera.Menu.ParkingMonitor"
#	/customer/wifi/webserver/www/CGI_PROCESS.sh set ParkingMonitor `$CMD`
#	CMD="nvconf get 0 Camera.Menu.GSensor"
#	/customer/wifi/webserver/www/CGI_PROCESS.sh set GSensor `$CMD`
#	CMD="nvconf get 0 Camera.Menu.VoiceSwitch"
#	/customer/wifi/webserver/www/CGI_PROCESS.sh set VoiceSwitch `$CMD`
	
	if [ $NETEN = "ON" ]; then	
		#ap.sh restart &
		if [ $APSTA = "STA" ]; then
			echo rtsp 0 > /tmp/cardv_fifo
			sta.sh restart &
			echo rtsp 1 > /tmp/cardv_fifo
		else
			echo rtsp 0 > /tmp/cardv_fifo
			ap.sh restart &
			echo rtsp 1 > /tmp/cardv_fifo
	fi
	fi
	
else
	echo "stop rec first!!!"
fi

