
TARGETNAME  := lyLinkUI
TARGETEXE	:= $(TARGETNAME)
TARGETSO	:= lib$(TARGETNAME).so
TARGET		:= $(TARGETEXE)
# TARGETSTRIP	:= lib$(TARGETNAME)strip.so
TARGETSTRIP	:= $(TARGETNAME)strip

CARLIFE_PATH=pwd

OBJS := main.o

DEPS := $(OBJS:%.o=%.d)
CXX=arm-linux-gnueabihf-g++
#AR=arm-linux-gnueabihf-ar
#NM=arm-linux-gnueabihf-nm
STRIP=arm-linux-gnueabihf-strip
#ST=$STRIP
#LD=arm-linux-gnueabihf-ld
#READELF=arm-linux-gnueabihf-readelf
#OBJCOPY=arm-linux-gnueabihf-objcopy
#OBJDUMP=arm-linux-gnueabihf-objdump
#SIZE=arm-linux-gnueabihf-size
#CP=cp
#MKDIR=mkdir

CPPFLAGS := -MMD -MP
CPPFLAGS += -I./inc

CFLAGS   := -fPIC -g -Wall -I$(CARLIFE_PATH)/inc
CXXFLAGS := $(CFLAGS) -std=c++11
CFLAGS	 += -Os -pipe  -fno-caller-saves -Wno-unused-result -mfloat-abi=hard -mfpu=vfp -Wformat -Werror=format-security -fstack-protector -D_FORTIFY_SOURCE=1 -Wl,-z,now -Wl,-z,relro -Wl,--warn-common -Wl,--warn-once -Wl,-z,combreloc
CXXFLAGS += -Os -pipe  -fno-caller-saves -Wno-unused-result -mfloat-abi=hard -mfpu=vfp -Wformat -Werror=format-security -fstack-protector -D_FORTIFY_SOURCE=1 -Wl,-z,now -Wl,-z,relro -fexceptions -fpermissive


LDFLAGS  := -L./lib
LIBS     := -llylink

LDFLAGS  += -o

.PHONY: genstrip genso genexe clean

all: genexe

-include $(DEPS)

genstrip: $(TARGETSTRIP)

genso: $(TARGETSO)

genexe: $(TARGETEXE)

$(TARGETSTRIP): $(OBJS)
	$(CXX) $(LDFLAGS) $@ $^ $(LIBS)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $@ $^ $(LIBS)
	$(STRIP) $@
	-mv $(TARGET) ./demo/carlife/
	
clean:
	@rm -rf $(OBJS) $(DEPS) $(TARGET) $(TARGETSTRIP) $(TARGETEXE)

