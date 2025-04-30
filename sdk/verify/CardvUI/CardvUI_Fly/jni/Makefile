#一般情况无需修改该文件

LOCAL_PATH=$(dir  $(abspath $(lastword $(MAKEFILE_LIST))))
ZKSW_PREBUILT_LIB_PATH := $(ZKSW_REAL_PREBUILT)/lib/
ZKSW_PREBUILT_INCLUDE_PATH := $(ZKSW_REAL_PREBUILT)/include

EASYUI_COMMON_INCLUDE := $(LOCAL_PATH)/include

ALKAID_PATH := ../../../../../
include $(ALKAID_PATH)/project/configs/current.configs

ifneq ($(OS), Windows_NT)
-include $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/toolchain.mk
CC = $(CXX)
else
# because CC is re-defined in current.configs, set windows IDE toolchain env here.
CC = arm-pc-linux-gnueabihf-g++
endif

ZKSW_REAL_PREBUILT := ../../app/minigui_dir
ZKSW_PREBUILT_LIB_PATH := $(ZKSW_REAL_PREBUILT)/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release
ZKSW_PREBUILT_INCLUDE_PATH := $(ZKSW_REAL_PREBUILT)/include

#配置源文件目录
PROJ_ROOT_PATH  :=./
#源文件后缀名
PROJ_SRC_SUFFIX := %.cpp %.c
PROJ_HEAD_SUFFIX := %.h
# 递归遍历目录下的所有的文件
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
# 获取相应的源文件
PROJ_ALL_FILES := $(foreach src_path,$(PROJ_ROOT_PATH), $(call rwildcard,$(src_path),*.*) )
PROJ_SRC_FILES := $(filter $(PROJ_SRC_SUFFIX),$(PROJ_ALL_FILES))
PROJ_HEAD_FILES := $(filter $(PROJ_HEAD_SUFFIX),$(PROJ_ALL_FILES))

# 获取相应的源文件
LOCAL_SRC_FILES  := $(PROJ_SRC_FILES)
LOCAL_HEAD_FILES := $(PROJ_HEAD_FILES)

#源文件目录结构
PROJ_DIR_TREE := $(dir $(foreach src_path, $(LOCAL_SRC_FILES), $(call rwildcard,$(src_path),*) ) )
PROJ_DIR_TREE := $(sort $(PROJ_DIR_TREE))

#wlan功能启用开关
#CONFIG_WLAN_SWITCH := "enable"
CONFIG_WLAN_SWITCH :=

#carlife功能启用开关
#CONFIG_CARLIFE_SWITCH := "enable"
#CONFIG_CARLIFE_SWITCH :=
ifeq ($(CARLIFE_ENABLE), yes)
CONFIG_CARLIFE_SWITCH := "enable"
else
CONFIG_CARLIFE_SWITCH :=
endif

#player功能启用开关
CONFIG_PLAYER_SWITCH := "enable"
#CONFIG_PLAYER_SWITCH :=

#同编同解功能启用开关
# CONFIG_PLAYING_AND_RECORDING := "enable"
CONFIG_PLAYING_AND_RECORDING :=

#cloudplay功能启用开关
#CONFIG_CLOUD_PLAY_SWITCH := "enable"
CONFIG_CLOUD_PLAY_SWITCH :=

# CONFIG_FB1_SUPPORT := "enable"
CONFIG_FB1_SUPPORT :=

OBJS_ROOT_DIR=../obj/
OBJS_1 = $(patsubst %.cpp,%.o,$(LOCAL_SRC_FILES))
OBJS_2 = $(patsubst %.c,%.o,$(OBJS_1))
OBJS   = $(addprefix $(OBJS_ROOT_DIR),$(OBJS_2))
OBJS_DIR_TREE=$(addprefix $(OBJS_ROOT_DIR),$(PROJ_DIR_TREE))

CFLAGS=-Os -pipe  -fno-caller-saves -Wno-unused-result -mfloat-abi=hard -mfpu=vfp -Wformat -Werror=format-security -fstack-protector -D_FORTIFY_SOURCE=1 -Wl,-z,now -Wl,-z,relro -Wl,--warn-common -Wl,--warn-once -Wl,-z,combreloc -Wl,-z,defs -Wall -Werror
CXXFLAGS=-Os -pipe  -fno-caller-saves -Wno-unused-result -mfloat-abi=hard -mfpu=vfp -Wformat -Werror=format-security -fstack-protector -D_FORTIFY_SOURCE=1 -Wl,-z,now -Wl,-z,relro -Wl,-z,defs
CXXFLAGS+= -fexceptions -fpermissive -Wall -Werror -O2

INCLUDE_DIR := -I$(EASYUI_COMMON_INCLUDE) \
				-I$(ZKSW_PREBUILT_INCLUDE_PATH) \
				-I. \
				-I$(ZKSW_PREBUILT_INCLUDE_PATH)/mgeff \
				-I ../jni/voicedetect \
				-I$(ALKAID_PATH)/project/release/include \
				-I$(ALKAID_PATH)/sdk/verify/Cardvimpl/cardv_IPC \
				-I$(ALKAID_PATH)/sdk/verify/Cardvimpl/DCF \
				-I$(ALKAID_PATH)/sdk/verify/Cardvimpl/exif/include \
				-I$(ALKAID_PATH)/sdk/verify/Cardvimpl/cardv/inc/adas \
				-I$(ALKAID_PATH)/sdk/verify/CardvUI/ffmpeg/ffmpeg-4.1.3/output/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/include \
				-I$(ALKAID_PATH)/sdk/verify/Cardvimpl/imo_dms/include/ \
				-I$(ALKAID_PATH)/sdk/verify/CardvUI/SDL/SDL2-2.0.16/include/SDL2 \
				-I$(ALKAID_PATH)/sdk/verify/CardvUI/SDL/SDL2_ttf-2.0.15/include/SDL2 \
				-I$(ALKAID_PATH)/sdk/verify/CardvUI/SDL/SDL2_image-2.0.5/include/SDL2 \
				-I$(ALKAID_PATH)/sdk/verify/CardvUI/SDL/CarUI/include \

INCLUDE_DIR += -I ../jni/player -I ../jni/player/mediastream -I ../jni/player/usbdetect
#LDFLAGS =  -leasyui -pthread -lmi_sys -lmi_disp -lmi_panel -lmi_ao -lmi_gfx -lmi_divp -lmi_vdec -lmi_common -lstdc++ -lmi_ai -ldl
LDFLAGS =  -leasyui -lexif -lIPC_msg -pthread -lDCF -lmi_sys -lmi_ao -lmi_scl -lmi_common -lstdc++ -ldl
LDFLAGS += -L$(ZKSW_PREBUILT_LIB_PATH)/ -L. -L ../jni/sdkdir/lib
LDFLAGS += -L$(ALKAID_PATH)/project/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/mi_libs/dynamic
LDFLAGS += -L$(ALKAID_PATH)/project/release/chip/$(CHIP)/sigma_common_libs/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/dynamic
LDFLAGS += -L$(ALKAID_PATH)/project/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/3rd_party_libs/dynamic
LDFLAGS += -L$(ALKAID_PATH)/sdk/verify/CardvUI/SDL/DirectFB-DIRECTFB_1_7_7/lib
LDFLAGS += -L$(ALKAID_PATH)/sdk/verify/CardvUI/SDL/CarUI/lib
LDFLAGS += -L$(ALKAID_PATH)/sdk/verify/Cardvimpl/exif/$(TOOLCHAIN)-lib

LDFLAGS += -rdynamic -Wl,--gc-sections
CFLAGS += -funwind-tables -ffunction-sections -fdata-sections -g
CXXFLAGS += -funwind-tables -ffunction-sections -fdata-sections -g

CFLAGS += -DCHIP_$(shell echo $(CHIP) | tr a-z A-Z)
CXXFLAGS += -DCHIP_$(shell echo $(CHIP) | tr a-z A-Z)

CFLAGS += -DUSE_KEY_BOARD
CXXFLAGS += -DUSE_KEY_BOARD

ifeq ($(DUAL_OS), on)
CFLAGS += -DDUAL_OS
LOCAL_LDFLAGS += -DDUAL_OS
endif

ifneq ($(CONFIG_WLAN_SWITCH),)
CFLAGS+=-DSUPPORT_WLAN_MODULE
CXXFLAGS+=-DSUPPORT_WLAN_MODULE
INCLUDE_DIR += -I ../jni/wifi
LDFLAGS += -lmi_wlan -lcjson
endif

ifneq ($(CONFIG_CARLIFE_SWITCH),)
LDFLAGS += -L ../../carlife/demo/carlife/lib -llylink
INCLUDE_DIR += -I ../jni/player/carlife
CFLAGS += -DSUPPORT_CARLIFE
CXXFLAGS += -DSUPPORT_CARLIFE
endif

ifneq ($(CONFIG_PLAYING_AND_RECORDING),)
CFLAGS += -DPLAYING_AND_RECORDING
CXXFLAGS += -DPLAYING_AND_RECORDING
endif

ifneq ($(CONFIG_PLAYER_SWITCH),)
CFLAGS+=-DSUPPORT_PLAYER_MODULE
CXXFLAGS+=-DSUPPORT_PLAYER_MODULE
LDFLAGS += -lavformat -lavcodec -lavutil -lswresample
endif

ifneq ($(CONFIG_CLOUD_PLAY_SWITCH),)
CFLAGS+=-DSUPPORT_CLOUD_PLAY_MODULE
CXXFLAGS+=-DSUPPORT_CLOUD_PLAY_MODULE
endif

ifneq ($(CONFIG_FB1_SUPPORT),)
CFLAGS += -DSUPPORT_FB1
CXXFLAGS += -DSUPPORT_FB1
LDFLAGS += -lCarUI -ldirectfb -lfusion -ldirect
endif

TARGET=../libs/armeabi/libzkgui.so

ifeq ($(TARGET), $(wildcard $(TARGET)))
	DELETE_TARGET=$(TARGET)
endif

# alias commads if compile on windows
RM =rm
MKDIR=mkdir
ECHO=echo
ifeq ($(OS),Windows_NT)
	RM=win32-rm
	MKDIR=win32-mkdir
	ECHO=win32-echo
else
	#keep default
endif
#alias end

all: prepare $(TARGET)
	@$(ECHO)
	@$(ECHO) "[armeabi] Install        : libzkgui.so => libs/armeabi/libzkgui.so"
$(OBJS_ROOT_DIR)%.o: %.cpp $(LOCAL_HEAD_FILES)
	@$(ECHO) "[armeabi] Compile++      : "$<
	$(CC) -fPIC -c $< -o $@ $(CXXFLAGS) $(INCLUDE_DIR)

$(OBJS_ROOT_DIR)%.o: %.c $(LOCAL_HEAD_FILES)
	@$(ECHO) "[armeabi] Compile++      : "$<
	$(CC) -fPIC -c $< -o $@ $(CFLAGS) $(INCLUDE_DIR)

$(TARGET):$(OBJS)
	@$(ECHO) "[armeabi] SharedLibrary  : "$@
	$(CC) -fPIC -shared $^ -o $@ $(LDFLAGS) $(CXXFLAGS)

prepare:
	@-$(MKDIR) $(OBJS_ROOT_DIR) -p
	@-$(MKDIR) $(OBJS_DIR_TREE) -p
	@$(ECHO) "[armeabi] ZK_SW_PREBUILT : "$(ZKSW_REAL_PREBUILT)
	@$(ECHO) "[armeabi] GCC            : "$(CC)
	-$(MKDIR) ../libs/armeabi -p
	-$(RM) $(TARGET) -rf
	$(ECHO) "todo OBJS...":$(OBJS)

clean:
	$(ECHO) "[armeabi] Clean          : "$(OBJS_ROOT_DIR)
	-$(RM)  $(OBJS_ROOT_DIR) -rf
	$(ECHO) "[armeabi] Clean          : "$(TARGET)
	-$(RM)  $(TARGET) -rf

ifneq ($(OS), Windows_NT)
install:
	$(ECHO) "[armeabi] Install        : release to alkaid"
	mkdir -p $(ALKAID_PATH)/project/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/bin/UI/lib 
	mkdir -p $(ALKAID_PATH)/project/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/bin/UI/res/ui/cardv 
	mkdir -p $(ALKAID_PATH)/project/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/bin/UI/res/ui 
	mkdir -p $(ALKAID_PATH)/project/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/bin/UI/bin 
	cp -P $(ZKSW_PREBUILT_LIB_PATH)/* $(ALKAID_PATH)/project/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/bin/UI/lib/
	cp ../libs/armeabi/libzkgui.so $(ALKAID_PATH)/project/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/bin/UI/lib/
	cp ../resources/cardv/*        $(ALKAID_PATH)/project/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/bin/UI/res/ui/cardv
	cp ../ui/*.ftu                 $(ALKAID_PATH)/project/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/bin/UI/res/ui
	cp ../../app/bin/zkgui         $(ALKAID_PATH)/project/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/bin/UI/bin
install_ffmpeg:
	cp -d ../../ffmpeg/ffmpeg-4.1.3/output/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/lib*.s* $(ALKAID_PATH)/project/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/3rd_party_libs/dynamic
	cp -d ../../ffmpeg/ffmpeg-4.1.3/output/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/*.a     $(ALKAID_PATH)/project/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/3rd_party_libs/static
endif

.PHONY:all clean prepare $(TARGET)



