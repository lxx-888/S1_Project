#!/bin/sh

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/customer/wifi
export PATH=$PATH:/customer/wifi
nvconf update 0
nvconf update 1
sigma_wifi_init.sh
/customer/cardv/cardv /customer/cardv/default.ini &
sleep 2
echo rtsp 1 > /tmp/cardv_fifo
run_goahead.sh
exit 0


