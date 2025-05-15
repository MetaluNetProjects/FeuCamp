# use : make pdincludepath=/path/to/pure-data/src/
#
# The following command will build the external and install the distributable
# files into a subdirectory called build/moonlib :
#
# make install pdincludepath=../pure-data/src/ objectsdir=./build

FWPATH = ../fw_feucamp

cflags = -I. -I$(FWPATH) -I$(FWPATH)/hw -g -Wno-unused-parameter -std=c++17
lib.name = emulator

emulator.class.sources = emulator.cpp audiolayer.cpp $(FWPATH)/sound/osc.cpp

PDLIBBUILDER_DIR ?= .
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder

