#!/bin/sh

UDHCPD_CFG_PATH=/etc/usbnet_udhcpd.conf
LEASE_PATH='/var/lib/misc/udhcpd.leases'
DEFAULT_IP=192.168.7.2

COLOR_RED="\033[31m"
COLOR_YELLOW="\033[33m"
COLOR_END="\033[\0m"

gen_conf()
{
	if [ -e $UDHCPD_CFG_PATH ];then
		echo -e $COLOR_YELLOW"$UDHCPD_CFG_PATH is already exist!!!"$COLOR_END
	else
		cat > $UDHCPD_CFG_PATH <<-EOF
		interface usb0
		start 192.168.7.20
		end 192.168.7.250
		max_leases 231
		opt subnet 255.255.255.0
		opt lease 864000
		EOF
	fi;

	if [ $? -ne 0 ];then
		echo -e $COLOR_RED"can't create config file: $UDHCPD_CFG_PATH, make sure permission!!!"$COLOR_END
		exit 1
	fi

	if [ ! -e $LEASE_PATH ];then
		mkdir -p `dirname $LEASE_PATH`
		touch $LEASE_PATH
	fi
}

insert_mod()
{
	mod_name=$1
	if [ ! -e $mod_name ];then
		echo -e $COLOR_RED"can't find rndis releated module, pls check it first!!!"$COLOR_END
		exit 1
	fi
	insmod $mod_name > /dev/null 2>&1
}

# check dependancy
which udhcpd > /dev/null 2>&1
if [ $? -ne 0 ];then
	echo -e $COLOR_RED"can't found udhcpd tool!!!"$COLOR_END
	exit 1
fi

# insert module now
insert_mod /lib/modules/5.10.117/udc-msb250x.ko
insert_mod /lib/modules/5.10.117/u_ether.ko
insert_mod /lib/modules/5.10.117/libcomposite.ko
insert_mod /lib/modules/5.10.117/usb_f_ecm.ko
insert_mod /lib/modules/5.10.117/usb_f_rndis.ko
insert_mod /lib/modules/5.10.117/g_ether.ko

# config usb0 now
gen_conf
pkill udhcpd
ifconfig usb0 $DEFAULT_IP netmask 255.255.255.0
udhcpd -fS $UDHCPD_CFG_PATH  &
