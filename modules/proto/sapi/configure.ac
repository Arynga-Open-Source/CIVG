#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([carsync-common-south-proto], [0.1.0], [dev@arynga.com])
AM_INIT_AUTOMAKE([1.9 foreign])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
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

AC_SUBST(AM_CXXFLAGS, "$AM_CXXFLAGS -std=c++0x")

# DEF_LANG(LANG, desc, default=yes|no, compiler)
# --------------------------
AC_DEFUN([DEF_LANG],
[

define([LANG_LCASE_NAME], translit($1, 'A-Z', 'a-z'))
AC_ARG_ENABLE(lang-LANG_LCASE_NAME, AC_HELP_STRING([--enable-lang-LANG_LCASE_NAME],
                                    [enable $2 support (default [$3])]),
              [lang_[$1]_enable=$enableval],
              [lang_[$1]_enable=$3])

AC_CHECK_PROG(PROTOC_[$1], [$4], [$4])
if test x$lang_[$1]_enable = xyes; then
   if test x$PROTOC_[$1] = x; then
      AC_MSG_ERROR($4 needed for LANG_LCASE_NAME not found)
   fi
fi

AM_CONDITIONAL(BUILD_$1, [test x$lang_$1_enable = xyes])

undefine([LANG_LCASE_NAME])
])

# C and Python enabled by default
DEF_LANG([C], "C", yes, protoc-c)
DEF_LANG([PYTHON], "Python", yes, protoc)
# C++ and Erlang disabled by default
DEF_LANG([CPP], "C++", no, protoc)
DEF_LANG([ERLANG], "Erlang", no, protoc-erl)

# Checks for libraries.
PKG_CHECK_MODULES(PROTOBUF, [protobuf],
                            [protobuf_found=yes],
                            [protobuf_found=no])

AC_CHECK_HEADERS(google/protobuf-c/protobuf-c.h)
AC_SEARCH_LIBS(protobuf_c_message_pack, [protobuf-c], [protobuf_c_found=yes],
                        [protobuf_c_found=no], [-lz] )

# check if dependencies for c++ stubs are met
if test x$lang_CPP_enable = xyes; then
   if test x$protobuf_found != xyes; then
      AC_MSG_ERROR(protobuf development headers needed for C++ were not found)
   fi
fi

# check if dependencies for c stubs are met
# c++ and c libraries are compiled, check for required tools
if test x$lang_C_enable = xyes; then
   if test x$protobuf_c_found != xyes; then
      AC_MSG_ERROR(protobuf-c development headers needed for C were not found)
   fi
fi

AC_SUBST(PROTOBUF_CFLAGS)
AC_SUBST(PROTOBUF_LIBS)

AC_SUBST(PROTOBUF_C_CFLAGS)
AC_SUBST(PROTOBUF_C_LIBS)

PKG_CHECK_MODULES(CARSYNC_COMMON, [carsync-hu-common])
AC_SUBST(CARSYNC_COMMON_CFLAGS)
AC_SUBST(CARSYNC_COMMON_LIBS)

# define some consts
#AC_SUBST(AM_TOP_SRC, "\$(top_srcdir)")
AC_SUBST(AM_TOP_SRC, "\$(abs_top_srcdir)")
#AC_SUBST(AM_TOP_ABS_SRC, "\$(abs_top_srcdir)")
AC_SUBST(AM_TOP_BUILD, "\$(top_builddir)")

AC_CONFIG_FILES([Makefile
                 cpp/Makefile
                 cpp/carsync-hu-proto-cpp.pc
                 python/Makefile
                 c/Makefile
                 c/carsync-hu-proto-c.pc
                 erlang/Makefile
                 test/Makefile])
AC_OUTPUT

echo "Enabled languages:"
echo "   C: $lang_C_enable"
echo "   C++: $lang_CPP_enable"
echo "   Python: $lang_PYTHON_enable"
echo "   Erlang: $lang_ERLANG_enable"
