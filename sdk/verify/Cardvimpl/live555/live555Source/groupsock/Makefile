INCLUDES = -Iinclude -I../UsageEnvironment/include

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

NAME = libgroupsock
LIB_FILE = $(NAME).$(LIB_SUFFIX)
ALL = $(LIB_FILE)
all:	$(ALL)

.$(C).$(OBJ):
	$(C_COMPILER) -c $(C_FLAGS) $<
.$(CPP).$(OBJ):
	$(CPLUSPLUS_COMPILER) -c $(CPLUSPLUS_FLAGS) $<

GROUPSOCK_LIB_OBJS = GroupsockHelper.$(OBJ) GroupEId.$(OBJ) inet.$(OBJ) Groupsock.$(OBJ) NetInterface.$(OBJ) NetAddress.$(OBJ) IOHandlers.$(OBJ)

GroupsockHelper.$(CPP):	include/GroupsockHelper.hh
include/GroupsockHelper.hh:	include/NetAddress.hh
include/NetAddress.hh:	include/NetCommon.h
GroupEId.$(CPP):	include/GroupEId.hh
include/GroupEId.hh:	include/NetAddress.hh
inet.$(C):		include/NetCommon.h
Groupsock.$(CPP):	include/Groupsock.hh include/GroupsockHelper.hh include/TunnelEncaps.hh
include/Groupsock.hh:	include/groupsock_version.hh include/NetInterface.hh include/GroupEId.hh
include/NetInterface.hh:	include/NetAddress.hh
include/TunnelEncaps.hh:	include/NetAddress.hh
NetInterface.$(CPP):	include/NetInterface.hh include/GroupsockHelper.hh
NetAddress.$(CPP):	include/NetAddress.hh include/GroupsockHelper.hh
IOHandlers.$(CPP):	include/IOHandlers.hh include/TunnelEncaps.hh

$(LIB_FILE): $(GROUPSOCK_LIB_OBJS) \
    $(PLATFORM_SPECIFIC_LIB_OBJS)
	$(LIBRARY_LINK) $@ $(LIBRARY_LINK_OPTS) \
		$(GROUPSOCK_LIB_OBJS)

clean:
	-rm -rf *.$(OBJ) $(ALL) core *.core *~ include/*~

install: install1 $(INSTALL2)
install1: $(LIB_FILE)
	install -d $(DESTDIR)$(INSTALL_INC_PATH)/groupsock $(DESTDIR)$(INSTALL_LIB_PATH)
	install -m 644 include/*.h $(DESTDIR)$(INSTALL_INC_PATH)/groupsock
	install -m 644 include/*.hh $(DESTDIR)$(INSTALL_INC_PATH)/groupsock
	install -m 644 $(LIB_FILE) $(DESTDIR)$(INSTALL_LIB_PATH)

##### Any additional, platform-specific rules come here: