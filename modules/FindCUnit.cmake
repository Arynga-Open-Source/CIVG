# Arynga CarSync(TM)
# 2014 Copyrights by Arynga Inc. All rights reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Proprietary and confidential.
 
include(FindPkgConfig)
pkg_check_modules(CUNIT cunit)

if(NOT CUNIT_FOUND)
    find_path(CUNIT_INCLUDE_DIRS NAMES CUnit/CUnit.h)

    find_library(CUNIT_LIBRARIES
            cunit
            libcunit
            cunitlib
    )

    if(CUNIT_INCLUDE_DIRS)
        if(CUNIT_LIBRARIES)
            set(CUNIT_FOUND "YES")
            set(CUNIT_LIBRARIES ${CUNIT_LIBRARIES} ${CMAKE_DL_LIBS} )
        endif(CUNIT_LIBRARIES)
    endif(CUNIT_INCLUDE_DIRS)

    include(FindPackageHandleStandardArgs)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(CUnit DEFAULT_MSG CUNIT_LIBRARIES CUNIT_INCLUDE_DIRS)
endif(NOT CUNIT_FOUND)

