
# Check for STLPort
AC_DEFUN([AX_LIB_STLPORT],
[
	AC_MSG_CHECKING([for STLPort])
	AC_LANG_PUSH(C++)
	AC_COMPILE_IFELSE(
	[
		AC_LANG_PROGRAM
		([[
#include <string>
#ifndef _STLPORT_VERSION
#error No STLPort
#endif
                ]])
        ],
        [AC_DEFINE([HAVE_STLPORT], 1, [If you have STLPort installed]) AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])]
	)
	AC_LANG_POP
])