AC_INIT([mt-translator], [0.1], [zub@linux.fjfi.cvut.cz])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SRCDIR([src/mt-translator.c])
AC_PROG_CC
AC_PROG_LIBTOOL

PKG_CHECK_MODULES([MTDEV], [mtdev])
AC_SUBST(MTDEV_CFLAGS)
AC_SUBST(MTDEV_LIBS)

AC_CONFIG_FILES([ \
	Makefile \
	src/Makefile \
])

AC_OUTPUT
