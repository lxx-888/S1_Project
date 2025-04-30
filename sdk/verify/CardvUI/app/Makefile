TARGET = zkgui

ALKAID_PATH = ../../../../
PROJECT_PATH = $(ALKAID_PATH)/project

include $(PROJECT_PATH)/configs/current.configs

MINIGUI_PATH=./minigui_dir

CHIP_NAME = CHIP_$(shell echo $(CHIP) | tr a-z A-Z)

CFLAGS = -I$(MINIGUI_PATH)/include -I$(MINIGUI_PATH)/include/easyui -I$(PROJECT_PATH)/release/include -I$(MINIGUI_PATH)/include/easyui/common
LDFLAGS = -L$(MINIGUI_PATH)/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release -L$(PROJECT_PATH)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/mi_libs/dynamic -L$(PROJECT_PATH)/release/chip/$(CHIP)/sigma_common_libs/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/dynamic

CONFIG_PREFIX =.

LOCAL_CFLAGS =
ifeq ($(DUAL_OS), on)
LOCAL_LDFLAGS = -lstdc++ -leasyui -lnanovg -lpng12 -ljpeg -lpthread -lfreetype -ldl -lts -lz -lmi_common -lmi_sys -lmi_disp -lcam_os_wrapper -lcam_fs_wrapper -Wl,--hash-style=gnu
else
LOCAL_LDFLAGS = -lstdc++ -leasyui -lnanovg -lpng12 -ljpeg -lpthread -lfreetype -ldl -lts -lz -lmi_common -lmi_sys -lmi_disp -lmi_ao -lmi_ai -lcam_os_wrapper -lcam_fs_wrapper -Wl,--hash-style=gnu
endif
ifneq ($(CHIP_NAME), CHIP_IFORD)
LOCAL_LDFLAGS += -lmi_panel
endif
LOCAL_LDFLAGS += -D$(CHIP_NAME)

LDFLAGS += -rdynamic
CFLAGS += -funwind-tables -ffunction-sections -g

ifeq ($(DUAL_OS), on)
CFLAGS += -DDUAL_OS
LOCAL_LDFLAGS += -DDUAL_OS
endif

CPPSRCS += ${wildcard *.cpp}
CPPOBJS = $(patsubst %.cpp, %.cpp.o, $(CPPSRCS))

CSRCS += ${wildcard *.c}
COBJS = $(patsubst %.c,%.c.o,$(CSRCS))

all:$(TARGET) install

$(TARGET):$(CPPOBJS) $(COBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -o $@ $^

$(CPPOBJS):%.cpp.o:%.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -c $^ -o $@

$(COBJS):%.c.o:%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -c $^ -o $@

install:$(TARGET)
	-mkdir -p $(CONFIG_PREFIX)/bin
	-mv $(TARGET) $(CONFIG_PREFIX)/bin/

clean:
	-rm -rf $(CPPOBJS) $(CONFIG_PREFIX)/bin/
	-rm -rf *.o
.PHONY:all clean $(TARGET) install
