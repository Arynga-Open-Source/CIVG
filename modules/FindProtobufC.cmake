# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

include(FindPkgConfig)
pkg_check_modules(PROTOBUFC libprotobuf-c)

if(NOT PROTOBUFC_FOUND)
    find_path(PROTOBUFC_INCLUDE_DIRS google/protobuf-c/protobuf-c.h
        "/usr/local/include"
        "/usr/include"
    )

    find_library(PROTOBUFC_LIBRARIES libprotobuf-c.so
        ${PROTOBUFC_INCLUDE_DIRS}/../lib
        /usr/local/lib
        /usr/lib
    )

    if(PROTOBUFC_INCLUDE_DIRS)
        if(PROTOBUFC_LIBRARIES)
            set(PROTOBUFC_FOUND "YES")
            set(PROTOBUFC_LIBRARIES ${PROTOBUFC_LIBRARIES} ${CMAKE_DL_LIBS})
        endif(PROTOBUFC_LIBRARIES)
    endif(PROTOBUFC_INCLUDE_DIRS)

    include(FindPackageHandleStandardArgs)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(PROTOBUFC DEFAULT_MSG PROTOBUFC_LIBRARIES PROTOBUFC_INCLUDE_DIRS)
endif(NOT PROTOBUFC_FOUND)
