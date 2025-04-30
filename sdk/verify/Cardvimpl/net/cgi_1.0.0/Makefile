ALKAID_DIR ?=$(shell pwd)/../../../../..
PROJECT_DIR  ?=$(ALKAID_DIR)/project

include $(PROJECT_DIR)/configs/current.configs
-include $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/toolchain.mk

PTHREAD_LIBS    = -lpthread
LIB     = libnv.a
INC     = -I. -I../../DCF
CFLAGS  = -fPIC -Wall -Werror -Wextra $(INC) $(PTHREAD_LIBS)
LFLAGS  = -static -Llib -lnv
OBJS    = nv.o
LIBSRC  = $(OBJS:.o=.c)

all: Config.cgi nvconf prepare

prepare:
	-$(CLANG_FORMAT) -i *.h *.c

$(LIB): crc.o nv.o
	$(Q)$(AR) rcu $(LIB) nv.o crc.o
	$(Q)mkdir -p lib;
	cp -a $@ lib;
	rm -f $@

crc.o: prepare
	$(CC) -Wall -Werror -Wextra -g -c -o crc.o crc.c

nv.o: prepare
	$(Q)$(CC) -g -c $(LIBSRC) $(CFLAGS) -o $@

Config.cgi: $(LIB)
	$(Q)$(CC) -o $@ $(CFLAGS) config.c  easy_cgi.c cgi_util.c directory.c -Llib -lnv
	$(Q)mkdir -p bin;
	cp -a $@ bin;
	cp -a cgi_config.bin bin;
	cp -a net_config.bin bin;

nvconf: $(LIB)
	$(Q)$(CC) -o $@ $(CFLAGS) nvconf.c -Llib -lnv
	$(Q)mkdir -p bin;
	cp -a $@ bin;

clean:
	$(Q)rm -f *.o $(OBJS) $(TARGET)
	$(Q)rm -rf lib
	$(Q)rm -rf bin
	$(Q)rm -f Config.cgi
	$(Q)rm -f nvconf
	$(Q)rm -f *~
	$(Q)rm -f ./inc/*~

install:
	cp bin/nvconf $(PROJECT_DIR)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/wifi/
	-@mkdir -p $(PROJECT_DIR)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/wifi/webserver/www/
	cp bin/Config.cgi $(PROJECT_DIR)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/wifi/webserver/www/cgi-bin
	cp bin/cgi_config.bin $(PROJECT_DIR)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/wifi/
	cp bin/net_config.bin $(PROJECT_DIR)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/wifi/

.PHONY: Config.cgi nvconf prepare
