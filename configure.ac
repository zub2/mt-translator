AC_INIT([mt-translator], [0.1], [zub@linux.fjfi.cvut.cz])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SRCDIR([src/mt-translator.c])
AC_PROG_CC
AC_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([MTDEV], [mtdev])
AC_SUBST(MTDEV_CFLAGS)
AC_SUBST(MTDEV_LIBS)

CFLAGS="-std=c99 $CFLAGS"
AC_DEFINE([_POSIX_C_SOURCE], [200809L], [POSIX feature test macro])

#AC_ARG_WITH([uinput],
#	[AS_HELP_STRING([--with-uinput],
#		[support uinput @<:@default=check@:>@])],
#	[],
#	[with_uinput=check])
#
#AS_CASE(["$with_uinput"],
#	[yes], [HAVE_LINUX_UINPUT_H=1],
#	[no], [HAVE_LINUX_UINPUT_H=0],
#	[AC_CHECK_HEADERS([linux/uinput.h])])

# FIXME: this is broken...
AM_CONDITIONAL([USE_UINPUT], [test "$with_uinput" != "no"]) # -a "${HAVE_LINUX_UINPUT_H}" -eq 1])

AC_CONFIG_FILES([ \
	Makefile \
	src/Makefile \
])

AC_CONFIG_HEADERS([config.h])
AC_OUTPUT
