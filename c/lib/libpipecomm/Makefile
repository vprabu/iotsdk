# -*- makefile -*-
# 
#	makefile for writing configurations into a file
#
# @author Yvan Castilloux

include ../../support/make/Makefile.include

LIB_NAME = libpipecomm
SOURCES = libpipecomm.c
RESULT_DIR = ./
CFLAGS += -I../../include

ifneq ($(HOST), mips-linux)
CFLAGS+= -g -pg
endif
ARFLAG = rcs

DYNLIB_EXTENSION = so
STATLIB_EXTENSION = a

LINK_FLAG = -shared -o $(RESULT_DIR)/$(LIB_NAME).so $(OBJECTS)

OBJECTS=$(SOURCES:.c=.o)
LDEXTRA+=$(PPCLIBPATH) $(LIBRT) -lpthread
LOCALINCLUDEPATH =

all: dynlib staticlib

clean:
	$(RM) -rf ./*.o ./*.d ./*.dll ./*.a ../*.a ./*.so ../*.so ../../include/libpipecomm.h $(LIB_NAME)
	
$(LIB_NAME): $(OBJECTS)
	$(CC) $(PPCINCLUDEPATH) $(LOCALINCLUDEPATH) $(LDFLAGS) -o $@ $(OBJECTS) $(LDEXTRA)

.c.o:
	$(CC) $(PPCINCLUDEPATH) $(LOCALINCLUDEPATH) $(CFLAGS) -c -o $@ $< 
	
staticlib: $(OBJECTS)
	$(AR) $(ARFLAG) $(RESULT_DIR)/$(LIB_NAME).$(STATLIB_EXTENSION) ${OBJECTS}
	@cp ./$(LIB_NAME).a ../$(LIB_NAME)a.a
	@mkdir -p ../../include
	@cp ./libpipecomm.h ../../include/.
	
dynlib: $(OBJECTS)
	$(CC) $(LINK_FLAG) $(LDEXTRA)
	@cp ./$(LIB_NAME).so ../
	@mkdir -p ../../include
	@cp ./libpipecomm.h ../../include/.