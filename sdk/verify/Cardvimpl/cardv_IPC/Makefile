TOP_DIR  ?=$(shell pwd)
ALKAID_DIR ?=$(shell pwd)/../../../..
SDK_DIR  ?=$(ALKAID_DIR)/sdk
PROJECT_DIR  ?=$(ALKAID_DIR)/project
CARDVIMPL_DIR  ?=$(ALKAID_DIR)/sdk/verify/Cardvimpl

include $(PROJECT_DIR)/configs/current.configs
-include $(PROJECT_DIR)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/toolchain.mk

CFLAGS := -Wall -Werror -O3
CFLAGS += -DCHIP_$(shell echo $(CHIP) | tr a-z A-Z)

OBJS = IPC_msg.o IPC_cardvInfo.o
LIB = libIPC_msg.so
STATIC_LIB = libIPC_msg.a

INCLUDE_PATH = -I$(TOP_DIR) \
               -I$(PROJECT_DIR)/release/include \
               -I$(CARDVIMPL_DIR)/DCF \
               -I$(CARDVIMPL_DIR)/cardv/inc/adas \
               -I$(CARDVIMPL_DIR)/imo_dms/include/ \

all : $(LIB) $(STATIC_LIB) prepare

prepare :
	-$(CLANG_FORMAT) -i *.c *.h

%.o : %.c prepare
	@echo "compile $<"
	@$(CC) $(CFLAGS) -fpic -c $< -o $@ $(INCLUDE_PATH)

$(LIB) : $(OBJS)
	rm -f $@
	$(CC) -shared -o $@ $(OBJS)

$(STATIC_LIB) : $(OBJS)
	rm -f $@
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) $(LIB) $(STATIC_LIB)

install:
	cp $(LIB) $(PROJECT_DIR)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/3rd_party_libs/dynamic
	cp $(STATIC_LIB) $(PROJECT_DIR)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/3rd_party_libs/static
