TOP_DIR  ?=$(shell pwd)
ALKAID_DIR ?=$(shell pwd)/../../../..
SDK_DIR  ?=$(ALKAID_DIR)/sdk
PROJECT_DIR  ?=$(ALKAID_DIR)/project
CARDVIMPL_DIR  ?=$(ALKAID_DIR)/sdk/verify/Cardvimpl

include $(PROJECT_DIR)/configs/current.configs
-include $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/toolchain.mk

CFLAGS := -Wall -Werror -O3 -std=c99
CFLAGS += -DNO_SSL
CFLAGS += -DNO_CGI
CFLAGS += -DNO_CACHING
CFLAGS += -DNO_FILES
CFLAGS += -DNO_FILESYSTEMS
CFLAGS += -DMG_EXTERNAL_FUNCTION_mg_cry_internal_impl
CFLAGS += -DMG_EXTERNAL_FUNCTION_log_access
CFLAGS += -DUSE_WEBSOCKET
# CFLAGS += -DDEBUG

OBJS = civetweb.o
LIB = libcivetweb.so
STATIC_LIB = libcivetweb.a

INCLUDE_PATH = -I$(TOP_DIR)

all : $(STATIC_LIB)

%.o : %.c
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