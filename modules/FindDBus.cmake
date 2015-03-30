# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

include(FindPkgConfig)
pkg_check_modules(DBUS dbus-1)

if(NOT DBUS_FOUND)
    find_path(DBUS_INCLUDE_DIRS dbus.h
      /usr/local/include
      /usr/include
    )

    find_library(DBUS_LIBRARIES dbus
           ${DBUS_INCLUDE_DIRS}/../lib
           /usr/local/lib
           /usr/lib
    )

    if(DBUS_INCLUDE_DIRS)
        if(DBUS_LIBRARIES)
            set(DBUS_FOUND "YES")
            set(DBUS_LIBRARIES ${DBUS_LIBRARIES} ${CMAKE_DL_LIBS})
        endif(DBUS_LIBRARIES)
    endif(DBUS_INCLUDE_DIRS)

    include(FindPackageHandleStandardArgs)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(DBUS DEFAULT_MSG DBUS_LIBRARIES DBUS_INCLUDE_DIRS)
endif(NOT DBUS_FOUND)

pkg_check_modules(DBUS_GLIB dbus-glib-1)

if(NOT DBUS_GLIB_FOUND)
    find_path(DBUS_GLIB_INCLUDE_DIRS dbus-glib.h
      /usr/local/include
      /usr/include
    )

    find_library(DBUS_GLIB_LIBRARIES dbus-glib
           ${DBUS_GLIB_INCLUDE_DIRS}/../lib
           /usr/local/lib
           /usr/lib
    )

    if(DBUS_GLIB_INCLUDE_DIRS)
        if(DBUS_GLIB_LIBRARIES)
            set(DBUS_GLIB_FOUND "YES")
            set(DBUS_GLIB_LIBRARIES ${DBUS_GLIB_LIBRARIES} ${CMAKE_DL_LIBS})
        endif(DBUS_GLIB_LIBRARIES)
    endif(DBUS_GLIB_INCLUDE_DIRS)

    include(FindPackageHandleStandardArgs)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(DBUS DEFAULT_MSG DBUS_GLIB_LIBRARIES DBUS_GLIB_INCLUDE_DIRS)
endif(NOT DBUS_GLIB_FOUND)
