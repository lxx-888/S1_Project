DMS ?= 0
ADAS ?= 0
ALINK ?= 0
LIGHTDETECT ?= 0

TOP_DIR  ?=$(shell pwd)
ALKAID_DIR ?=$(shell pwd)/../../../..
SDK_DIR  ?=$(ALKAID_DIR)/sdk
PROJECT_DIR  ?=$(ALKAID_DIR)/project
CARDVIMPL_DIR  ?=$(ALKAID_DIR)/sdk/verify/Cardvimpl

include $(PROJECT_DIR)/configs/current.configs

OBJDIR ?= $(shell pwd)/obj
BINDIR ?= $(shell pwd)/bin
LIBMIDDIR ?= $(shell pwd)/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/mid

# DEBUG config is shared with different toolchain
DEBUG_ASAN:=$(shell if [[ "$(DEBUG)X" != "X"  ]] && ((( $(DEBUG) & 256 )) || (( $(DEBUG) & 1 ))); then echo "1"; fi)
DEBUG_GCOV:=$(shell if [[ "$(DEBUG)X" != "X"  ]] && ((( $(DEBUG) & 512 ))); then echo "1"; fi)
ifeq ($(DEBUG_ASAN), 1)
TARGET_REL_FOLDER := debug
else
TARGET_REL_FOLDER := release
endif
# End of DEBUG config

INSTALLDIR ?= $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(TARGET_REL_FOLDER)/bin/cardv

SDK_INC ?=$(PROJECT_DIR)/release/include
ifeq (static, $(BUILD_MODE))
SIGMA_MI_LIBS ?=$(PROJECT_DIR)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(TARGET_REL_FOLDER)/mi_libs/static
SIGMA_COMLIBS ?=$(PROJECT_DIR)/release/chip/$(CHIP)/sigma_common_libs/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(TARGET_REL_FOLDER)/static
CARDV_LIBS ?=$(PROJECT_DIR)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(TARGET_REL_FOLDER)/3rd_party_libs/static
BUILD_MESSAGE="===== make $(CHIP) cardv bin file with STATIC library ====="
else
SIGMA_MI_LIBS ?=$(PROJECT_DIR)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(TARGET_REL_FOLDER)/mi_libs/dynamic
SIGMA_COMLIBS ?=$(PROJECT_DIR)/release/chip/$(CHIP)/sigma_common_libs/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(TARGET_REL_FOLDER)/dynamic
CARDV_LIBS ?=$(PROJECT_DIR)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(TARGET_REL_FOLDER)/3rd_party_libs/dynamic
BUILD_MESSAGE="===== make $(CHIP) cardv bin file with DYNAMIC library ====="
endif

# link ffmpeg from cardv UI
FFMPEG_INC ?=$(CARDVIMPL_DIR)/../CardvUI/ffmpeg/ffmpeg-4.1.3/output/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/include

NMEA_INC ?=$(CARDVIMPL_DIR)/nmealib/include
NMEA_LIBS ?=$(CARDVIMPL_DIR)/nmealib/$(TOOLCHAIN)-lib

EDOG_INC ?=$(CARDVIMPL_DIR)/edog/
EDOG_LIBS ?=$(CARDVIMPL_DIR)/edog/$(TOOLCHAIN)-lib

EXIF_INC ?=$(CARDVIMPL_DIR)/exif/include
EXIF_LIBS ?=$(CARDVIMPL_DIR)/exif/$(TOOLCHAIN)-lib

LIVE_INC_PATH := $(CARDVIMPL_DIR)/live555/include
LIVE_LIB_PATH := $(CARDVIMPL_DIR)/live555/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)

NV_INC ?=$(CARDVIMPL_DIR)/net/cgi_1.0.0
NV_LIBS ?=$(CARDVIMPL_DIR)/net/cgi_1.0.0/lib

DMS_INC ?=$(CARDVIMPL_DIR)/imo_dms/include
DMS_LIBS ?=$(CARDVIMPL_DIR)/imo_dms/$(TOOLCHAIN)-lib

LD_INC ?=$(CARDVIMPL_DIR)/LD/include
LD_LIBS ?=$(CARDVIMPL_DIR)/LD/lib

CIVETWEB_INC ?=$(CARDVIMPL_DIR)/civetweb
CIVETWEB_LIBS ?=$(CARDVIMPL_DIR)/civetweb

C_FLAGS += -Wall -Werror -g
C_FLAGS += -fPIC -DPIC
C_FLAGS += -D__STDC_CONSTANT_MACROS
C_FLAGS += -O3 -funwind-tables
C_FLAGS += -ffunction-sections -fdata-sections
C_FLAGS += -MMD
C_FLAGS += -DCHIP_$(shell echo $(CHIP) | tr a-z A-Z)

ifeq ($(interface_vdf), enable)
C_FLAGS += -DINTERFACE_VDF=1
endif
ifeq ($(interface_ldc), enable)
C_FLAGS += -DINTERFACE_LDC=1
endif
ifeq ($(interface_vdec), enable)
C_FLAGS += -DINTERFACE_VDEC=1
endif
ifeq ($(interface_jpd), enable)
C_FLAGS += -DINTERFACE_JPD=1
endif
ifeq ($(interface_vdisp), enable)
C_FLAGS += -DINTERFACE_VDISP=1
endif
ifeq ($(interface_panel), enable)
C_FLAGS += -DINTERFACE_PANEL=1
endif
ifeq ($(interface_disp), enable)
C_FLAGS += -DINTERFACE_DISP=1
endif
ifeq ($(interface_hdmi), enable)
C_FLAGS += -DINTERFACE_HDMI=1
endif
ifeq ($(interface_mipitx), enable)
C_FLAGS += -DINTERFACE_MIPITX=1
endif
ifeq ($(ADAS), 1)
C_FLAGS += -DADAS=1
endif
ifeq ($(LIGHTDETECT), 1)
C_FLAGS += -DLIGHTDETECT=1
endif
ifeq ($(ALINK), 1)
C_FLAGS += -DALINK=1
endif
ifeq ($(DMS), 1)
C_FLAGS += -DDMS=1
endif
# libCSpotter & libuvdz
ifeq ($(SPEECH_VENDOR), libuvdz)
C_FLAGS += -DSPEECH_VENDOR_TONGXINGZHE=1
endif

ifeq ($(DUAL_OS), on)
C_FLAGS += -DDUAL_OS=1
endif

LDFLAGS := -ldl -lpthread
LDFLAGS += -Wl,--gc-sections,--print-gc-sections
LDFLAGS += -lgcc_s -rdynamic

LDFLAGS += -L$(SIGMA_MI_LIBS)
LDFLAGS += -L$(SIGMA_COMLIBS)
LDFLAGS += -L$(CARDV_LIBS)
LDFLAGS += -L$(NMEA_LIBS)
LDFLAGS += -L$(EDOG_LIBS)
LDFLAGS += -L$(EXIF_LIBS)
LDFLAGS += -L$(DMS_LIBS)
LDFLAGS += -L$(LD_LIBS)
LDFLAGS += -L$(CIVETWEB_LIBS)
LDFLAGS += -L$(NV_LIBS)
LDFLAGS += -L$(LIVE_LIB_PATH)
LDFLAGS += -L$(LIBMIDDIR)
LDFLAGS += -L$(CARDVIMPL_DIR)/opencv/static_lib_$(TOOLCHAIN_VERSION)
LDFLAGS += -L$(CARDVIMPL_DIR)/opencv/static_lib_$(TOOLCHAIN_VERSION)/opencv4/3rdparty
LDFLAGS += -L$(CARDVIMPL_DIR)/exfat/lib/glibc

LDFLAGS += -lmid_common
LDFLAGS += -lmid_rtsp
LDFLAGS += -lmid

LDFLAGS += -lmi_common
LDFLAGS += -lmi_sensor
LDFLAGS += -lmi_vif
LDFLAGS += -lmi_scl
LDFLAGS += -lmi_venc
LDFLAGS += -lmi_ai
LDFLAGS += -lmi_ao
LDFLAGS += -lmi_rgn
LDFLAGS += -lmi_sys
LDFLAGS += -lmi_isp
ifeq ($(interface_ive), enable)
LDFLAGS += -lmi_ive
endif
LDFLAGS += -lmi_iqserver
LDFLAGS += -lcus3a
LDFLAGS += -lispalgo
ifeq ($(ADAS), 1)
LDFLAGS += -lADAS_LINUX
LDFLAGS += -lBCam_ADAS_LINUX
LDFLAGS += -lMVE_LINUX_userspace
endif
ifeq ($(interface_vdf), enable)
LDFLAGS += -lmi_vdf
LDFLAGS += -lMD_LINUX
LDFLAGS += -lmi_shadow
endif
LDFLAGS += -lfbc_decode
ifeq ($(interface_disp), enable)
LDFLAGS += -lmi_disp
endif
ifeq ($(interface_vdisp), enable)
LDFLAGS += -lmi_vdisp
endif
ifeq ($(interface_panel), enable)
LDFLAGS += -lmi_panel
endif
ifeq ($(interface_hdmi), enable)
LDFLAGS += -lmi_hdmi
endif
ifeq ($(interface_mipitx), enable)
LDFLAGS += -lmi_mipitx
endif
ifeq ($(interface_vdec), enable)
LDFLAGS += -lmi_vdec
endif
ifeq ($(interface_jpd), enable)
LDFLAGS += -lmi_jpd
endif
ifeq ($(interface_ldc), enable)
LDFLAGS += -lmi_ldc
endif
ifeq ($(interface_ipu), enable)
LDFLAGS += -lmi_ipu
endif
LDFLAGS += -lcam_os_wrapper
LDFLAGS += -lcam_fs_wrapper
LDFLAGS += -lcivetweb

# ffmpeg
LDFLAGS += -lswresample
LDFLAGS += -lavformat
LDFLAGS += -lavcodec
LDFLAGS += -lavutil
LDFLAGS += -lfdk-aac

# GPS / EDOG / EXIF
LDFLAGS += -lnmea
LDFLAGS += -ledog
LDFLAGS += -lexif

# Live555
LDFLAGS += -lliveMedia
LDFLAGS += -lgroupsock
LDFLAGS += -lBasicUsageEnvironment
LDFLAGS += -lUsageEnvironment

LDFLAGS += -lDCF
LDFLAGS += -lIPC_msg
LDFLAGS += -lg711

LDFLAGS += -lmkexfatfs
LDFLAGS += -lexfat

# For menu setting
LDFLAGS += -lnv

ifeq ($(ALINK), 1)
LDFLAGS += -lalinksvr
LDFLAGS += -lcrypto
LDFLAGS += -ldns_sd
LDFLAGS += -lprotobuf-lite
LDFLAGS += -lssl
endif

ifeq ($(DMS), 1)
LDFLAGS += -limo_dms_static
LDFLAGS += -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_calib3d -lopencv_core
LDFLAGS += -littnotify -ltegra_hal -lmi_common -lmi_ipu
LDFLAGS += -lgomp
endif

ifeq ($(LIGHTDETECT), 1)
# LDFLAGS += -lLightDetect # CPU version
LDFLAGS += -lLdSdk # IPU version
LDFLAGS += -lmi_ipu
endif

C_INCLUDES := \
	-I./inc \
	-I./inc/mid \
	-I./inc/rtsp \
	-I./inc/speech/include \
	-I./inc/adas \
	-I$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/drivers/sstar/include \
	-I./inc/alink \
	-I$(SDK_INC) \
	-I$(FFMPEG_INC) \
	-I$(NMEA_INC) \
	-I$(EDOG_INC) \
	-I$(EXIF_INC) \
	-I$(DMS_INC) \
	-I$(LD_INC) \
	-I$(CIVETWEB_INC) \
	-I$(NV_INC) \
	-I../cardv_IPC \
	-I../DCF \
	-I../exfat/include \
	-I$(LIVE_INC_PATH)/UsageEnvironment \
	-I$(LIVE_INC_PATH)/groupsock \
	-I$(LIVE_INC_PATH)/liveMedia  \
	-I$(LIVE_INC_PATH)/BasicUsageEnvironment \
	-I$(SDK_DIR)/misc/fbdev/fbdev_video \
	-I$(CARDVIMPL_DIR)/opencv/include/opencv4 \

PROGS = cardv
LIB_MID_COMMON = libmid_common.a
LIB_MID_RTSP = libmid_rtsp.a
LIB_MID = libmid.a

CARDV_MODULE_CPP_SRCS = $(wildcard ./src/*.cpp)
CARDV_MODULE_CPP_SRCS_NO_DIR=$(notdir $(CARDV_MODULE_CPP_SRCS))
CPP_OBJECTS += $(patsubst %.cpp, %.o, $(CARDV_MODULE_CPP_SRCS_NO_DIR))

CARDV_MODULE_C_SRCS += $(wildcard ./src/*.c)
CARDV_MODULE_C_SRCS_NO_DIR=$(notdir $(CARDV_MODULE_C_SRCS))
CPP_OBJECTS+=$(patsubst %.c, %.o, $(CARDV_MODULE_C_SRCS_NO_DIR))

OBJPROG = $(addprefix $(OBJDIR)/, $(PROGS))
OBJLIB_MID_COMMON = $(addprefix $(OBJDIR)/, $(LIB_MID_COMMON))
OBJLIB_MID_RTSP = $(addprefix $(OBJDIR)/, $(LIB_MID_RTSP))
OBJLIB_MID = $(addprefix $(OBJDIR)/, $(LIB_MID))

.PHONY: clean prepare $(OBJLIB_MID_COMMON) $(OBJLIB_MID_RTSP) $(OBJLIB_MID) $(OBJPROG) help


all: prepare $(OBJPROG)
	@echo $(BUILD_MESSAGE)

prepare:
	@echo '  CC' $(CC)
	@echo '  TOP_DIR' $(TOP_DIR)
	@echo '  CHIP' $(CHIP)

help:
	$(warning "make command build cardv bin file with DYNAMIC library")
	$(warning "make BUILD_MODE=static command build cardv bin file with STATIC library")

clean:
	@rm -Rf $(OBJDIR)
	@rm -Rf $(BINDIR)

install:
	@mkdir -p ${INSTALLDIR}
	@$(STRIP) --strip-unneeded ${BINDIR}/cardv
	rm -rf ${INSTALLDIR}/cardv
	cp -vf ${BINDIR}/cardv ${INSTALLDIR}
	cp -vf ini/default.ini ${INSTALLDIR}

$(OBJPROG):	 $(addprefix $(OBJDIR)/, $(CPP_OBJECTS))
	@mkdir -p $(OMX_TOP)/bin
	@echo "  BIN   $@"
	@$(CXX) -o $@  $(addprefix $(OBJDIR)/, $(CPP_OBJECTS)) $(LDFLAGS)
	@echo ""
	@rm -rf ${BINDIR}
	@mkdir ${BINDIR}
	@cp -f ${OBJDIR}/$(PROGS) ${BINDIR}

DFILES := $(foreach dir,$(OBJDIR),$(wildcard $(dir)/*.d))
sinclude $(DFILES)
$(OBJDIR)/%.o : src/%.c prepare
	@mkdir -p obj
	@echo "  CC    $<"
	@$(CC) $(C_FLAGS) $(C_INCLUDES) -c $< -o $@

DFILES := $(foreach dir,$(OBJDIR),$(wildcard $(dir)/*.d))
sinclude $(DFILES)
$(OBJDIR)/%.o : src/%.cpp prepare
	@mkdir -p obj
	@echo "  CC    $<"
	@$(CXX) $(C_FLAGS) $(C_INCLUDES) -c $< -o $@ -std=c++11
