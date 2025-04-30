#!/bin/sh

##############################
## CGI_CMD
##############################
export PATH=/sbin:/bin:/customer/wifi:/usr/bin:/usr/sbin
export LD_LIBRARY_PATH=/lib:/customer/wifi/lib

SDCARD_PATH=/mnt/mmc/
REC_STATUS=/tmp/rec_status
MMC_BLK=`mount | grep /mnt/mmc | awk '{print $1}'`
VIDEOPARAM=/tmp/cardv_fifo

##with no need for I6E
if [ -f "/config/bin/fw_setenv" ]; then
FW_SETENV=/config/bin/fw_setenv
else
FW_SETENV=echo
fi

RECORDING()
{
	echo "($0): RECORDING $1"
	REC_ING=`cat $REC_STATUS`

	if [ "$1" = "1" ]; then
		if [ "$REC_ING" = "0" ]; then
			echo "ON"
			#echo "1" > $REC_STATUS
			echo "rec 1" > $VIDEOPARAM
		fi
	elif [ "$1" = "0" ]; then
		if [ "$REC_ING" = "1" ]; then
			echo "OFF"
			echo "rec 0" > $VIDEOPARAM
		fi
	elif [ "$1" = "2" ]; then
		if [ "$REC_ING" = "1" ]; then
			echo "OFF"
			echo "rec 0" > $VIDEOPARAM
		else
			echo "ON"
			echo "rec 1" > $VIDEOPARAM
		fi
	else
		echo "none"
	fi
	sync
}

TAKE_PICTURE()
{
	echo "($0): TAKE_PICTURE $1"
	if [ "$1" = "1" ]; then
		echo "ON"
		echo "capture" > $VIDEOPARAM
		sync
	elif [ "$1" = "0" ]; then
		echo "OFF"
	else
		echo "none"
	fi
}

VIDEO_RESOLUTION_FPS()
{
	echo "VIDEO_RESOLUTION_FPS $1"
	case $1 in
		"2160P25fps")
			REC_RES=3840\ 2160
			REC_FPS=30
			;;
		"1440P30fps")
			REC_RES=2560\ 1440
			REC_FPS=30
			;;
		"1080P30fps")
			REC_RES=1920\ 1080
			REC_FPS=30
			;;
		"1080P27.5fpsHDR")
			REC_RES=1920\ 1080
			REC_FPS=27.5
			;;
		"720P30fps")
			REC_RES=1280\ 720
			REC_FPS=30
			;;
		"720P27.5fpsHDR")
			REC_RES=1280\ 720
			REC_FPS=27.5
			;;
		"720P60fps")
			REC_RES=1280\ 720
			REC_FPS=60
			;;
		"VGA")
			REC_RES=640\ 480
			REC_FPS=30
			;;
		*)
			REC_RES=1920\ 1080
			REC_FPS=30
			;;
	esac
	echo "vidres $REC_RES " > $VIDEOPARAM
	nvconf set 0 Camera.Menu.VideoRes $1
}

JPG_RESOLUTION()
{
        echo "JPEG_RESOLUTION $1"
        case $1 in
                "3M")
                        REC_RES=2304\ 1296
                        ;;
                "2M")
                        REC_RES=1920\ 1080
                        ;;
                "1D2M")
                        REC_RES=1280\ 960
                        ;;
                "8M")
                        REC_RES=3840\ 2160
                        ;;
                *)
                        REC_RES=1920\ 1080
                        ;;
        esac
        echo "capres $REC_RES " > $VIDEOPARAM
        nvconf set 0 Camera.Menu.ImageRes $1
}

BRIGHTNESS()
{
	echo "($0): BRIGHTNESS $1"
	VALUE=$1
	if [ $VALUE -gt 100 ];then
		VALUE=100
	elif [ $VALUE -lt 0 ];then
		VALUE=0
	fi
 	echo "bri $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.Brightness $1
}

CONTRAST()
{
	echo "($0): CONTRAST $1"
	VALUE=$1
	if [ $VALUE -gt 100 ];then
		VALUE=100
	elif [ $VALUE -lt -0 ];then
		VALUE=0
	fi
 	echo "con $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.Contrast $1
}

HUE()
{
	echo "($0): HUE $1"
	VALUE=$1
 	echo "hue $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.Hue $1
}

SATURATION()
{
	echo "($0): SATURATION $1"
	VALUE=$1
	if [ $VALUE -gt 127 ];then
		VALUE=100
	elif [ $VALUE -lt 0 ];then
		VALUE=0
	fi
 	echo "sat $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.Saturation $1
}

SHARPNESS()
{
	echo "($0): SHARPNESS $1"
	VALUE=$1
	if [ $VALUE -gt 1023 ];then
		VALUE=100
	elif [ $VALUE -lt 0 ];then
		VALUE=0
	fi
 	echo "sha $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.Sharpness $1
}

GAMMA()
{
	echo "($0): GAMMA $1"
	VALUE=$1
	#VALUE=`expr $VALUE - 128`
	if [ $VALUE -gt 100 ];then
		VALUE=100
	elif [ $VALUE -lt 0 ];then
		VALUE=0
	fi
 	echo "gamma $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.Gamma $1
}

EXPOSURE()
{
	echo "($0): EXPOSURE $1"
	case $1 in
		"EVN200")
			VALUE=-6
			;;
		"EVN167")
			VALUE=-5
			;;
		"EVN133")
			VALUE=-4
			;;
		"EVN100")
			VALUE=-3
			;;
		"EVN67")
			VALUE=-2
			;;
		"EVN33")
			VALUE=-1
			;;
		"EV0")
			VALUE=0
			;;
		"EVP33")
			VALUE=1
			;;
		"EVP67")
			VALUE=2
			;;
		"EVP100")
			VALUE=3
			;;
		"EVP133")
			VALUE=4
			;;
		"EVP167")
			VALUE=5
			;;
		"EVP200")
			VALUE=6
			;;
		*)
			VALUE=0
			;;
	esac
 	echo "ev $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.EV $1
}

EXPOSURE_AUTO()
{
	echo "($0): EXPOSURE_AUTO $1"
	VALUE=$1
 	echo "3a $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.ISO $1
}

ISO()
{
	echo "($0): ISO $1"
	case $1 in
		"ISO_AUTO")
			VALUE=0
			;;
		"ISO_100")
			VALUE=1
			;;
		"ISO_200")
			VALUE=2
			;;
		"ISO_400")
			VALUE=3
			;;
		"ISO_800")
			VALUE=4
			;;
		"ISO_1600")
			VALUE=5
			;;
		"ISO_3200")
			VALUE=6
			;;
		*)
			VALUE=0
			;;
	esac
	echo "iso $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.ISO $1
}

EFFECT()
{
	echo "($0): EFFECT $1"
	case $1 in
		"noraml")
			VALUE=0
			;;
		"sepia")
			VALUE=1
			;;
		"blackwhite")
			VALUE=2
			;;
		"emboss")
			VALUE=3
			;;
		"negative")
			VALUE=3
			;;
		"sketch")
			VALUE=3
			;;
		"oli")
			VALUE=4
			;;
		"crayon")
			VALUE=5
			;;
		"beauty")
			VALUE=6
			;;
		*)
			VALUE=0
			;;
	esac
	echo "effect $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.effect $1
}

FLICKER()
{
	echo "($0): FLICKER $1"
	case $1 in
		"50HZ")
			VALUE=50
			;;
		"60HZ")
			VALUE=60
			;;
		*)
			VALUE=50
			;;
	esac
 	echo "flicker $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.Flicker $1
}

WHITE_BALANCE()
{
	echo "($0): WHITE_BALANCE $1"
	case $1 in
		"Auto")
			VALUE=0
			;;
		"Daylight")
			VALUE=1
			;;
		"Cloudy")
			VALUE=2
			;;
		"Fluorescent1")
			VALUE=3
			;;
		"Fluorescent2")
			VALUE=3
			;;
		"Fluorescent3")
			VALUE=3
			;;
		"Incandescent")
			VALUE=4
			;;
		*)
			VALUE=0
			;;
	esac
 	echo "wb $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.AWB $1
}

SHUTTER_SPEED()
{
	echo "($0): SHUTTER_SPEED $1"
	VALUE=$1
	VALUE=`expr $VALUE - 1`
	VALUE=`expr 100 + $VALUE \* 146`

 	echo "shutter $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.Shutter $1
}

LCDBrightness()
{
	echo "($0) : LCDBRI $1"
	VALUE=$1
	if [ $VALUE -gt 100 ]; then
		VALUE=100
	elif [ $VALUE -lt -0 ]; then
		VALUE=0
	fi
	echo "lcdbri $VALUE" > $VIDEOPARAM
	nvconf set 0 Cemera.Menu.LCDBrightness $1
}

setDateTimeFormat()
{
	case $1 in
		"NONE")
			VALUE=0
			;;
		"YMD")
			VALUE=1
			;;
		"MDY")
			VALUE=2
			;;
		"DMY")
			VALUE=3
			;;
		*)
			VALUE=0
			;;
	esac
	echo "timeformat $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.DateTimeFormat $1
}

setDateLogoStamp()
{
	case $1 in
		"DATELOGO")
			VALUE=0
			;;
		"DATE")
			VALUE=1
			;;
		"LOGO")
			VALUE=2
			;;
		"OFF")
			VALUE=3
			;;
		*)
			VALUE=0
			;;
	esac
	echo "datelogoStamp $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.TimeStampLogoTXT $1
}

setGpsStamp()
{
	case $1 in
		"ON")
			VALUE=0
			;;
		"OFF")
			VALUE=1
			;;
		*)
			VALUE=0
			;;
	esac
	echo "gpsstamp $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.GpsStamp $1
}

setSpeedStamp()
{
	case $1 in
		"ON")
			VALUE=0
			;;
		"OFF")
			VALUE=1
			;;
		*)
			VALUE=0
			;;
	esac
	echo "speedstamp $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.SpeedStamp $1
}

setLanguage()
{
	case $1 in
		"English")
			VALUE=0
			;;
		"Spanish")
			VALUE=1
			;;
		"Portuguese")
			VALUE=2
			;;
		"Russian")
			VALUE=3
			;;
		"Simplified Chinese")
			VALUE=4
			;;
		"Traditional Chinese")
			VALUE=5
			;;
		"German")
			VALUE=6
			;;
		"Italian")
			VALUE=7
			;;
		"Latvian")
			VALUE=8
			;;
		"Polish")
			VALUE=9
			;;
		"Romanian")
			VALUE=10
			;;
		"Slovak")
			VALUE=11
			;;
		"UKRomanian")
			VALUE=12
			;;
		"French")
			VALUE=13
			;;
		"Japanese")
			VALUE=14
			;;
		"Korean")
			VALUE=15
			;;
		"Czech")
			VALUE=16
			;;
		*)
			VALUE=0
			;;
	esac
	#echo "lang $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.Language $1
}

setUsbFunction()
{
	case $1 in
		"MSDC")
			VALUE=0
			;;
		"PCAM")
			VALUE=1
			;;
		*)
			VALUE=0
			;;
	esac
	#echo "usbmode $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.USB $1
}

setLcdPowerSave()
{
	case $1 in
		"OFF")
			VALUE=0
			;;
		"10SEC")
			VALUE=10
			;;
		"30SEC")
			VALUE=30
			;;
		"1MIN")
			VALUE=60
			;;
		"3MIN")
			VALUE=180
			;;
		*)
			VALUE=0
			;;
	esac
	#echo "lcdpwrsave $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.PowerSaving $1
}

setPowerOnGSensor()
{
	case $1 in
		"OFF")
			VALUE=0
			;;
		"LEVEL0")
			VALUE=1
			;;
		"LEVEL1")
			VALUE=2
			;;
		"LEVEL2")
			VALUE=3
			;;
		*)
			VALUE=0
			;;
	esac
	echo "park $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.GSensorPowerOnSens $1
}

setMotionDetect()
{
	case $1 in
		"OFF")
			VALUE=0
			;;
		"LOW")
			VALUE=1
			;;
		"MID")
			VALUE=2
			;;
		"HIGH")
			VALUE=3
			;;
		*)
			VALUE=0
			;;
	esac
	echo "mdt $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.MotionSensitivity $1
}

Camera_System_Power()
{
	echo "($0): Camera_System_Power $1"
}

TimeSettings()
{
	LEN=${#1}

	if [ "$LEN" = "44" ]; then
		echo "LEN=$LEN"

		YYYY=`echo $1 | sed 's/[[:print:]]\{40\}$//' `
		MM=`echo $1   | sed 's/[[:print:]]\{33\}$//'  |sed 's/^[[:print:]]\{9\}//' `
		DD=`echo $1   | sed 's/[[:print:]]\{26\}$//'  |sed 's/^[[:print:]]\{16\}//' `
		hh=`echo $1   | sed 's/[[:print:]]\{19\}$//'  |sed 's/^[[:print:]]\{23\}//'  `
		mm=`echo $1   | sed 's/[[:print:]]\{12\}$//'  |sed 's/^[[:print:]]\{30\}//'  `
		ss=`echo $1   | sed 's/[[:print:]]\{5\}$//'  |sed 's/^[[:print:]]\{37\}//'  `

	elif [ "$LEN" = "38" ]; then
		echo "LEN=$LEN"

		YYYY=`echo $1 | sed 's/[[:print:]]\{34\}$//' `
		MM=`echo $1   | sed 's/[[:print:]]\{28\}$//'  |sed 's/^[[:print:]]\{8\}//' `
		DD=`echo $1   | sed 's/[[:print:]]\{22\}$//'  |sed 's/^[[:print:]]\{14\}//' `
		hh=`echo $1   | sed 's/[[:print:]]\{16\}$//'  |sed 's/^[[:print:]]\{20\}//'  `
		mm=`echo $1   | sed 's/[[:print:]]\{10\}$//'  |sed 's/^[[:print:]]\{26\}//'  `
		ss=`echo $1   | sed 's/[[:print:]]\{4\}$//'  |sed 's/^[[:print:]]\{32\}//'  `
	elif [ "$LEN" = "32" ]; then
		echo "LEN=$LEN"

		YYYY=`echo $1 | sed 's/[[:print:]]\{28\}$//' `
		MM=`echo $1   | sed 's/[[:print:]]\{23\}$//'  |sed 's/^[[:print:]]\{7\}//' `
		DD=`echo $1   | sed 's/[[:print:]]\{18\}$//'  |sed 's/^[[:print:]]\{12\}//' `
		hh=`echo $1   | sed 's/[[:print:]]\{13\}$//'  |sed 's/^[[:print:]]\{17\}//'  `
		mm=`echo $1   | sed 's/[[:print:]]\{8\}$//'  |sed 's/^[[:print:]]\{22\}//'  `
		ss=`echo $1   | sed 's/[[:print:]]\{3\}$//'  |sed 's/^[[:print:]]\{27\}//'  `

	else
		echo "LEN=$LEN"
		echo "no handle this LEN"
	fi

	DATE=$YYYY-$MM-$DD
	TIME=$hh:$mm:$ss
	echo $DATE
	echo $TIME

	date -s "$DATE $TIME" &
	hwclock -w &
}

setbitrate()
{
	echo "($0): setbitrate $1"
 	echo "bitrate $1" > $VIDEOPARAM
}

reset_to_default()
{
 	CMD="resetdefault.sh"
	$CMD
}

REBOOT()
{
	sync
	sleep 1
	echo "restart wifi ..."
	CMD="ap.sh restart"
	$CMD
}

SD_Format()
{
	REC_ING=`cat $REC_STATUS`
	if [ "$REC_ING" = "0" ]; then
		echo "Format SDMMC!"
	else
		echo "Stop rec first!"
		echo "rec 0" > $VIDEOPARAM
		usleep 500000
		echo "Format SDMMC!"
	fi
	echo "format" > $VIDEOPARAM
}

Setting_Update()
{
	case $1 in
		"enter")
			echo "record stop"
			echo "0" > $REC_STATUS
			echo "rec 0" > $VIDEOPARAM
			;;
		"exit")
			echo "record start"
			echo "1" > $REC_STATUS
			echo "rec 1" > $VIDEOPARAM
			;;
	esac
}

setVideoClipTime()
{
	case $1 in
		"OFF")
			echo "loop rec Off"
			VALUE=1
		;;
		"1MIN")
			echo "loop rec 1 MIN"
			VALUE=1
		;;
		"2MIN")
			echo "loop rec 2 MIN"
			VALUE=2
		;;
		"3MIN")
			echo "loop rec 3 MIN"
			VALUE=3
		;;
		"5MIN")
			echo "loop rec 5 MIN"
			VALUE=5
		;;
		"10MIN")
			echo "loop rec 10 MIN"
			VALUE=10
		;;
		"15MIN")
			echo "loop rec 15 MIN"
			VALUE=15
		;;
		*)
			VALUE=1
		;;
	esac
	nvconf set 0 Camera.Menu.LoopingVideo $1
	echo "loop $VALUE" > $VIDEOPARAM
	$FW_SETENV LoopingVideo $VALUE
}

setStillBurstShot()
{
	case $1 in
		"OFF")
			echo "burstshot level off"
			VALUE=0
		;;
		"LO")
			echo "burstshot level low"
			VALUE=1
		;;
		"MID")
			echo "burstshot level middle"
			VALUE=2
		;;
		"HI")
			echo "burstshot level high"
			VALUE=3
		;;
		*)
			VALUE=0
		;;
	esac
	echo "burstshot $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.BurstShot $1
}

setLDWS()
{
	case $1 in
		"OFF")
			echo "adas ldws off"
			VALUE=0
		;;
		"ON")
			echo "adas ldws on"
			VALUE=1
		;;
		*)
			VALUE=0
		;;
	esac
	echo "adas ldws $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Preview.Adas.LDWS $1
}

setFCWS()
{
	case $1 in
		"OFF")
			echo "adas fcws off"
			VALUE=0
		;;
		"ON")
			echo "adas fcws on"
			VALUE=1
		;;
		*)
			VALUE=0
		;;
	esac
	echo "adas fcws $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Preview.Adas.FCWS $1
}

setSAG()
{
	case $1 in
		"OFF")
			echo "adas sag off"
			VALUE=0
		;;
		"ON")
			echo "adas sag on"
			VALUE=1
		;;
		*)
			VALUE=0
		;;
	esac
	echo "adas sag $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Preview.Adas.SAG $1
}

setNightMode()
{
	case $1 in
		"OFF")
			echo "nightmode off"
			VALUE=0
		;;
		"ON")
			echo "nightmode on"
			VALUE=1
		;;
		*)
			VALUE=0
		;;
	esac
	echo "night $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.NightMode $1
}

setWNR()
{
	case $1 in
		"OFF")
			echo "wnr off"
			VALUE=0
		;;
		"ON")
			echo "wnr on"
			VALUE=1
		;;
		*)
			VALUE=0
		;;
	esac
	echo "wnr $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.WNR $1
}

setHDR()
{
	case $1 in
		"OFF")
			echo "hdr off"
			VALUE=0
		;;
		"ON")
			echo "hdr on"
			VALUE=1
		;;
		*)
			VALUE=0
		;;
	esac
	echo "hdr $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.HDR $1
}

setSlowMotion()
{
	case $1 in
		"X1")
			echo "slowmotion X1"
			VALUE=0
		;;
		"X2")
			echo "slowmotion X2"
			VALUE=1
		;;
		"X4")
			echo "slowmotion X4"
			VALUE=2
		;;
		"X8")
			echo "slowmotion X8"
			VALUE=3
		;;
		*)
			VALUE=0
		;;
	esac
	echo "slowmotion $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.SlowMotion $1
}

setTimelapse()
{
	case $1 in
		"OFF")
			echo "timelapse OFF"
			VALUE=0
		;;
		"1SEC")
			echo "timelapse 1SEC"
			VALUE=1
		;;
		"5SEC")
			echo "timelapse 5SEC"
			VALUE=5
		;;
		"10SEC")
			echo "timelapse 10SEC"
			VALUE=10
		;;
		"30SEC")
			echo "timelapse 30SEC"
			VALUE=30
		;;
		"60SEC")
			echo "timelapse 60SEC"
			VALUE=60
		;;
		*)
			VALUE=0
		;;
	esac
	echo "timelapse $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.Timelapse $1
}

setAutoRec()
{
	case $1 in
		"OFF"|"off")
			echo "AutoRec OFF"
			VALUE=1
		;;
		"ON"|"on")
			echo "AutoRec ON"
			VALUE=0
		;;
		*)
			VALUE=0
		;;
	esac
	echo "autorec $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.AutoRec $1
}

setVideoPreRecord()
{
	case $1 in
		"OFF"|"off")
			echo "VideoPreRecord OFF"
			VALUE=1
		;;
		"ON"|"on")
			echo "VideoPreRecord ON"
			VALUE=0
		;;
		*)
			VALUE=0
		;;
	esac
	echo "prerec $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.VideoPreRecord $1
}

setMicSensitivity()
{
	case $1 in
		"STANDARD")
			echo "MicSensitivity STANDARD"
			VALUE=1
		;;
		"LOW")
			echo "MicSensitivity LOW"
			VALUE=0
		;;
		*)
			VALUE=0
		;;
	esac
	echo "micsen $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.MicSensitivity $1
}

setVideoQuality()
{
	case $1 in
		"SUPER_FINE")
			echo "VideoQuality STANDARD"
			VALUE=0
		;;
		"FINE")
			echo "VideoQuality LOW"
			VALUE=1
		;;
		*)
			VALUE=0
		;;
	esac
	echo "quality $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.VideoQuality $1
}

setVideoOffTime()
{
	case $1 in
		"0MIN")
			echo "videoofftime OFF"
			VALUE=0
		;;
		"5SEC")
			echo "videoofftime 5SEC"
			VALUE=5
		;;
		"10SEC")
			echo "videoofftime 10SEC"
			VALUE=10
		;;
		"15SEC")
			echo "videoofftime 15SEC"
			VALUE=15
		;;
		"30SEC")
			echo "videoofftime 30SEC"
			VALUE=30
		;;
		"1MIN")
			echo "videoofftime 1MIN"
			VALUE=60
		;;
		"2MIN")
			echo "videoofftime 2MIN"
			VALUE=120
		;;
		"3MIN")
			echo "videoofftime 3MIN"
			VALUE=180
		;;
		"5MIN")
			echo "videoofftime 5MIN"
			VALUE=300
		;;
		"10MIN")
			echo "videoofftime 10MIN"
			VALUE=600
		;;
		"15MIN")
			echo "videoofftime 15MIN"
			VALUE=900
		;;
		"30MIN")
			echo "videoofftime 30MIN"
			VALUE=1800
		;;
		"60MIN")
			echo "videoofftime 60MIN"
			VALUE=3600
		;;
		*)
			VALUE=0
		;;
	esac
	echo "videoofftime $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.VideoOffTime $1
}

setPlaybackVolume()
{
	case $1 in
		"00")
			echo "pbvolume 00"
			VALUE=0
		;;
		"01")
			echo "pbvolume 01"
			VALUE=1
		;;
		"02")
			echo "pbvolume 02"
			VALUE=2
		;;
		"03")
			echo "pbvolume 03"
			VALUE=3
		;;
		"04")
			echo "pbvolume 04"
			VALUE=4
		;;
		"05")
			echo "pbvolume 05"
			VALUE=5
		;;
		"06")
			echo "pbvolume 06"
			VALUE=6
		;;
		"07")
			echo "pbvolume 07"
			VALUE=7
		;;
		"08")
			echo "pbvolume 08"
			VALUE=8
		;;
		"09")
			echo "pbvolume 09"
			VALUE=9
		;;
		"10")
			echo "pbvolume 10"
			VALUE=10
		;;
		*)
			VALUE=5
		;;
	esac
	echo "pbvolume $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.PlaybackVolume $1
}

setBeep()
{
	case $1 in
		"OFF"|"off")
			echo "Beep OFF"
			VALUE=1
		;;
		"ON"|"on")
			echo "Beep ON"
			VALUE=0
		;;
		*)
			VALUE=0
		;;
	esac
	echo "beep $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.Beep $1
}

setAutoPowerOff()
{
	case $1 in
		"NEVER")
			echo "AutoPowerOff NEVER"
			VALUE=0
		;;
		"15SEC")
			echo "AutoPowerOff 15SEC"
			VALUE=15
		;;
		"30SEC")
			echo "AutoPowerOff 30SEC"
			VALUE=30
		;;
		"1MIN")
			echo "AutoPowerOff 1MIN"
			VALUE=60
		;;
		"2MIN")
			echo "AutoPowerOff 2MIN"
			VALUE=120
		;;
		"3MIN")
			echo "AutoPowerOff 3MIN"
			VALUE=180
		;;
		"5MIN")
			echo "AutoPowerOff 5MIN"
			VALUE=300
		;;
		*)
			VALUE=0
		;;
	esac
	#echo "AutoPowerOff $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.AutoPowerOff $1
}

setSoundRecord()
{
	case $1 in
		"OFF"|"off")
			echo "SoundRecord OFF"
			VALUE=1
		;;
		"ON"|"on")
			echo "SoundRecord ON"
			VALUE=0
		;;
		*)
			VALUE=0
		;;
	esac
	echo "audiorec $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.RecordWithAudio $1
}

setMotionVideoTime()
{
	case $1 in
		"5")
			echo "VMD 5sec"
			VALUE=5
		;;
		"10")
			echo "VMD 10sec"
			VALUE=10
		;;
		"30")
			echo "VMD 30sec"
			VALUE=30
		;;
		"60")
			echo "VMD 60sec"
			VALUE=60
		;;
		*)
			VALUE=0
		;;
	esac
	echo "vmd $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.MotionVideoTime $1
}

setRecStamp()
{
	case $1 in
		"OFF")
			echo "RecStamp OFF"
			VALUE=0
		;;
		"ON")
			echo "RecStamp ON"
			VALUE=1
		;;
		*)
			VALUE=0
		;;
	esac
	echo "recstamp $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.DateTimeFormat $1
}

setSpeedUint()
{
	case $1 in
		"km/h")
			echo "SpeedUint km/h"
			VALUE=0
		;;
		"mph")
			echo "SpeedUint mph"
			VALUE=1
		;;
		*)
			VALUE=0
		;;
	esac
	echo "speeduint $VALUE" > $VIDEOPARAM
}

setSpeedCamAlert()
{
	case $1 in
		"OFF")
			echo "SpeedCamAlert OFF"
			VALUE=0
		;;
		"ON")
			echo "SpeedCamAlert ON"
			VALUE=1
		;;
		*)
			VALUE=0
		;;
	esac
	echo "speedCamAlert $VALUE" > $VIDEOPARAM
}

setSpeedLimitAlert()
{
	case $1 in
		"OFF")
			echo "SpeedLimitAlert OFF"
			VALUE=0
		;;
		"30mph"|"50km/h")
			echo "SpeedLimitAlert $1"
			VALUE=50
		;;
		"35mph"|"60km/h")
			echo "SpeedLimitAlert $1"
			VALUE=60
		;;
		"40mph"|"70km/h")
			echo "SpeedLimitAlert $1"
			VALUE=70
		;;
		"50mph"|"80km/h")
			echo "SpeedLimitAlert $1"
			VALUE=80
		;;
		"55mph"|"90km/h")
			echo "SpeedLimitAlert $1"
			VALUE=90
		;;
		"60mph"|"100km/h")
			echo "SpeedLimitAlert $1"
			VALUE=100
		;;
		"65mph"|"110km/h")
			echo "SpeedLimitAlert $1"
			VALUE=110
		;;
		"75mph"|"120km/h")
			echo "SpeedLimitAlert $1"
			VALUE=120
		;;
		"80mph"|"130km/h")
			echo "SpeedLimitAlert $1"
			VALUE=130
		;;
		"85mph"|"140km/h")
			echo "SpeedLimitAlert $1"
			VALUE=140
		;;
		"90mph"|"150km/h")
			echo "SpeedLimitAlert $1"
			VALUE=150
		;;
		"100mph"|"160km/h")
			echo "SpeedLimitAlert $1"
			VALUE=160
		;;
		"105mph"|"170km/h")
			echo "SpeedLimitAlert $1"
			VALUE=170
		;;
		"110mph"|"180km/h")
			echo "SpeedLimitAlert $1"
			VALUE=180
		;;
		"115mph"|"190km/h")
			echo "SpeedLimitAlert $1"
			VALUE=190
		;;
		"123mph"|"200km/h")
			echo "SpeedLimitAlert $1"
			VALUE=200
		;;
		*)
			VALUE=0
		;;
	esac
	echo "SpeedLimitAlert $VALUE" > $VIDEOPARAM
}

setTimeZone()
{
	case $1 in
		"GMT_M_12")
			echo "TimeZone $1"
			VALUE="GMT-12:00"
		;;
		"GMT_M_11")
			echo "TimeZone $1"
			VALUE="GMT-11:00"
		;;
		"GMT_M_10")
			echo "TimeZone $1"
			VALUE="GMT-10:00"
		;;
		"GMT_M_9")
			echo "TimeZone $1"
			VALUE="GMT-09:00"
		;;
		"GMT_M_8")
			echo "TimeZone $1"
			VALUE="GMT-08:00"
		;;
		"GMT_M_7")
			echo "TimeZone $1"
			VALUE="GMT-07:00"
		;;
		"GMT_M_6")
			echo "TimeZone $1"
			VALUE="GMT-06:00"
		;;
		"GMT_M_5")
			echo "TimeZone $1"
			VALUE="GMT-05:00"
		;;
		"GMT_M_4")
			echo "TimeZone $1"
			VALUE="GMT-04:00"
		;;
		"GMT_M_3_30")
			echo "TimeZone $1"
			VALUE="GMT-03:30"
		;;
		"GMT_M_3")
			echo "TimeZone $1"
			VALUE="GMT-03:00"
		;;
		"GMT_M_2")
			echo "TimeZone $1"
			VALUE="GMT-02:00"
		;;
		"GMT_M_1")
			echo "TimeZone $1"
			VALUE="GMT-01:00"
		;;
		"GMT00")
			echo "TimeZone $1"
			VALUE="GMT-00:00"
		;;
		"GMT_P_1")
			echo "TimeZone $1"
			VALUE="GMT+01:00"
		;;
		"GMT_P_2")
			echo "TimeZone $1"
			VALUE="GMT+02:00"
		;;
		"GMT_P_3")
			echo "TimeZone $1"
			VALUE="GMT+03:00"
		;;
		"GMT_P_3_30")
			echo "TimeZone $1"
			VALUE="GMT+03:30"
		;;
		"GMT_P_4")
			echo "TimeZone $1"
			VALUE="GMT+04:00"
		;;
		"GMT_P_4_30")
			echo "TimeZone $1"
			VALUE="GMT+04:30"
		;;
		"GMT_P_5")
			echo "TimeZone $1"
			VALUE="GMT+05:00"
		;;
		"GMT_P_5_30")
			echo "TimeZone $1"
			VALUE="GMT+05:30"
		;;
		"GMT_P_5_45")
			echo "TimeZone $1"
			VALUE="GMT+05:45"
		;;
		"GMT_P_6")
			echo "TimeZone $1"
			VALUE="GMT+06:00"
		;;
		"GMT_P_6_30")
			echo "TimeZone $1"
			VALUE="GMT+06:30"
		;;
		"GMT_P_7")
			echo "TimeZone $1"
			VALUE="GMT+07:00"
		;;
		"GMT_P_8")
			echo "TimeZone $1"
			VALUE="GMT+08:00"
		;;
		"GMT_P_9")
			echo "TimeZone $1"
			VALUE="GMT+09:00"
		;;
		"GMT_P_9_30")
			echo "TimeZone $1"
			VALUE="GMT+09:30"
		;;
		"GMT_P_10")
			echo "TimeZone $1"
			VALUE="GMT+10:00"
		;;
		"GMT_P_11")
			echo "TimeZone $1"
			VALUE="GMT+11:00"
		;;
		"GMT_P_12")
			echo "TimeZone $1"
			VALUE="GMT+12:00"
		;;
		"GMT_P_13")
			echo "TimeZone $1"
			VALUE="GMT+13:00"
		;;
		"GMT_P_14")
			echo "TimeZone $1"
			VALUE="GMT+14:00"
		;;
		*)
			VALUE="GMT-00:00"
		;;
	esac
	echo "timezone $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.TimeZone $VALUE
}

setSyncTime()
{
	case $1 in
		"OFF")
			echo "SyncTime OFF"
			VALUE=0
		;;
		"ON")
			echo "SyncTime ON"
			VALUE=1
		;;
		*)
			VALUE=0
		;;
	esac
	echo "synctime $VALUE" > $VIDEOPARAM
}

setPosSetting_Add()
{
	echo "setPosSetting_Add"
	echo "setPosSetting 0" > $VIDEOPARAM
}

setPosSetting_DelLast()
{
	echo "PosSetting_DelLast"
	echo "setPosSetting 1" > $VIDEOPARAM
}

setPosSetting_DelAll()
{
	echo "PosSetting_DelAll"
	echo "setPosSetting 2" > $VIDEOPARAM
}

setParkingMonitor()
{
	case $1 in
		"DISABLE")
			echo "ParkingMonitor OFF"
			VALUE=0
		;;
		"ENABLE")
			echo "ParkingMonitor ON"
			VALUE=1
		;;
		*)
			VALUE=0
		;;
	esac
	echo "park $VALUE" > $VIDEOPARAM
	$FW_SETENV ParkingMonitor $VALUE
}
setVoiceSwitch()
{
	case $1 in
		"OFF")
			echo "VoiceSwitch OFF"
			VALUE=0
		;;
		"ON")
			echo "VoiceSwitch ON"
			VALUE=1
		;;
		*)
			VALUE=0
		;;
	esac
	echo "voice $VALUE" > $VIDEOPARAM
	$FW_SETENV VoiceSwitch $VALUE
}
setGSensor()
{
	case $1 in
		"OFF")
			##echo "GSensorSensitivity OFF"
			VALUE=0
		;;
		"LEVEL0")
			##echo "GSensorSensitivity U-LOW"
			VALUE=1
		;;
		"LEVEL1")
			##echo "GSensorSensitivity LOW"
			VALUE=2
		;;
		"LEVEL2")
			##echo "GSensorSensitivity MID"
			VALUE=3
		;;
		"LEVEL3")
			##echo "GSensorSensitivity HIGH"
			VALUE=4
		;;
		"LEVEL4")
			##echo "GSensorSensitivity U-HIGH"
			VALUE=5
		;;
		*)
			VALUE=0
		;;
	esac
	echo "gsensor $VALUE" > $VIDEOPARAM
	nvconf set 0 Camera.Menu.GSensorSensitivity $1
	$FW_SETENV GSensor $VALUE
}

reboot_system()
{
 	CMD="rebootsystem.sh"
	$CMD
}

SET()
{
	echo "($0): SET $1 $2 "
	case $1 in
  		"Camera.Preview.MJPEG.TimeStamp")
			TimeSettings $2
			;;
  		"Net")
  			if [ "$2" = "reset" ];then
				REBOOT
			elif [ "$2" = "findme" ];then
				echo $@
			fi
			;;
  		"Net.WIFI_AP.SSID")
 			CMD="nvconf set 1 wireless.ap.ssid $2"
			$CMD
			;;
  		"Net.WIFI_AP.CryptoKey")
 			CMD="nvconf set 1 wireless.ap.wpa.psk $2"
 			$CMD
			;;
  		"Net.WIFI_STA.AP.2.SSID")
 			CMD="nvconf set 1 wireless.sta.ssid $2"
 			$CMD
			;;
  		"Net.WIFI_STA.AP.2.CryptoKey")
 			CMD="nvconf set 1 wireless.sta.wpa.psk $2"
 			$CMD
			;;
		"Net.WIFI_STA.AP.Switch")
			echo "$1:$2"
			if [ "$2" = "ENABLE" ]; then
				apsta_switch.sh
			fi
			;;
  		"Video")
			if [ "$2" = "recordon" ];then

				if [ "$MMC_BLK" = "" ]; then
					exit 1 ## return system() return value
				else
					RECORDING 1
				fi
			elif [ "$2" = "recordoff" ];then

				if [ "$MMC_BLK" = "" ]; then
					exit 1 ## return system() return value
				else
					RECORDING 0
				fi
			elif [ "$2" = "record" ];then

				if [ "$MMC_BLK" = "" ]; then
					exit 1 ## return system() return value
                        	else
					RECORDING 2
				fi
			elif [ "$2" = "capture" ];then

				if [ "$MMC_BLK" = "" ]; then
					exit 1 ## system() return value
				else
					TAKE_PICTURE 1
				fi
			else
				echo $@
			fi
			;;
  		"Imageres"|"ImageRes")
			JPG_RESOLUTION $2
			;;
		"Videores"|"VideoRes")
			VIDEO_RESOLUTION_FPS $2
			;;
		"LcdPowerSave")
			setLcdPowerSave $2
			;;
		"DateTimeFormat")
			setDateTimeFormat $2
			;;
		"DateLogoStamp")
			setDateLogoStamp $2
			;;
		"GpsStamp")
			setGpsStamp $2
			;;
		"SpeedStamp")
			setSpeedStamp $2
			;;
		"Language")
			setLanguage $2
			;;
		"UsbFunction")
			setUsbFunction $2
			;;
		"PowerOnGSensor")
			setPowerOnGSensor $2
			;;
		"MotionDetect")
			setMotionDetect $2
			;;
  		"Brightness")
			BRIGHTNESS $2
			;;
  		"Contrast")
			CONTRAST $2
			;;
  		"Hue")
			HUE $2
			;;
  		"Saturation")
			SATURATION $2
			;;
  		"Sharpness")
			SHARPNESS $2
			;;
  		"Gamma")
			GAMMA $2
			;;
  		"EV"|"Exposure")
			EXPOSURE $2
			;;
  		"AE")
			EXPOSURE_AUTO $2
			;;
		"ISO")
			ISO $2
			;;
		"Effect")
			EFFECT $2
			;;
  		"Flicker")
			FLICKER $2
			;;
  		"AWB")
			WHITE_BALANCE $2
			;;
  		"Shutter")
			SHUTTER_SPEED $2
			;;
  		"Camera.System.Power")
			Camera_System_Power $2
			;;
  		"TimeSettings")
			TimeSettings $2
			;;
  		"setbitrate")
			setbitrate $2
			;;
  		"FactoryReset"|"reset_to_default")
			reset_to_default $2
			;;
  		"reboot")
			REBOOT
			;;
		"SD0")
			SD_Format
			;;
		"Setting")
			Setting_Update $2
			;;
		"VideoClipTime"|"LoopingVideo")
			setVideoClipTime $2
			;;
		"StillBurstShot")
			setStillBurstShot $2
			;;
		"LDWS")
			setLDWS $2
			;;
		"FCWS")
			setFCWS $2
			;;
		"SAG")
			setSAG $2
			;;
		"NightMode")
			setNightMode $2
			;;
		"WNR")
			setWNR $2
			;;
		"HDR")
			setHDR $2
			;;
		"SlowMotion")
			setSlowMotion $2
			;;
		"Timelapse")
			setTimelapse $2
			;;
		"AutoRec")
			setAutoRec $2
			;;
		"VideoOffTime")
			setVideoOffTime $2
			;;
		"VideoPreRecord")
			setVideoPreRecord $2
			;;
		"MicSensitivity")
			setMicSensitivity $2
			;;
		"VideoQuality")
			setVideoQuality $2
			;;
		"PlaybackVolume")
			setPlaybackVolume $2
			;;
		"Beep")
			setBeep $2
			;;
		"LCDBrightness")
			LCDBrightness $2
			;;
		"AutoPowerOff")
			setAutoPowerOff $2
			;;
		"SoundRecord"|"MovieAudio")
			setSoundRecord $2
			;;
		"MotionVideoTime")
			setMotionVideoTime $2
			;;
		"RecStamp")
			setRecStamp $2
			;;
		"SpeedUint")
			setSpeedUint $2
			;;
		"SpeedCamAlert")
			setSpeedCamAlert $2
			;;
		"SpeedLimitAlert")
			SpeedLimitAlert $2
			;;
		"TimeZone")
			setTimeZone $2
			;;
		"SyncTime")
			setSyncTime $2
			;;
		"PosSetting_Add")
			setPosSetting_Add $2
			;;
		"PosSetting_DelLast")
			setPosSetting_DelLast $2
			;;
		"PosSetting_DelAll")
			setPosSetting_DelAll $2
			;;
		"ParkingMonitor")
			setParkingMonitor $2
			;;
		"VoiceSwitch")
			setVoiceSwitch $2
			;;
		"GSensor")
			setGSensor $2
			;;
		"RebootSystem")
			reboot_system $2
			;;
  		"")
			echo "You MUST input parameters, ex> {$Para0 someword}"
			;;
  		*)
			echo "Usage $Para0 {no this parameter}"
			;;

	esac
}

GET()
{
	#echo "($0): GET $1 "
	case $1 in
		"Camera.Menu.SDInfo")
			if [ "$MMC_BLK" = "" ]; then
				echo "$1=OFF"
			else
				echo "$1=ON"
			fi
			;;
		"Camera.Menu.CardInfo.*")
			if [ "$MMC_BLK" = "" ]; then
				echo "$1=NONE"
			else
				echo "Camera.Menu.CardInfo.LifeTimeTotal=$(df -h /mnt/mmc/ |sed -n '2p' |awk '{print $2}')"
				echo "Camera.Menu.CardInfo.RemainLifeTime=$(df -h /mnt/mmc/ |sed -n '2p' |awk '{print $4}')"
				str=$(cat /sys/devices/soc0/soc/soc:sdmmc/mmc_host/mmc0/mmc0:1388/cardlife)
				RemainWrGBNumInfo=$(echo ${str#*,})
				SizeOfDevSMARTInfo=$(echo ${str%,*})
				echo "Camera.Menu.CardInfo.RemainWrGBNum=$RemainWrGBNumInfo"
				echo "Camera.Menu.CardInfo.SizeOfDevSMART=$SizeOfDevSMARTInfo"
			fi
			;;
  		"Camera.Preview.MJPEG.TimeStamp")
 			CMD="date +"%d""
			echo "Camera.Preview.MJPEG.TimeStamp.day=`$CMD`"
 			CMD="date +"%H""
			echo "Camera.Preview.MJPEG.TimeStamp.hour=`$CMD`"
 			CMD="date +"%M""
			echo "Camera.Preview.MJPEG.TimeStamp.minute=`$CMD`"
 			CMD="date +"%m""
			echo "Camera.Preview.MJPEG.TimeStamp.month=`$CMD`"
 			CMD="date +"%S""
			echo "Camera.Preview.MJPEG.TimeStamp.second=`$CMD`"
 			CMD="date +"%Y""
			echo "Camera.Preview.MJPEG.TimeStamp.year=`$CMD`"
			;;
  		"Camera.Preview.MJPEG.TimeStamp.*")
 			CMD="date +"%d""
			echo "Camera.Preview.MJPEG.TimeStamp.day=`$CMD`"
 			CMD="date +"%H""
			echo "Camera.Preview.MJPEG.TimeStamp.hour=`$CMD`"
 			CMD="date +"%M""
			echo "Camera.Preview.MJPEG.TimeStamp.minute=`$CMD`"
 			CMD="date +"%m""
			echo "Camera.Preview.MJPEG.TimeStamp.month=`$CMD`"
 			CMD="date +"%S""
			echo "Camera.Preview.MJPEG.TimeStamp.second=`$CMD`"
 			CMD="date +"%Y""
			echo "Camera.Preview.MJPEG.TimeStamp.year=`$CMD`"
			;;
  		"Net.WIFI_AP.SSID")
 			CMD="nvconf get 1 wireless.ap.ssid"
			echo "Net.WIFI_AP.SSID=`$CMD`"
			;;
  		"Net.WIFI_AP.CryptoKey")
 			CMD="nvconf get 1 wireless.ap.wpa.psk"
			echo "Net.WIFI_AP.CryptoKey=`$CMD`"
			;;
  		"Net.WIFI_STA.AP.2.SSID")
 			CMD="nvconf get 1 wireless.sta.ssid"
			echo "Net.WIFI_STA.AP.2.SSID=`$CMD`"
			;;
  		"Net.WIFI_STA.AP.2.CryptoKey")
 			CMD="nvconf get 1 wireless.sta.wpa.psk"
			echo "Net.WIFI_STA.AP.2.CryptoKey=`$CMD`"
			;;
		"Net.WIFI_STA.AP.Switch")
			;;
  		"Camera.Preview.MJPEG.status.*")
			REC_ING=`cat $REC_STATUS`
			if [ "$REC_ING" = "1" ]; then
				OUTPUT="Recording"
			else
				OUTPUT="Standby"
			fi
			echo "Camera.Preview.MJPEG.status.mode=Videomode"
			echo "Camera.Preview.MJPEG.status.record=$OUTPUT"
			;;
		"Camera.Preview.Source.1.Camid")
			;;
		"Camera.Preview.Adas.*")
			;;
  		"Videores")
			CMD="nvconf get 0 Camera.Menu.VideoRes"
			echo "$1=$CMD"
			;;
  		"Brightness")
			CMD="nvconf get 0 Camera.Menu.Brightness"
			echo "$1=$CMD"
			;;
  		"Contrast")
 			CMD="nvconf get 0 Camera.Menu.Contrast"
			echo "$1=$CMD"
			;;
  		"Hue")
			CMD="nvconf get 0 Camera.Menu.Hue"
			echo "$1=$CMD"
			;;
  		"Saturation")
			CMD="nvconf get 0 Camera.Menu.Saturation"
			echo "$1=$CMD"
			;;
  		"Sharpness")
			CMD="nvconf get 0 Camera.Menu.Sharpness"
			echo "$1=$CMD"
			;;
  		"Gamma")
			CMD="nvconf get 0 Camera.Menu.gamma"
			echo "$1=$CMD"
			;;
  		"EV")
			CMD="nvconf get 0 Camera.Menu.EV"
			echo "$1=$CMD"
			;;
  		"AE")
			CMD="nvconf get 0 Camera.Menu.AE"
			echo "$1=$CMD"
			;;
		"Flicker")
			CMD="nvconf get 0 Camera.Menu.Flicker"
			echo "$1=$CMD"
			;;
  		"AWB")
			CMD="nvconf get 0 Camera.Menu.AWB"
			echo "$1=$CMD"
			;;
  		"Shutter")
			CMD="nvconf get 0 Camera.Menu.Shutter"
			echo "$1=$CMD"
			;;
		"FwVer")
			CMD="nvconf get 1 devinfo.fwver"
			echo "$1=`$CMD`"
			;;
  		"StillBurstShot")
			CMD="nvconf get 0 Camera.Menu.BurstShot"
			echo "$1=$CMD"
			;;
  		"LDWS")
			CMD="nvconf get 0 Camera..Preview.Adas.LDWS"
			echo "$1=$CMD"
			;;
  		"FCWS")
			CMD="nvconf get 0 Camera..Preview.Adas.FCWS"
			echo "$1=$CMD"
			;;
  		"SAG")
			CMD="nvconf get 0 Camera..Preview.Adas.SAG"
			echo "$1=$CMD"
			;;
  		"NightMode")
			CMD="nvconf get 0 Camera.Menu.NightMode"
			echo "$1=$CMD"
			;;
  		"WNR")
			CMD="nvconf get 0 Camera.Menu.WNR"
			echo "$1=$CMD"
			;;
		"HDR")
			CMD="nvconf get 0 Camera.Menu.HDR"
			echo "$1=$CMD"
			;;
		"SlowMotion")
			CMD="nvconf get 0 Camera.Menu.SlowMotion"
			echo "$1=$CMD"
			;;
		"Timelapse")
			CMD="nvconf get 0 Camera.Menu.Timelapse"
			echo "$1=$CMD"
			;;
		"AutoRec")
			CMD="nvconf get 0 Camera.Menu.AutoRec"
			echo "$1=$CMD"
			;;
		"VideoOffTime")
			CMD="nvconf get 0 Camera.Menu.VideoOffTime"
			echo "$1=$CMD"
			;;
		"VideoPreRecord")
			CMD="nvconf get 0 Camera.Menu.VideoPreRecord"
			echo "$1=$CMD"
			;;
		"VideoClipTime")
			CMD="nvconf get 0 Camera.Menu.VideoClipTime"
			echo "$1=$CMD"
			;;
		"MicSensitivity")
			CMD="nvconf get 0 Camera.Menu.MicSensitivity"
			echo "$1=$CMD"
			;;
		"VideoQuality")
			CMD="nvconf get 0 Camera.Menu.VideoQuality"
			echo "$1=$CMD"
			;;
		"SoundRecord")
			CMD="nvconf get 0 Camera.Menu.RecordWithAudio"
			echo "$1=$CMD"
			;;
		"ParkingMonitor")
			CMD="nvconf get 0 Camera.Menu.ParkingMonitor"
			echo "$1=$CMD"
			;;
		"MotionVideoTime")
			CMD="nvconf get 0 Camera.Menu.MotionVideoTime"
			echo "$1=$CMD"
			;;
		"PlaybackVolume")
			CMD="nvconf get 0 Camera.Menu.PlaybackVolume"
			echo "$1=$CMD"
			;;
		"Beep")
			CMD="nvconf get 0 Camera.Menu.Beep"
			echo "$1=$CMD"
			;;
		"AutoPowerOff")
			CMD="nvconf get 0 Camera.Menu.AutoPowerOff"
			echo "$1=$CMD"
			;;
		"DateTimeFormat")
			CMD="nvconf get 0 Camera.Menu.DateTimeFormat"
			echo "$1=$CMD"
			;;
		"DateLogoStamp")
			CMD="nvconf get 0 Camera.Menu.DateLogoStamp"
			echo "$1=$CMD"
			;;
		"GpsStamp")
			CMD="nvconf get 0 Camera.Menu.GpsStamp"
			echo "$1=$CMD"
			;;
		"SpeedStamp")
			CMD="nvconf get 0 Camera.Menu.SpeedStamp"
			echo "$1=$CMD"
			;;
		"Language")
			CMD="nvconf get 0 Camera.Menu.Language"
			echo "$1=$CMD"
			;;
		"UsbFunction")
			CMD="nvconf get 0 Camera.Menu.USB"
			echo "$1=$CMD"
			;;
		"LcdPowerSave")
			CMD="nvconf get 0 Camera.Menu.PowerSaving"
			echo "$1=$CMD"
			;;
		"GSensor")
			CMD="nvconf get 0 Camera.Menu.GSensorSensitivity"
			echo "$1=$CMD"
			;;
		"PowerOnGSensor")
			CMD="nvconf get 0 Camera.Menu.GSensorPowerOnSens"
			echo "$1=$CMD"
			;;
		"MotionDetect")
			CMD="nvconf get 0 Camera.Menu.MotionSensitivity"
			echo "$1=$CMD"
			;;
		"TimeZone")
			CMD="nvconf get 0 Camera.Menu.TimeZone"
			echo "$1=$CMD"
			;;
  		"")
			echo "You MUST input parameters, ex> {$Para0 someword}"
			;;
  		*)
			echo "Usage $Para0 {no this parameter}"
			;;
	esac
}

DEL()
{
	echo "($0): DEL $1 "
	echo "rm $1" > $VIDEOPARAM
}

Para0=$0
Para1=$1
Para2=$2
Para3=$3
Para4=$4
Para5=$5
Para6=$6

case $Para1 in
  "set")
	SET $Para2 $Para3
	;;
  "get")
	GET $Para2
	;;
  "del")
	DEL $Para2
	;;
  "")
	echo "You MUST input parameters, ex> {$Para0 someword}"
	;;
  *)
	echo "Usage $Para0 {no this parameter}"
	;;
esac


exit 0
