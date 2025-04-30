TOP_DIR  ?=$(shell pwd)
ALKAID_DIR ?=$(shell pwd)/../../../..
SDK_DIR  ?=$(ALKAID_DIR)/sdk
PROJECT_DIR  ?=$(ALKAID_DIR)/project
CARDVIMPL_DIR  ?=$(ALKAID_DIR)/sdk/verify/Cardvimpl

include $(PROJECT_DIR)/configs/current.configs
-include $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/toolchain.mk

CFLAGS  := -Wall -Werror -O3 -std=c99

OBJS = DCF.o
LIB = libDCF.so
STATIC_LIB = libDCF.a

INCLUDE_PATH = -I$(TOP_DIR)

all : $(LIB) $(STATIC_LIB) prepare

prepare :
	-$(CLANG_FORMAT) -i *.c

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
