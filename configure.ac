dnl Process this file with autoconf to produce a configure script.
AC_INIT([DCPlusPlus], 0.667)
AC_CONFIG_AUX_DIR(build)
AM_INIT_AUTOMAKE(foreign)
AC_CONFIG_SRCDIR(client/ADLSearch.cpp)
AC_GNU_SOURCE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_MAKE_SET

dnl Checks for libraries.
AC_CHECK_LIB(pthread, pthread_create)
AC_CHECK_LIB(bz2, BZ2_bzCompressInit)
AC_CHECK_LIB(z, deflateInit)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h limits.h malloc.h sys/ioctl.h sys/time.h unistd.h asm/atomic.h pthread.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_CHECK_TYPES([long long])
AC_CHECK_DECLS([PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP],[],[],[#include <pthread.h>])
AC_C_BIGENDIAN
AC_CHECK_SIZEOF(long)

dnl check for stlport
AX_LIB_STLPORT


dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(gettimeofday mkdir socket strerror strstr)

AC_CONFIG_FILES([Makefile client/Makefile])
AC_CONFIG_HEADERS([client/autoconf.h])

AC_OUTPUT
