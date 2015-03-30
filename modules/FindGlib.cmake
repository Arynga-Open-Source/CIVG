# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

include(FindPkgConfig)
pkg_check_modules(GLIB glib-2.0)

if(NOT GLIB_FOUND)
    find_path(GLIB_INCLUDE_DIRS glib.h
      /usr/local/include
      /usr/include
    )

    find_library(GLIB_LIBRARIES glib
           ${GLIB_INCLUDE_DIRS}/../lib
           /usr/local/lib
           /usr/lib
    )

    if(GLIB_INCLUDE_DIRS)
        if(GLIB_LIBRARIES)
            set(GLIB_FOUND "YES")
            set(GLIB_LIBRARIES ${GLIB_LIBRARIES} ${CMAKE_DL_LIBS})
        endif(GLIB_LIBRARIES)
    endif(GLIB_INCLUDE_DIRS)

    include(FindPackageHandleStandardArgs)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLIB DEFAULT_MSG GLIB_LIBRARIES GLIB_INCLUDE_DIRS)
endif(NOT GLIB_FOUND)
