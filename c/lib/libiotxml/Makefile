# -*- makefile -*-
# 
#	makefile for http communication with PPC web server
#
# @author Yvan Castilloux

include ../../support/make/Makefile.include

LIB_NAME = libiotxml
SOURCE_NAME = libiotxml
SOURCES = ../../iot/xml/parser/iotcommandlisteners.c
SOURCES += ../../iot/xml/parser/iotparser.c
SOURCES += ../../iot/xml/generator/iotxmlgen.c
SOURCES += ../../iot/eui64/eui64.c
SOURCES += ../../iot/utils/timestamp.c

RESULT_DIR = ./
CFLAGS += -I../../include
CFLAGS += -I../../iot/xml/
CFLAGS += -I../../iot/xml/parser
CFLAGS += -I../../iot/xml/generator
CFLAGS += -I../../iot/xml/generator
CFLAGS += -I../../iot/eui64/
CFLAGS += -I../../iot/utils/

# Include 3rd party libraries
CFLAGS += -I../3rdparty/${LIBXML2_VERSION}/include
CFLAGS += -I../3rdparty/${LIBCURL_VERSION}/include
CFLAGS += -I../3rdparty/${OPENSSL_VERSION}/include

ifneq ($(HOST), mips-linux)
CFLAGS+= -g -pg
endif
ARFLAG = rcs

DYNLIB_EXTENSION = so
STATLIB_EXTENSION = a

# Link Flag
#LINK_FLAG = -L$(RESULT_DIR) -shared -Wl,-soname,$(LIB_NAME).$(DYNLIB_EXTENSION) -lc -lm -lpthread

LINK_FLAG = -shared -o $(RESULT_DIR)/$(LIB_NAME).so $(OBJECTS)

OBJECTS=$(SOURCES:.c=.o)
LDEXTRA+=$(PPCLIBPATH) $(LIBRT) $(LIBXML2) $(LIBCURL) -lpthread
LOCALINCLUDEPATH =

all: dynlib staticlib

clean:
	$(RM) -rf ./*.o ./*.d ./*.dll ./*.a ../*.a ./*.so ../*.so ../../include/iotapi.h ../../include/eui64.h ../../include/timestamp.h $(LIB_NAME)
	
$(LIB_NAME): $(OBJECTS)
	@$(CC) $(PPCINCLUDEPATH) $(LOCALINCLUDEPATH) $(LDFLAGS) -o $@ $(OBJECTS) $(LDEXTRA)

.c.o:
	@$(CC) $(PPCINCLUDEPATH) $(LOCALINCLUDEPATH) $(CFLAGS) -c -o $@ $< 
	
staticlib: $(OBJECTS)
	@$(AR) $(ARFLAG) $(RESULT_DIR)/$(LIB_NAME).$(STATLIB_EXTENSION) ${OBJECTS}
	@cp ./$(LIB_NAME).a ../$(LIB_NAME).a
	@mkdir -p ../../include
	@cp ../../iot/xml/iotapi.h ../../include/.
	@cp ../../iot/eui64/eui64.h ../../include/.
	@cp ../../iot/utils/timestamp.h ../../include/.
	
dynlib: $(OBJECTS)
	@$(CC) $(LINK_FLAG) $(LDEXTRA)
	@cp ./$(LIB_NAME).so ../
	@mkdir -p ../../include
	@cp ../../iot/xml/iotapi.h ../../include/.
	@cp ../../iot/eui64/eui64.h ../../include/.
	@cp ../../iot/utils/timestamp.h ../../include/.
