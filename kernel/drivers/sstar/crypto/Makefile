#
# Copyright (c) [2019~2022] SigmaStar Technology.
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

subdir-ccflags-y := -DSSTAR_CODING_STYLE_SANITIZE_MODULE
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

obj-$(CONFIG_SSTAR_CRYPTO) += drv_crypto.o

ifeq ($(CONFIG_SSTAR_CHIP_NAME), mercury6p)
    EXTRA_CFLAGS += -DSUPPORT_PKA_RSA4096
endif

EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/include
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/crypto
EXTRA_CFLAGS      +=  -I$(srctree)/drivers/sstar/crypto/hal/$(CONFIG_SSTAR_CHIP_NAME)

drv_crypto-objs := drv_aes.o \
               drv_sha.o \
               drv_rsa.o \
               drv_cipher.o \
               hal/$(CONFIG_SSTAR_CHIP_NAME)/halAESDMA.o

obj-$(CONFIG_CRYPTODEV) += cryptodev/
obj-$(CONFIG_SSTAR_RNG) += char/hw_random/ \
                           hal/$(CONFIG_SSTAR_CHIP_NAME)/halAESDMA.o

