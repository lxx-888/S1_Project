CROSS_COMPILE ?=arm-linux-gnueabihf-
CC  = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
AR  = $(CROSS_COMPILE)ar

COM_FLAGS = -Wall -O2 -fPIC -mcpu=cortex-a9 -mfpu=neon-fp16 -mfloat-abi=hard -mthumb-interwork -marm
C_FLAGS	   = $(COM_FLAGS) -std=c11
CPP_FLAGS  = $(COM_FLAGS) -std=c++11

# user path
FFMPEG_PATH = ../ffmpeg-4.1.3/output/glibc/9.1.0/
ALKAID_PATH = ../../../../../

INCLUDES  += -I. -I$(ALKAID_PATH)/project/release/include -I$(FFMPEG_PATH)/include

TARGET_NAME  = SsPlayer

CPP_SRCS  =

C_SRCS    =  $(wildcard *.c */*.c)

CPP_OBJS = $(patsubst %.cpp, %.cpp.o, $(CPP_SRCS))
C_OBJS = $(patsubst %.c, %.c.o, $(C_SRCS))

LIB_PATH = -L.
LIB_PATH += -L$(ALKAID_PATH)/project/release/chip/m6/cardv/common/glibc/9.1.0/mi_libs/dynamic
LIB_PATH += -L$(ALKAID_PATH)/project/release/chip/m6/sigma_common_libs/glibc/9.1.0/dynamic
LIB_PATH += -L$(ALKAID_PATH)/project/release/chip/m6/cardv/common/glibc/9.1.0/3rd_party_libs/dynamic
LIB_NAME += -lavformat -lavcodec -lavutil -lswresample -lfdk-aac -pthread -lm -lmi_vdec -lmi_jpd -lmi_sys -lmi_divp -lmi_disp -lmi_ao -lmi_panel -ldl -lmi_common -lcam_os_wrapper -lcam_fs_wrapper

.PHONY: all prepare clean

all: prepare $(TARGET_NAME) finish

prepare:
	@echo
	@echo ">>>>========================================================"
	@echo "TARGET_NAME = $(TARGET_NAME)"
	@echo


clean:
	@rm -Rf $(CPP_OBJS)
	@rm -f $(C_OBJS)
	@rm -Rf $(TARGET_NAME)

finish:
	@echo "make done"
	@echo "<<<<========================================================"
	@echo

$(TARGET_NAME): $(CPP_OBJS) $(CPP_SRCS) $(C_OBJS) $(C_SRCS)
	@echo "generate $@"
	@$(CC) -o $@ $(C_OBJS) $(CPP_OBJS) $(LIB_PATH) $(LIB_NAME) -lm -lpthread

%.c.o : %.c
	@echo "compile $@"
	@$(CC) $(C_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

%.cpp.o : %.cpp
	@echo "compile $@"
	@$(CPP) $(CPP_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@
