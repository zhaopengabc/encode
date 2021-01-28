TARGET=sigmastar_encode
CC = arm-linux-gnueabihf-9.1.0-gcc
CXX = arm-linux-gnueabihf-9.1.0-g++
SSC339G_PATH=./SSC339G


##### add SSC339G include ######
INC_PATH+=-I$(SSC339G_PATH)/common
INC_PATH+=-I$(SSC339G_PATH)/src
INC_PATH+=-I$(SSC339G_PATH)/include
INC_PATH+=-I$(SSC339G_PATH)/vif
INC_PATH+=-I$(SSC339G_PATH)/vpe
INC_PATH+=-I$(SSC339G_PATH)/venc


CFLAGS= -Wall -g ${INC_PATH} -lpthread -ldl 
CXXFLAGS= -std=gnu++11 ${CFLAGS}


LDLIBS += -L${SSC339G_PATH}/mi_libs/static
LDLIBS += -L${SSC339G_PATH}/ex_libs/static

LDLIBS += -lmi_sensor -lmi_vif -lmi_vpe -lmi_venc -lmi_common -lmi_sys -lmi_isp -lmi_iqserver -lmi_mipitx -lsigma3a -lmi_panel -lmi_rgn  \
-lcus3a -lispalgo -lmi_ai -lmi_ao -lmi_disp -lmi_divp -lmi_gyro -lmi_ipu -lmi_ldc -lmi_shadow -lmi_vdf -lmi_vdisp -lcam_os_wrapper


###### add SCC339G libs #######
# LDLIBS += ${SSC339G_PATH}/mi_libs/dynamic


# CXX_SRC = main.cpp \
# 	  player.cpp \
# 	  CXStreamer/element.cpp \
# 	  CXStreamer/factory.cpp \
# 	  CXStreamer/common/factorycommon.cpp \
# 	  CXStreamer/common/websocketserver.cpp \
# 	  CXStreamer/common/httpserver.cpp \
# 	  CXStreamer/common/websocketserverinf.cpp \
# 	  CXStreamer/common/websocketserverimplcfwk.cpp \
# 	  CXStreamer/common/livertspserver.cpp \
# 	  CXStreamer/common/FramedPipeSource.cpp \
# 	  CXStreamer/common/RTSPH264PassiveServer.cpp \
# 	  CXStreamer/common/RTSPServer.cpp \
# 	  CXStreamer/common/RTSPOnDemandServer.cpp \
# 	  CXStreamer/common/VideoPipeServerMediaSubsession.cpp \
# 	  CXStreamer/sigmastar/SSC339G/hielement.cpp \
# 	  CXStreamer/sigmastar/SSC339G/factory/SSC339GFactory.cpp \
# 	  CXStreamer/sigmastar/SSC339G/vif/vif_element.cpp \
# 	  CXStreamer/sigmastar/SSC339G/vpe/vpe_element.cpp \
#   	  CXStreamer/sigmastar/SSC339G/venc/venc_element.cpp

C_SRC = ./src/test.c \
	./src/nova_encode.c \
	./src/nova_pushStream.c \
	$(SSC339G_PATH)/vif/st_vif.c \
  	$(SSC339G_PATH)/vpe/st_vpe.c \
	$(SSC339G_PATH)/venc/st_venc.c \
  	$(SSC339G_PATH)/common/st_common.c 

CXXOBJS := $(CXX_SRC:.cpp=.ox)
COBJS := $(C_SRC:.c=.o)
# %.ox:%.cpp
# 	$(CXX) $< $(CXXFLAGS)  -c -o $@
%.o:%.c
	$(CC) $< -std=gnu99 $(CFLAGS) -c -o $@
$(TARGET): $(COBJS)  $(CXXOBJS)
	$(CXX) ${CXXFLAGS} $^ $(LDLIBS) -o $@
all: $(TARGET)
clean:
	rm -f $(TARGET)
	rm -f $(COBJS)  $(CXXOBJS)
	#find ./ -name "*.ox" -exec rm {} \;

distclean: clean
