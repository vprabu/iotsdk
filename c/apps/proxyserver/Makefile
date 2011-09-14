# -*- makefile -*-
# 
#	makefile for writing configurations into a file
#
# @author Yvan Castilloux
# @author David Moss

include ../../support/make/Makefile.include

# What is the main file we want to compile
TARGET = proxyserver

# Which file(s) are we trying to test
SOURCES_C = ${TARGET}.c

SOURCES_C += ./clientmanager/proxyclientmanager.c
SOURCES_C += ./agent/proxyagent.c
SOURCES_C += ./cli/proxycli.c
SOURCES_C += ./activation/proxyactivation.c
SOURCES_C += ./proxymanager/proxymanager.c

SOURCES_C += ../../iot/proxy/proxy.c
SOURCES_C += ../../iot/proxy/proxylisteners.c
SOURCES_C += ../../iot/proxy/proxyconfig.c
SOURCES_C += ../../iot/proxy/h2swrapper.c
SOURCES_C += ../../iot/eui64/eui64.c
SOURCES_C += ../../iot/utils/timestamp.c
SOURCES_C += ../../iot/xml/generator/iotxmlgen.c
SOURCES_C += ../../iot/xml/parser/iotparser.c
SOURCES_C += ../../iot/xml/parser/iotcommandlisteners.c

# Which test(s) are we trying to run
SOURCES_CPP = 

# Where is the IOT include directory
CFLAGS += -I../../include

# Where are all of our directories we should include
CFLAGS += -I./
CFLAGS += -I./clientmanager
CFLAGS += -I./agent
CFLAGS += -I./cli
CFLAGS += -I./activation
CFLAGS += -I./proxymanager
CFLAGS += -I../../iot/proxy 
CFLAGS += -I../../iot/eui64 
CFLAGS += -I../../iot/utils
CFLAGS += -I../../iot/xml
CFLAGS += -I../../iot/xml/generator
CFLAGS += -I../../iot/xml/parser

# What 3rd party library headerse should we include. 
# Version information is pulled from support/make/Makefile.include
CFLAGS += -I../../lib/3rdparty/${LIBXML2_VERSION}/include
CFLAGS += -I../../lib/3rdparty/${LIBCURL_VERSION}/include
CFLAGS += -I../../lib/3rdparty/${OPENSSL_VERSION}/include

# Version information
CFLAGS += -DGIT_FIRMWARE_VERSION=$(shell git log -1 --pretty=format:0x%h)

CC = gcc
CPP = g++
AR = ar
STRIP=strip
INTEL = 0
export HARDWARE_PLATFORM = INTEL

OBJECTS_C = $(SOURCES_C:.c=.o)
OBJECTS_CPP = $(SOURCES_CPP:.cpp=.o)

LDEXTRA += -L../../lib -liotxml -lhttpcomm -lpipecomm -lxml2 -lconfigio -lcurl -lpthread -lm
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

	
