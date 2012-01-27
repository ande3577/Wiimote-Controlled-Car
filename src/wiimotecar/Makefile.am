# Copyright (C) 2011 David S Anderson
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

bin_PROGRAMS=wiimotecarapp

wiimotecarapp_SOURCES=ControlTasks.c main.c wiicar_math.c WiiMotor.c
wiimotecarapp_LDADD = ../controlboard/libcontrolboard.la ../wiicarutility/libwiicarutility.la
AM_CPPFLAGS = -I ../ 

#this has to be added here for OpenWRT, since AC_SEARCH_LIBS not working
wiimotecarapp_LDFLAGS = -lm -lcwiid -lbluetooth


if HAVE_GTK
AM_CPPFLAGS += -DHAVE_GTK=1 -export-dynamic `pkg-config --cflags --libs gtk+-2.0`
wiimotecarapp_LDFLAGS += -export-dynamic `pkg-config --cflags --libs gtk+-2.0`
wiimotecarapp_LDADD += 	../wiicargui/libwiicargui.la
endif