# -*- makefile -*-
# 
#	makefile for socket library
#
# @author Yvan Castilloux

# create all libraries -> call all makefiles in subdirectories
SUBTARGETS=libhttpcomm libpipecomm libconfigio

all:
	for d in $(SUBTARGETS); do \
		make -C $$d; \
	done

clean:
	for d in $(SUBTARGETS); do \
		make -C $$d clean; \
	done
