
SUBDIRS=wiicarutility controlboard
if HAVE_GTK
SUBDIRS += wiicargui
endif
SUBDIRS += wiimotecar

## Place generated object files (.o) into the same directory as their source
## files, in order to avoid collisions when non-recursive make is used.
AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = -I m4

STANDARD=gnu99
