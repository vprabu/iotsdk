# -*- makefile -*-
# 
#	makefile for writing configurations into a file
#
# @author Yvan Castilloux
# @author David Moss

# Only run on this computer platform, not an embedded target platform
ifneq ($(HOST), mips-linux)

# Which file(s) are we trying to test
SOURCES_C = ../eui64.c

# Which test(s) are we trying to run
SOURCES_CPP = main.cpp  eui64_test.cpp 

# Where is the IOT include directory
CFLAGS += -I../../../include

# What directories should we include
CFLAGS += -I../


TARGET = unittest
CC = gcc
CPP = g++
AR = ar
STRIP=strip
INTEL = 0
export HARDWARE_PLATFORM = INTEL

OBJECTS_C = $(SOURCES_C:.c=.o)
OBJECTS_CPP = $(SOURCES_CPP:.cpp=.o)

LDEXTRA += -L../../../lib -lcppunit -lhttpcomm -lpipecomm -lcurl -lpthread -lm
LDFLAGS += -Wl,-rpath,/opt/lib

CFLAGS += -g3
CFLAGS += -Os
CFLAGS += -Wall


.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<
	
.cpp.o:
	$(CPP) -c $(CFLAGS) -o $@ $<

test: clean $(TARGET)

clean:
	@$(RM) -rf ./*.o $(TARGET) ../*.o ../src/*.o ../*.so *.xml ../../eui64/*.o
	
$(TARGET): lib $(OBJECTS_C) $(OBJECTS_CPP)
	$(CPP) ${CFLAGS} $(LDFLAGS) -o $@ $(OBJECTS_CPP) $(OBJECTS_C) $(LDEXTRA)

lib:
	make -s -C ../../../lib
	
endif
	