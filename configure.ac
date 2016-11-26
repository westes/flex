#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# This file is part of flex.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:

# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.

# Neither the name of the University nor the names of its contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.

# THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE.

# autoconf requirements and initialization

AC_INIT([the fast lexical analyser generator],[2.6.3],[flex-help@lists.sourceforge.net],[flex])
AC_CONFIG_SRCDIR([src/scan.l])
AC_CONFIG_AUX_DIR([build-aux])
LT_INIT
AM_INIT_AUTOMAKE([-Wno-portability foreign check-news std-options dist-lzip parallel-tests subdir-objects 1.14.1])
AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_LIBOBJ_DIR([lib])
AC_CONFIG_MACRO_DIR([m4])
SHARED_VERSION_INFO="2:0:0"
AC_SUBST(SHARED_VERSION_INFO)

# checks for programs

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19])
AC_PROG_YACC
AM_PROG_LEX
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_LN_S
AC_PROG_AWK
AC_PROG_INSTALL

# allow passing a variable `WARNINGFLAGS',
#   either when invoking `configure', or when invoking `make'
# default to something useful if GCC was detected

AC_ARG_ENABLE([warnings],
  [AS_HELP_STRING([--enable-warnings],
                  [enable a bunch of compiler warning flags (defaults to GCC warning flags).])],
  [AS_IF([test "x$GCC" = xyes],
    [ : ${WARNINGFLAGS="-Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wnested-externs -Wold-style-definition -Wredundant-decls -Wconversion -Wno-unused-but-set-variable"} ])])

AC_SUBST([WARNINGFLAGS])

AC_ARG_ENABLE([libfl],
  [AS_HELP_STRING([--disable-libfl],
                  [do not build -lfl runtime support library])],
  [], [enable_libfl=yes])
AM_CONDITIONAL([ENABLE_LIBFL], [test "x$enable_libfl" = xyes])

# --disable-bootstrap is intended only to workaround problems with bootstrap
# (e.g. when cross-compiling flex or when bootstrapping has bugs).
# Ideally we should be able to bootstrap even when cross-compiling.
AC_ARG_ENABLE([bootstrap],
  [AS_HELP_STRING([--disable-bootstrap],
                  [don't perform a bootstrap when building flex])],
  [], [enable_bootstrap=yes])
AM_CONDITIONAL([ENABLE_BOOTSTRAP], [test "x$enable_bootstrap" = xyes])

AC_PATH_PROG([BISON], bison, no)
AS_IF([test "$BISON" != no],[],
	[ AC_SUBST([BISON], [\${top_srcdir}/build-aux/missing bison])
	  AC_MSG_NOTICE(no bison program found: only required for maintainers)
	])

AC_PATH_PROG([HELP2MAN], help2man, [\${top_srcdir}/build-aux/missing help2man])
  AS_IF([test "$HELP2MAN" = "\${top_srcdir}/build-aux/missing help2man"],
    AC_MSG_WARN(help2man: program not found: building man page will not work)
  )

AC_PATH_PROGS([TEXI2DVI], [gtexi2dvi texi2dvi], [\${top_srcdir}/build-aux/missing texi2dvi])
  AS_IF([test "$TEXI2DVI" = "\${top_srcdir}/build-aux/missing texi2dvi"],
    AC_MSG_WARN(texi2dvi: program not found: building pdf version of manual will not work)
  )

# Check for a m4 that supports -P

AC_CACHE_CHECK([for m4 that supports -P], [ac_cv_path_M4],
[AC_PATH_PROGS_FEATURE_CHECK([M4], [gm4 gnum4 m4],
              [[m4out=`echo 'm''4_divnum' | $ac_path_M4 -P`]
                [test "x$m4out" = x0 \
                && ac_cv_path_M4=$ac_path_M4 ac_path_M4_found=:]],
              [AC_MSG_ERROR([could not find m4 that supports -P])])])
AC_SUBST([M4], [$ac_cv_path_M4])
AC_DEFINE_UNQUOTED([M4], ["$M4"], [Define to the m4 executable name.])

AC_PATH_PROG(INDENT, indent, indent)
# if INDENT is set to 'indent' then we didn't find indent
if test "$INDENT" != indent ; then
   AC_MSG_CHECKING(if $INDENT is GNU indent)
   if $INDENT --version 2>/dev/null | head -n 1|grep "GNU indent" > /dev/null ; then
      AC_MSG_RESULT(yes)
   else
      AC_MSG_RESULT(no)
      AC_MSG_WARN($INDENT does not appear to be GNU indent.)
   fi
else
   AC_MSG_WARN(no indent program found: make indent target will not function)
fi

# checks for headers

AC_CHECK_HEADERS([inttypes.h libintl.h limits.h locale.h malloc.h netinet/in.h regex.h unistd.h])

# checks for libraries

# The test test-pthread uses libpthread, so we check for it here, but
# all we need is the preprocessor symbol defined since we don't need
# LIBS to include libpthread for building flex.

AC_CHECK_LIB(pthread, pthread_mutex_lock,
AC_DEFINE([HAVE_LIBPTHREAD], 1, [pthread library] ),
AC_DEFINE([HAVE_LIBPTHREAD], 0, [pthread library] )
)
AC_CHECK_HEADERS([pthread.h])
AM_CONDITIONAL([want_pthread], [test "x$ac_cv_lib_pthread_pthread_mutex_lock" = xyes])

AC_CHECK_LIB(m, log10)

# Checks for typedefs, structures, and compiler characteristics.

AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.

AC_FUNC_ALLOCA
AC_FUNC_FORK
dnl Autoconf bug: AC_FUNC_MALLOC and AC_FUNC_REALLOC might not warn of cross
dnl compilation. Workaround this.
AC_FUNC_MALLOC
AS_IF([test "$cross_compiling" = yes],
   AC_MSG_WARN([result $ac_cv_func_malloc_0_nonnull guessed because of cross compilation]))
AC_FUNC_REALLOC
AS_IF([test "$cross_compiling" = yes],
   AC_MSG_WARN([result $ac_cv_func_realloc_0_nonnull guessed because of cross compilation]))

AC_CHECK_FUNCS(dup2 dnl
memset dnl
pow dnl
regcomp dnl
setlocale dnl
strchr dnl
strdup dnl
strtol)

AC_REPLACE_FUNCS(reallocarray)

AC_CONFIG_FILES(
Makefile
doc/Makefile
examples/Makefile
examples/fastwc/Makefile
examples/manual/Makefile
lib/Makefile
po/Makefile.in
src/Makefile
tools/Makefile
tests/Makefile
)

AC_OUTPUT
