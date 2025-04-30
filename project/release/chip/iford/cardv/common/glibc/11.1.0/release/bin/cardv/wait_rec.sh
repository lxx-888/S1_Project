#! /bin/sh
var1=0
echo "waiting record"
while true
do

        if [ -e /tmp/cardv_rec ]; then \
                echo "xxxxxxxxxx cardv_rec has created xxxxxxxxxx" ; \
                break ; \
        fi;

        usleep 100000
        var1=`busybox expr $var1 + 1`
        #echo $var1

        if test $var1 -ge 20
        then
                echo "xxxxxxxxxx wait timeout xxxxxxxxxx"
                break
        fi

done

