
## Place generated object files (.o) into the same directory as their source
## files, in order to avoid collisions when non-recursive make is used.
AUTOMAKE_OPTIONS = subdir-objects

## Additional flags to pass to aclocal when it is invoked automatically at
## make time. The ${ACLOCAL_FLAGS} variable is picked up from the environment
## to provide a way for the user to supply additional arguments.
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS=wiimotecarapp

wiimotecarapp_SOURCES=wiicarutility/error_message.c wiicarutility/timestamp.c wiicarutility/utility.c

wiimotecarapp_SOURCES+=controlboard/comm.c controlboard/command.c

wiimotecarapp_SOURCES+=wiimotecar/ControlTasks.c wiimotecar/main.c 
wiimotecarapp_SOURCES+=wiimotecar/wiicar_math.c  wiimotecar/WiiMotor.c
	
wiimotecarapp_CFLAGS = -Iwiicarutility -Icontrolboard -g2 -O0
wiimotecarapp_LDFLAGS = -lm -lcwiid
