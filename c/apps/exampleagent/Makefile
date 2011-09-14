# -*- makefile -*-
# 
#	makefile for writing configurations into a file
#
# @author Yvan Castilloux
# @author David Moss

include ${IOTSDK}/c/support/make/Makefile.include

# What is the main file we want to compile
TARGET = gadgetagent

# Which file(s) are we using
SOURCES_C = ${TARGET}.c

SOURCES_C += ./control/gadgetcontrol.c
SOURCES_C += ./manager/gadgetmanager.c
SOURCES_C += ./discovery/gadgetdiscovery.c
SOURCES_C += ./heartbeat/gadgetheartbeat.c
SOURCES_C += ./measure/gadgetmeasure.c

SOURCES_C += ${IOTSDK}/c/iot/client/clientsocket.c
SOURCES_C += ${IOTSDK}/c/iot/proxy/proxy.c
SOURCES_C += ${IOTSDK}/c/iot/proxy/proxylisteners.c
SOURCES_C += ${IOTSDK}/c/iot/proxy/proxyconfig.c
SOURCES_C += ${IOTSDK}/c/iot/proxy/h2swrapper.c
SOURCES_C += ${IOTSDK}/c/iot/eui64/eui64.c
SOURCES_C += ${IOTSDK}/c/iot/utils/timestamp.c
SOURCES_C += ${IOTSDK}/c/iot/xml/generator/iotxmlgen.c
SOURCES_C += ${IOTSDK}/c/iot/xml/parser/iotparser.c
SOURCES_C += ${IOTSDK}/c/iot/xml/parser/iotcommandlisteners.c

SOURCES_C += ${IOTSDK}/c/lib/3rdparty/cJSON/cJSON.c

# Which test(s) are we trying to run
SOURCES_CPP = 

# Where is the IOT include directory
CFLAGS += -I${IOTSDK}/c/include

# What directories should we include
CFLAGS += -I./
CFLAGS += -I./control
CFLAGS += -I./manager
CFLAGS += -I./discovery
CFLAGS += -I./heartbeat
CFLAGS += -I./measure
CFLAGS += -I${IOTSDK}/c/apps/proxyserver
CFLAGS += -I${IOTSDK}/c/iot/proxy 
CFLAGS += -I${IOTSDK}/c/iot/eui64 
CFLAGS += -I${IOTSDK}/c/iot/client
CFLAGS += -I${IOTSDK}/c/iot/utils
CFLAGS += -I${IOTSDK}/c/iot/xml
CFLAGS += -I${IOTSDK}/c/iot/xml/generator
CFLAGS += -I${IOTSDK}/c/iot/xml/parser
CFLAGS += -I${IOTSDK}/c/lib/3rdparty/cJSON/

# What 3rd party library headerse should we include. 
# Version information is pulled from support/make/Makefile.include
CFLAGS += -I${IOTSDK}/c/lib/3rdparty/${LIBXML2_VERSION}/include
CFLAGS += -I${IOTSDK}/c/lib/3rdparty/${LIBCURL_VERSION}/include
CFLAGS += -I${IOTSDK}/c/lib/3rdparty/${OPENSSL_VERSION}/include
CFLAGS += -I${IOTSDK}/c/lib/3rdparty/${CJSON_VERSION}

CC = gcc
CPP = g++
AR = ar
STRIP=strip
INTEL = 0
export HARDWARE_PLATFORM = INTEL

OBJECTS_C = $(SOURCES_C:.c=.o)
OBJECTS_CPP = $(SOURCES_CPP:.cpp=.o)

LDEXTRA += -L${IOTSDK}/c/lib -liotxml -lhttpcomm -lpipecomm -lxml2 -lconfigio -lcurl -lpthread -lm
LDFLAGS += -Wl,-rpath,/opt/lib

CFLAGS += -Os
CFLAGS += -Wall


.bin:
	@mkdir -p ./bin
  
.c.o:
	@$(CC) -c $(CFLAGS) -o ./bin/$(shell basename $@) $<
	
.cpp.o:
	@mkdir ./bin
	@$(CPP) -c $(CFLAGS) -o ./bin/$(shell basename $@) $<

test: clean $(TARGET)

clean:
	@rm -rf ./*.o $(TARGET) ./bin
	
$(TARGET): .bin lib $(OBJECTS_C) $(OBJECTS_CPP)
	@$(CPP) ${CFLAGS} $(LDFLAGS) -o ./bin/$(shell basename $@) ./bin/*.o $(LDEXTRA)

lib:
	@make -s -C ../../lib


	
