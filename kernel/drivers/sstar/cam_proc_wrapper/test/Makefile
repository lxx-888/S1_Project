#
# Copyright (c) [2019~2020] SigmaStar Technology.
#
#
# This software is licensed under the terms of the GNU General Public
# License version 2, as published by the Free Software Foundation, and
# may be copied, distributed, and modified under those terms.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License version 2 for more details.
#

obj-m       := proc_ktest.o
proc_ktest-objs := ../src/cam_proc_wrapper.o cam_proc_wrapper_test.o

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

KERNEL = $(PWD)/../../../../
CC  = $(CROSS_COMPILE)gcc
APP = cam_proc_wrapper_test
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
EXTRA_CFLAGS := -I$(KERNEL)drivers/sstar/include/ -I$(KERNEL)/drivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME) -I$(KERNEL)/drivers/sstar/cam_proc_wrapper/inc/
all:
	make modules -C $(KERNEL)  M=`pwd`
clean:
	make modules clean -C $(KERNEL)  M=`pwd`
	rm -rf $(APP)

