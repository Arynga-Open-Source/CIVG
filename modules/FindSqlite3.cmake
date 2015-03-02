# Arynga CarSync(TM)
# 2014 Copyrights by Arynga Inc. All rights reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Proprietary and confidential.

include(FindPkgConfig)
pkg_check_modules(SQLITE3 sqlite3)

if(NOT SQLITE3_FOUND)
    find_path(SQLITE3_INCLUDE_DIRS sqlite3.h
      /usr/local/include
      /usr/include
    )

    find_library(SQLITE3_LIBRARIES sqlite3
           ${SQLITE3_INCLUDE_DIRS}/../lib
           /usr/local/lib
           /usr/lib
    )

    if(SQLITE3_INCLUDE_DIRS)
        if(SQLITE3_LIBRARIES)
            set(SQLITE3_FOUND "YES")
            set(SQLITE3_LIBRARIES ${SQLITE3_LIBRARIES} ${CMAKE_DL_LIBS})
        endif(SQLITE3_LIBRARIES)
    endif(SQLITE3_INCLUDE_DIRS)

    include(FindPackageHandleStandardArgs)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(SQLITE3 DEFAULT_MSG SQLITE3_LIBRARIES SQLITE3_INCLUDE_DIRS)
endif(NOT SQLITE3_FOUND)
