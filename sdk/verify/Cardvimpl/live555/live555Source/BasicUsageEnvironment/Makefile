INCLUDES = -Iinclude -I../UsageEnvironment/include -I../groupsock/include

##### Change the following for your environment:
ALKAID_DIR ?=$(shell pwd)/../../../../../..
CARDVIMPL_DIR  ?=$(ALKAID_DIR)/sdk/verify/Cardvimpl
PROJECT_DIR  ?=$(ALKAID_DIR)/project
include $(PROJECT_DIR)/configs/current.configs

INSTALL_INC_PATH = $(CARDVIMPL_DIR)/live555/include
INSTALL_LIB_PATH = $(CARDVIMPL_DIR)/live555/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)

C_COMPILER = $(CROSS_COMPILE)gcc
COMPILE_OPTS =$(INCLUDES) -I. -O2 -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64
C =	c
C_COMPILER = $(CROSS_COMPILE)gcc
C_FLAGS = $(COMPILE_OPTS)
CPP = cpp
CPLUSPLUS_COMPILER = $(CROSS_COMPILE)g++
CPLUSPLUS_FLAGS = $(COMPILE_OPTS) -Wall -DBSD=1 -DALLOW_RTSP_SERVER_PORT_REUSE
OBJ = o
LINK = $(CROSS_COMPILE)g++ -o
LINK_OPTS =
CONSOLE_LINK_OPTS =	$(LINK_OPTS)
LIBRARY_LINK = $(CROSS_COMPILE)ar cr
LIBRARY_LINK_OPTS =	$(LINK_OPTS)
LIB_SUFFIX = a
LIBS_FOR_CONSOLE_APPLICATION =
LIBS_FOR_GUI_APPLICATION =
EXE =
##### End of variables to change

NAME = libBasicUsageEnvironment
LIB_FILE = $(NAME).$(LIB_SUFFIX)
ALL = $(LIB_FILE)
all:	$(ALL)

OBJS = BasicUsageEnvironment0.$(OBJ) BasicUsageEnvironment.$(OBJ) \
	BasicTaskScheduler0.$(OBJ) BasicTaskScheduler.$(OBJ) \
	DelayQueue.$(OBJ) BasicHashTable.$(OBJ)

$(LIB_FILE): $(OBJS)
	$(LIBRARY_LINK) $@ $(LIBRARY_LINK_OPTS) \
		$(OBJS)

.$(C).$(OBJ):
	$(C_COMPILER) -c $(C_FLAGS) $<

.$(CPP).$(OBJ):
	$(CPLUSPLUS_COMPILER) -c $(CPLUSPLUS_FLAGS) $<

BasicUsageEnvironment0.$(CPP):	include/BasicUsageEnvironment0.hh
include/BasicUsageEnvironment0.hh:	include/BasicUsageEnvironment_version.hh include/DelayQueue.hh
BasicUsageEnvironment.$(CPP):	include/BasicUsageEnvironment.hh
include/BasicUsageEnvironment.hh:	include/BasicUsageEnvironment0.hh
BasicTaskScheduler0.$(CPP):	include/BasicUsageEnvironment0.hh include/HandlerSet.hh
BasicTaskScheduler.$(CPP):	include/BasicUsageEnvironment.hh include/HandlerSet.hh
DelayQueue.$(CPP):		include/DelayQueue.hh
BasicHashTable.$(CPP):		include/BasicHashTable.hh

clean:
	-rm -rf *.$(OBJ) $(ALL) core *.core *~ include/*~

install: install1 $(INSTALL2)
install1: $(LIB_FILE)
	install -d $(DESTDIR)$(INSTALL_INC_PATH)/BasicUsageEnvironment $(DESTDIR)$(INSTALL_LIB_PATH)
	install -m 644 include/*.hh $(DESTDIR)$(INSTALL_INC_PATH)/BasicUsageEnvironment
	install -m 644 $(LIB_FILE) $(DESTDIR)$(INSTALL_LIB_PATH)

##### Any additional, platform-specific rules come here: