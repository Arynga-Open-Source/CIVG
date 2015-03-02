# Arynga CarSync(TM)
# 2014 Copyrights by Arynga Inc. All rights reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Proprietary and confidential.

include(FindPkgConfig)
pkg_check_modules(LIBSOUP libsoup-2.4)

if(NOT LIBSOUP_FOUND)
  message(FATAL_ERROR "libsoup-2.4 is required but not available")
endif(NOT LIBSOUP_FOUND)
