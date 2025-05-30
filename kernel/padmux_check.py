from __future__ import print_function
from sys import argv
import sys
import os

def main():
    dts_name = argv[1]
    if os.path.exists(dts_name):
        dtsi_file = open(dts_name, 'r')
    else:
        sys.exit(0)
    dtsi_read = dtsi_file.readline()
    while dtsi_read:
        if dtsi_read.find('sstar-padmux') != -1:
            dtsi_read = dtsi_file.readline()
            break
        else:
            dtsi_read = dtsi_file.readline()
    if not dtsi_read :
        sys.exit(0)
    dtsi_read = dtsi_read[dtsi_read.find('<')+1:dtsi_read.find('>')]
    dtsi_read = dtsi_read.split(' ')
    pad_digit = [0 for i in range(int(len(dtsi_read)/3))]
    pad_mode = [0 for i in range(int(len(dtsi_read)/3))]
    mdrv_puse = [0 for i in range(int(len(dtsi_read)/3))]


    pinmux_name = argv[2]
    pinmux_file = open(pinmux_name, 'r')
    pinmux_read = pinmux_file.readlines()
    pinmux_name = [0 for i in range(len(pinmux_read))]
    pinmux_mode = [0 for i in range(len(pinmux_read))]
    pinmux_num = 0
    line_num = 0

    while line_num < len(pinmux_read):
        if pinmux_read[line_num].find('ST_PadMuxInfo m_stPadMuxTbl') != -1:
            line_num = line_num + 1
            try:
                while len(pinmux_read[line_num]) != 1:
                    pad_num = pinmux_read[line_num].lstrip().split('\t')
                    pinmux_name[pinmux_num] = int(pad_num[1][4:],16)
                    mode_val = pinmux_read[line_num+3].lstrip().split('\t')
                    pinmux_mode[pinmux_num] = int(mode_val[1][:4],16)
                    pinmux_num += 1
                    line_num += 4
            except:
                break
        else:
            line_num += 1


    for num in range(len(pad_digit)):
        pad_digit[num] = int(dtsi_read[num * 3],16)
        pad_mode[num] = int(dtsi_read[1 + num * 3],16)
        mdrv_puse[num] = int(dtsi_read[2 + num * 3],16)


    for check_num in range(len(pad_digit)):
        compared_num = check_num + 1
        while compared_num < (len(pad_digit)):
            if pad_digit[check_num] != pad_digit[compared_num]:
                compared_num = compared_num + 1
            else:
                raise IOError('[error] check whether the PadName was reused! the gpio index is :',pad_digit[check_num])


    for check_num in range(len(mdrv_puse)):
        compared_num = check_num + 1
        while compared_num < (len(mdrv_puse)):
            if mdrv_puse[check_num] != mdrv_puse[compared_num]:
                compared_num = compared_num + 1
            else:
                raise Exception('[error] check whether the MDRV_PUSE was reused! the gpio index is :',pad_digit[check_num])


    for check_num in range(len(pad_digit)):
        flag = 0
        if pad_digit[check_num] > max(pinmux_name):
            break
        for pin_compared_num in range(len(pinmux_name)):
            if pad_digit[check_num] == pinmux_name[pin_compared_num]:
                if pad_mode[check_num] == pinmux_mode[pin_compared_num]:
                    flag = 1
            if pin_compared_num == len(pinmux_name)-1 and flag != 1:
                raise Exception('[error] Please check the PadName and ModeVal are matched! the gpio index is :', pad_digit[check_num])


    for check_num in range(len(pad_digit)):
        flag = 0
        if pad_mode[check_num] == 0:
            continue
        for pin_compared_num in range(len(pinmux_name)):
            if pad_mode[check_num] == pinmux_mode[pin_compared_num]:
                if pinmux_name[pin_compared_num] not in pad_digit:
                    raise Exception('[error] Please write all the Pads corresponding to one ModeVal! the gpio index is :', pad_digit[check_num])
                    sys.exit(0)
                else:
                    for Dtsi_Compared_Line in range(len(pad_digit)):
                        if pad_digit[Dtsi_Compared_Line] == pinmux_name[pin_compared_num]:
                            if pad_mode[Dtsi_Compared_Line] != pinmux_mode[pin_compared_num]:
                                flag = 1
                        if flag == 1:
                            raise Exception('[error] Please write all the Pads corresponding to one ModeVal! the gpio index is :', pad_digit[check_num])

    print('  The configuration of GPIO & PADMUX is Success!')
if __name__ == "__main__":
    main()