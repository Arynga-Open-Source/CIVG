###########################################################################
# Copyright 2013 Open-RnD Sp. zo.o.
#
# __OPEN-RND_COPYRIGHTS_HERE__
#
#
# AUTHOR      : Bartlomiej Jozwiak (bj@open-rnd.pl)
# DATE        : 22-03-2013
# BRIEF       : __BRIEF__
# DESCRIPTION :
#              __DESCRIPTION__
#
###########################################################################

AC_PREREQ(2.61)
AC_INIT([DUMM], [0.1], [bj@open-rnd.pl], [DUMM.tar])
AC_CONFIG_SRCDIR([src/dumm.h])
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_MAKE_SET
AM_INIT_AUTOMAKE([1.9 tar-pax foreign])
AM_SILENT_RULES([yes])

AC_CONFIG_HEADER([dumm-config.h])

# set common flags
AC_SUBST(flagsboth, "$flagsboth -fno-strict-aliasing")
AC_SUBST(flagsboth, "$flagsboth -Dlinux")
AC_SUBST(flagsboth, "$flagsboth -Wall")
AC_SUBST(flagsboth, "$flagsboth -fkeep-inline-functions")

AC_SUBST(AM_CFLAGS, "$AM_CFLAGS $flagsboth $flagsc")
AC_SUBST(AM_CXXFLAGS, "$AM_CXXFLAGS $flagsboth $flagscxx")
AC_SUBST(AM_CPPFLAGS, "")

DUMMY=''; AC_SUBST(DUMMY)

# define some consts
AC_SUBST(AM_TOP_SRC, "\$(abs_top_srcdir)")
AC_SUBST(AM_TOP_BUILD, "\$(top_builddir)")

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
LT_INIT

AC_CANONICAL_HOST
# Qnx cross compile detection
_QNX_BUILD=no
AS_CASE([$host],
        [*-*-*qnx*],
            [_QNX_BUILD=yes])
AM_CONDITIONAL([QNX_BUILD], [ test x$_QNX_BUILD == xyes ] )
AM_COND_IF([QNX_BUILD], AC_DEFINE([QNX_BUILD], [1], [QNX cross compilation] ) , )

PKG_CHECK_MODULES(GLIB, [ glib-2.0 >= 2.16.0 ])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(DBUS_GLIB, [dbus-glib-1])
AC_SUBST(DBUS_GLIB_CFLAGS)
AC_SUBST(DBUS_GLIB_LIBS)

PKG_CHECK_MODULES(GTHREAD, [gthread-2.0])
AC_SUBST(GTHREAD_CFLAGS)
AC_SUBST(GTHREAD_LIBS)

GLIB_GENMARSHAL=`$PKG_CONFIG --variable=glib_genmarshal glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)

PKG_CHECK_MODULES([CURL], [libcurl], ,
                  [AC_SEARCH_LIBS([curl_easy_init], [curl],[CURL_LIBS=-lcurl],
                                  [AC_MSG_ERROR([Missing curl library])])])
AC_SUBST(CURL_LIBS)
AC_SUBST(CURL_CFLAGS)

PKG_CHECK_MODULES(SQLITE3, [sqlite3], ,
                  [AC_SEARCH_LIBS([sqlite3_free], [sqlite3], [SQLITE3_LIBS=-lsqlite3],
                                  [AC_MSG_ERROR([Missing sqlite3 library])], [-lz] )])
AC_SUBST(SQLITE3_CFLAGS)
AC_SUBST(SQLITE3_LIBS)

PKG_CHECK_MODULES(SYSTEMD, [systemd],
                           [with_systemd=yes], [with_systemd=no])

if test x$with_systemd = xyes; then
AC_SUBST(SYSTEMDSYSTEMUNITDIR,
        [$($PKG_CONFIG --variable=systemdsystemunitdir systemd)])
fi

AM_CONDITIONAL([WITH_SYSTEMD], [test x$with_systemd = xyes])

# Files to generate
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([tests/Makefile])
AC_CONFIG_FILES([others/genivi-dumm.service])
# Output
AC_OUTPUT
