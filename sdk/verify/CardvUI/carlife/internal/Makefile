PWD=$(shell pwd)


TARGETNAME		:= AVInternal

ifeq ($(TARGETTYPE),)
TARGETTYPE      := lib
endif

ifeq ($(TARGETTYPE), exe)
TARGET			:= $(TARGETNAME)
TARGETSTRIP		:= $(TARGETNAME)strip
LINKTYPE		:= -o
else ifeq ($(TARGETTYPE), lib)
TARGET			:= lib$(TARGETNAME).so
TARGETSTRIP		:= lib$(TARGETNAME)strip.so
LINKTYPE		:= -shared -o
endif

TARGET  		:= lib$(TARGETNAME).so
TARGETSTRIP		:= lib$(TARGETNAME)strip.so
ALKAID_PATH := ../../../../../
include $(ALKAID_PATH)/project/configs/current.configs

OBJS := \
	src/H264Player.o \
	src/PcmPlayer.o \
	src/PcmRecord.o \
	src/iniparser.o \
	src/dictionary.o \
	src/strlib.o \
	
DEPS := $(OBJS:%.o=%.d)
CXX=arm-linux-gnueabihf-g++
STRIP=arm-linux-gnueabihf-strip


CPPFLAGS := -MMD -MP
#CPPFLAGS += -I./inc
CPPFLAGS += -I$(ALKAID_PATH)project/release/include
CPPFLAGS += -Iinc/utils
	
CFLAGS   := -fPIC -g -Wall -Os -pipe  -fno-caller-saves -Wno-unused-result -mfloat-abi=hard -mfpu=vfp -Wformat -Werror=format-security -fstack-protector -D_FORTIFY_SOURCE=1 -Wl,-z,now -Wl,-z,relro 
CXXFLAGS := $(CFLAGS) -std=c++11
CFLAGS	 += -Wl,--warn-common -Wl,--warn-once -Wl,-z,combreloc
CXXFLAGS += -fexceptions -fpermissive -Wall


LDFLAGS := -Wl,-z,defs
LDFLAGS += -L$(ALKAID_PATH)project/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/mi_libs/dynamic
LDFLAGS += -L$(ALKAID_PATH)project/release/chip/$(CHIP)/sigma_common_libs/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/dynamic
LIBS	:= -lmi_common -lmi_disp -lmi_panel -lmi_sys -lmi_vdec -lmi_ao -lmi_ai -lcam_os_wrapper
LIBS    += -lpthread -ldl


.PHONY: install clean genstrip

all: $(TARGET)

-include $(DEPS)

genstrip: $(TARGETSTRIP)

$(TARGETSTRIP): $(OBJS)
	$(CXX) $(LINKTYPE) $@ $^ $(LDFLAGS) $(LIBS)

$(TARGET): $(OBJS)
	$(CXX) $(LINKTYPE) $@ $^ $(LDFLAGS) $(LIBS)
	$(STRIP) $@

install: all
	@cp src/H264Player.h ../inc/internal/
	@cp src/PcmPlayer.h ../inc/internal/
	@cp src/PcmRecord.h ../inc/internal/
	@mkdir -p ../lib
	@cp $(TARGET) ../lib/
	@cp -ar ../lib ../demo/carlife
	@cp -ar ../demo/carlife $(ALKAID_PATH)project/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/
	
clean:
	@rm -rf $(OBJS) $(DEPS) $(TARGET) $(TARGETSTRIP)

