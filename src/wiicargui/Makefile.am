lib_LTLIBRARIES = libwiicargui.la
libwiicargui_la_SOURCES = wiicargui.c
include_HEADERS = wiicargui.h

libwiicargui_la_LIBADD = ../controlboard/libcontrolboard.la ../wiicarutility/libwiicarutility.la
AM_CPPFLAGS = -I ../ -export-dynamic `pkg-config --cflags --libs gtk+-2.0`
libwiicargui_la_LDFLAGS = -export-dynamic `pkg-config --cflags --libs gtk+-2.0`