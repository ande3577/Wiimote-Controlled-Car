bin_PROGRAMS=wiimotecarapp

wiimotecarapp_SOURCES=ControlTasks.c main.c wiicar_math.c WiiMotor.c
wiimotecarapp_LDADD = ../controlboard/libcontrolboard.la ../wiicarutility/libwiicarutility.la

AM_CPPFLAGS = -I ../

#this has to be added here for OpenWRT, since AC_SEARCH_LIBS not working
wiimotecarapp_LDFLAGS = -lm -lcwiid -lbluetooth
