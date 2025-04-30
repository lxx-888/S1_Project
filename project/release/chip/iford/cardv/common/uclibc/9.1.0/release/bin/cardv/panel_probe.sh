#!/bin/sh

BL_ON=0x007B
BL_REG_R=`riu_r 3f 01`
BL_VAL=`echo $BL_REG_R|awk '{print $4}'`


if [ "$BL_VAL" = "$BL_ON" ]; then
	echo "BL is turn on. $BL_VAL"
	##touch /tmp/bl_on
else
	echo "BL is turn off."
	touch /tmp/bl_off
fi

