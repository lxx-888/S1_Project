# SigmaStar trade secret
# Copyright (c) [2019~2020] SigmaStar Technology.
# All rights reserved.
#
# Unless otherwise stipulated in writing, any and all information contained
# herein regardless in any format shall remain the sole proprietary of
# SigmaStar and be kept in strict confidence
# (SigmaStar Confidential Information) by the recipient.
# Any unauthorized act including without limitation unauthorized disclosure,
# copying, use, reproduction, sale, distribution, modification, disassembling,
# reverse engineering and compiling of the contents of SigmaStar Confidential
# Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
# rights to any and all damages, losses, costs and expenses resulting therefrom.
#
.PHONY: rootfs

rootfs_$(PRODUCT):
rootfs: rootfs_$(PRODUCT)

LFS_MOUNT_BLK := $(foreach m, $(filter %_lfs, $(foreach block, $(USR_MOUNT_BLOCKS), $(block)_$($(block)$(FSTYPE)))), $(patsubst %_lfs, %, $(m)))
FWFS_MOUNT_BLK := $(foreach m, $(filter %_fwfs, $(foreach block, $(USR_MOUNT_BLOCKS), $(block)_$($(block)$(FSTYPE)))), $(patsubst %_fwfs, %, $(m)))
MOUNT_BLK := $(filter-out $(LFS_MOUNT_BLK), $(USR_MOUNT_BLOCKS))
MOUNT_BLK := $(filter-out $(FWFS_MOUNT_BLK), $(MOUNT_BLK))

ifeq ($(TOOLCHAIN), uclibc)
LIBC=$(strip $(patsubst %.tar.gz, %, $(word 1, $(notdir $(wildcard $(LIB_DIR_PATH)/package/libuclibc-*.tar.gz)))))
else
LIBC=$(strip $(patsubst %.tar.gz, %, $(word 1, $(notdir $(wildcard $(LIB_DIR_PATH)/package/libc-*.tar.gz)))))
endif

rootfs_$(PRODUCT):
	@echo [================= $@ =================]

	# pack rootfs
	cd rootfs;tar xf rootfs.tar.gz -C $(OUTPUTDIR)

	# pack busybox
	tar xf busybox/$(BUSYBOX).tar.gz -C $(OUTPUTDIR)/rootfs

	# pack libc
	tar xf $(LIB_DIR_PATH)/package/$(LIBC).tar.gz -C $(OUTPUTDIR)/rootfs/lib

	# pack mnt
	mkdir -p  $(OUTPUTDIR)/rootfs/mnt/nfs
	mkdir -p  $(OUTPUTDIR)/rootfs/mnt/mmc

	# pack UI config
	cp -rf $(LIB_DIR_PATH)/bin/UI/etc/EasyUI.cfg ${OUTPUTDIR}/rootfs/etc/
	cp -rf $(LIB_DIR_PATH)/bin/UI/etc/ts.conf ${OUTPUTDIR}/rootfs/etc/	

	# pack base scripts
	cp -rf etc/* $(OUTPUTDIR)/rootfs/etc
	if [ -d $(PROJ_ROOT)/board/$(CHIP)/dspfile ]; then \
		mkdir -p  $(OUTPUTDIR)/rootfs/lib/firmware; \
		cp  $(PROJ_ROOT)/board/$(CHIP)/dspfile/* $(OUTPUTDIR)/rootfs/lib/firmware; \
	fi;
	# pack fw_printenv
	cp -rvf $(PROJ_ROOT)/tools/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/fw_printenv/* $(OUTPUTDIR)/rootfs/etc/
	echo "$(ENV_CFG)" > $(OUTPUTDIR)/rootfs/etc/fw_env.config
	cd $(OUTPUTDIR)/rootfs/etc/;ln -sf fw_printenv fw_setenv;chmod 777 fw_printenv

	# set env
	echo export PATH=\$$PATH:/config:/customer/cardv:/customer/UI/bin >> ${OUTPUTDIR}/rootfs/etc/profile
	echo export TERMINFO=/config/terminfo >> ${OUTPUTDIR}/rootfs/etc/profile
	echo export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/config/lib:/customer/UI/lib >> ${OUTPUTDIR}/rootfs/etc/profile

	# mount block
	echo mount -t tmpfs mdev /dev >> ${OUTPUTDIR}/rootfs/etc/init.d/rcS
	$(foreach block, $(MOUNT_BLK), mkdir -p $(OUTPUTDIR)/rootfs/$($(block)$(MOUNTTG));)
	$(foreach block, $(LFS_MOUNT_BLK), mkdir -p $(OUTPUTDIR)/rootfs/$($(block)$(MOUNTTG));)
	$(foreach block, $(FWFS_MOUNT_BLK), mkdir -p $(OUTPUTDIR)/rootfs/$($(block)$(MOUNTTG));)
	echo -e mdev -s >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	if [ "$(FLASH_TYPE)"x = "emmc"x  ]; then \
		if [[ $(TOOLCHAIN_VERSION) = "10.2.1" ]]; then \
			cp -f $(PROJ_ROOT)/tools/glibc/$(TOOLCHAIN_VERSION)/mkfs.ext4 $(OUTPUTDIR)/rootfs/sbin/ ;\
			cp -f $(PROJ_ROOT)/tools/glibc/$(TOOLCHAIN_VERSION)/resize2fs $(OUTPUTDIR)/rootfs/sbin/ ;\
		else \
			cp -f $(PROJ_ROOT)/tools/mkfs.ext4 $(OUTPUTDIR)/rootfs/sbin/ ;\
			cp -f $(PROJ_ROOT)/tools/resize2fs $(OUTPUTDIR)/rootfs/sbin/ ;\
		fi; \
	fi;
	if [ $(PZ1) != 1  ]; then \
		echo -e $(foreach block, $(MOUNT_BLK), "mount -t $($(block)$(FSTYPE)) $($(block)$(MOUNTPT)) $($(block)$(MOUNTTG))\n") >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS ;\
	fi;
	if [ "$(LFS_MOUNT_BLK)" != "" ]; then \
		if [ "$(TOOLCHAIN)" = "glibc" ] && [ "$(TOOLCHAIN_VERSION)" = "10.2.1" ]; then \
			tar -vxf $(PROJ_ROOT)/image/fuse/fuse-2.9.9-aarch64-linux-glibc-10.2.1.tar.gz -C $(OUTPUTDIR)/rootfs/;	\
			tar -vxf $(PROJ_ROOT)/image/firmwarefs-fuse/firmwarefs-fuse-2.2.0-aarch64-linux-gnu-glibc-9.2.1.tar.gz -C $(OUTPUTDIR)/rootfs/;	\
		fi;	\
		if [ "$(TOOLCHAIN)" = "glibc" ] && [ "$(TOOLCHAIN_VERSION)" = "9.1.0" ]; then \
			tar -vxf $(PROJ_ROOT)/image/fuse/fuse-2.9.9-arm-linux-gnueabihf-glibc-9.1.0.tar.gz -C $(OUTPUTDIR)/rootfs/;	\
			tar -vxf $(PROJ_ROOT)/image/littlefs-fuse/littlefs-fuse-2.2.0-arm-linux-gnueabihf-glibc-9.1.0.tar.gz -C $(OUTPUTDIR)/rootfs/;	\
		fi;	\
		if [ "$(TOOLCHAIN)" = "glibc" ] && [ "$(TOOLCHAIN_VERSION)" = "8.2.1" ]; then \
			tar -vxf $(PROJ_ROOT)/image/fuse/fuse-2.9.9-arm-linux-gnueabihf-glibc-8.2.1.tar.gz -C $(OUTPUTDIR)/rootfs/;	\
			tar -vxf $(PROJ_ROOT)/image/littlefs-fuse/littlefs-fuse-2.2.0-arm-linux-gnueabihf-glibc-8.2.1.tar.gz -C $(OUTPUTDIR)/rootfs/;	\
		fi;	\
		mkdir -p $(OUTPUTDIR)/rootfs/misc;\
		$(foreach block, $(LFS_MOUNT_BLK), $(PYTHON) $(PROJ_ROOT)/image/makefiletools/script/lfs_mount.py --part_size=$($(block)$(PATSIZE)) --rcs_dir=$(OUTPUTDIR)/rootfs/etc/init.d/rcS $($(block)$(MOUNTPT)) $($(block)$(MOUNTTG));)\
	fi;
	if [ "$(FWFS_MOUNT_BLK)" != "" ] && [ "$(rtos$(RESOURCE))" != "" ]; then \
		if [ "$(ARCH)" = "arm64" ] && [ "$(TOOLCHAIN)" = "glibc" ] && [ "$(TOOLCHAIN_VERSION)" = "10.2.1" ]; then \
			tar -vxf $(PROJ_ROOT)/image/fuse/fuse-2.9.9-aarch64-linux-glibc-10.2.1.tar.gz -C $(OUTPUTDIR)/rootfs/;	\
			tar -vxf $(PROJ_ROOT)/image/firmwarefs-fuse/firmwarefs-fuse-2.2.0-aarch64-linux-gnu-glibc-9.2.1.tar.gz -C $(OUTPUTDIR)/rootfs/;	\
		fi;	\
		if [ "$(TOOLCHAIN)" = "glibc" ] && [ "$(TOOLCHAIN_VERSION)" != "" ]; then \
			tar -vxf $(PROJ_ROOT)/image/fuse/fuse-2.9.9-arm-linux-gnueabihf-glibc-$(TOOLCHAIN_VERSION).tar.gz -C $(OUTPUTDIR)/rootfs/;	\
			tar -vxf $(PROJ_ROOT)/image/firmwarefs-fuse/firmwarefs-fuse-2.2.0-arm-linux-gnueabihf-glibc-$(TOOLCHAIN_VERSION).tar.gz -C $(OUTPUTDIR)/rootfs/;	\
		fi;	\
		if [ "$(TOOLCHAIN)" = "uclibc" ] && [ "$(TOOLCHAIN_VERSION)" != "" ]; then \
			tar -vxf $(PROJ_ROOT)/image/fuse/fuse-2.9.9-arm-sigmastar-linux-uclibcgnueabihf-$(TOOLCHAIN_VERSION).tar.gz -C $(OUTPUTDIR)/rootfs/;	\
			tar -vxf $(PROJ_ROOT)/image/firmwarefs-fuse/firmwarefs-fuse-2.2.0-arm-sigmastar-linux-uclibcgnueabihf-$(TOOLCHAIN_VERSION).tar.gz -C $(OUTPUTDIR)/rootfs/;	\
		fi;	\
		mkdir -p $(OUTPUTDIR)/rootfs/misc;\
		$(foreach block, $(FWFS_MOUNT_BLK), $(PYTHON) $(PROJ_ROOT)/image/makefiletools/script/fwfs_mount.py --flash_type=$(FLASH_TYPE) --block_size=$(FLASH_BLK_SIZE) --page_size=$(FLASH_PG_SIZE) --part_size=$($(block)$(PATSIZE)) --rcs_dir=$(OUTPUTDIR)/rootfs/etc/init.d/rcS $($(block)$(MOUNTPT)) $($(block)$(MOUNTTG));)\
	fi;

	# telnet
	echo mkdir -p /dev/pts >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	echo mount -t devpts devpts /dev/pts >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS

	# echo export PATH=\$$PATH:/config:/customer/cardv >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	# echo export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/config/lib >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS

	# demo.sh
	echo "if [ -e /customer/demo.sh ]; then" >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	echo "    /customer/demo.sh" >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	echo "fi;" >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS
	echo "telnetd &" >> $(OUTPUTDIR)/rootfs/etc/init.d/rcS

	# sshd
	if [[ "$(FLASH_TYPE)" = "spinand" ]] && [[ -d "$(OUTPUTDIR)/customer/ssh" ]]; then \
		if [[ $(TOOLCHAIN_VERSION) = "9.1.0" ]] || [[ $(TOOLCHAIN_VERSION) = "8.2.1" ]]; then \
			echo "sshd:x:74:74:Privilege-separated SSH:/var/empty/sshd:/sbin/nologin" >> $(OUTPUTDIR)/rootfs/etc/passwd; \
			echo "export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/customer/ssh/lib" >> ${OUTPUTDIR}/rootfs/etc/init.d/rcS; \
			echo "mkdir /var/empty" >> ${OUTPUTDIR}/rootfs/etc/init.d/rcS; \
			echo "/customer/ssh/sbin/sshd -f /customer/ssh/etc/sshd_config" >> ${OUTPUTDIR}/rootfs/etc/init.d/rcS; \
			echo "export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/customer/ssh/lib" >> ${OUTPUTDIR}/rootfs/etc/profile; \
		fi; \
	fi;

	# set default password NULL
	echo "root::0:0:Linux User,,,:/home/root:/bin/sh" >> $(OUTPUTDIR)/rootfs/etc/passwd;

	# create symbol link
	mkdir -p $(OUTPUTDIR)/rootfs/lib/modules/$(CUR_KERNEL_VERSION)
	ln -fs /config/modules/$(KERNEL_VERSION) $(OUTPUTDIR)/rootfs/lib/modules/$(CUR_KERNEL_VERSION)
