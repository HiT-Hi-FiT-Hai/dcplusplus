dnl Process this file with autoconf to produce a configure script.
AC_INIT([dcpp], 0.40325)
AC_CONFIG_SRCDIR(client/ADLSearch.cpp)
AC_GNU_SOURCE

AM_INIT_AUTOMAKE
			      
dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h limits.h malloc.h sys/ioctl.h sys/time.h unistd.h asm/atomic.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

dnl check for stlport
AC_MSG_CHECKING([for STLPort])
AC_LANG_PUSH(C++)
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[
#ifndef _STLPORT_VERSION
#error No STLPort
#endif
		]]
	)],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])]
)
AC_LANG_POP

dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(gettimeofday mkdir socket strerror strstr)

AC_CONFIG_FILES([Makefile client/Makefile])
dnl AM_CONFIG_HEADERS([client/config.h])

AC_OUTPUT
