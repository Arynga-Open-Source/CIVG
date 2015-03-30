# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

include(FindPkgConfig)
pkg_check_modules(LIBSOUP libsoup-2.4)

if(NOT LIBSOUP_FOUND)
  message(FATAL_ERROR "libsoup-2.4 is required but not available")
endif(NOT LIBSOUP_FOUND)
