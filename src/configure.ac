#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])

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

AC_CONFIG_FILES([Makefile
                 controlboard/Makefile
                 wiicarutility/Makefile
                 wiimotecar/Makefile])
AC_OUTPUT
