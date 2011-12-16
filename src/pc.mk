#####################################################################
##
##  avr.mk
##  
##  standard platform specific build options for GNU make (avr)
##
##
##  Rev 0.0.1 12/6/2010
##
#####################################################################

CC=gcc
CXX=g++
AR=ar
SIZE=size
OBJCOPY=objcopy
NM=nm
CP=cp

MAKEFILES=Makefile \
	makedefs.mk \
	config.mk \
	pc.mk \
	Makefile

EXTRA_CFLAGS+= -fPIC -Wall

EXTRA_CXXFLAGS+= $(EXTRA_CFLAGS)

MAP_FILE=$(TARGET_NAME).map

EXTRA_NMFLAGS = --format=posix
		
SIZE_FLAGS = 

