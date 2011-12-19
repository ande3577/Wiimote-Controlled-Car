#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([Wiimote Controlled Car], [1.0], [https://github.com/ande3577/Wiimote-Controlled-Car/issues])
AC_PACKAGE_URL([https://github.com/ande3577/Wiimote-Controlled-Car])
AC_CONFIG_HEADERS([config.h])


AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE(-Wall)

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CC_C99
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])

case $target in
	*-*-mipsel*)
		AM_CONDITIONAL(TARGET_IS_PC,false)
		# Checks for libraries.
		# in OpenWRT this check will fail
		AC_CHECK_LIB([pthread], [pthread_create],,
			AC_MSG_ERROR([pthread library not found]))
		AC_CHECK_LIB([bluetooth], [hci_devid],,
			AC_MSG_ERROR([bluetooth library not found]))
		AC_CHECK_LIB([cwiid], [cwiid_open],,
			AC_MSG_ERROR([cwiid library not found]))
		AC_CHECK_LIB([m], [cos],,
			AC_MSG_ERROR([math library not found]))
		;;
	*)
		AM_CONDITIONAL(TARGET_IS_PC,true)
		;;
esac

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stddef.h stdint.h stdlib.h string.h sys/time.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([clock_gettime gettimeofday sqrt])

# Checks for installed packages
PKG_CHECK_MODULES(GTK, gtk+-2.0, AM_CONDITIONAL(HAVE_GTK,true), AM_CONDITIONAL(HAVE_GTK,false))

AC_CONFIG_FILES([Makefile
                 controlboard/Makefile
                 wiicargui/Makefile
                 wiicarutility/Makefile
                 wiimotecar/Makefile])
AC_OUTPUT
