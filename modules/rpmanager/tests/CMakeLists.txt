# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(rpmanager_tests)
cmake_minimum_required(VERSION 2.8)

function(PrepareUFStructure RPUUID UFUUID )
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${RPUUID}/${UFUUID}/data)
    file(COPY ivi4-demo.patch DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${RPUUID}/${UFUUID}/data)
endfunction(PrepareUFStructure)


if(NOT TARGET rpmanager_tests)

    include_directories(../)
    include_directories(../../utils)
    include_directories(../../proto)
    include_directories(${GENERATED_FILES_DIR})

    add_library(rpmanager_tests STATIC
            rpmanager_tests.h
            rpmanager_tests.c
    )

    file(COPY ./1234567890 DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    PrepareUFStructure(1234567890 111222333)
    PrepareUFStructure(1234567890 444555666)
    PrepareUFStructure(1234567890 777888999)

    file(COPY ./1020304050 DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    PrepareUFStructure(1020304050 111222333)
    PrepareUFStructure(1020304050 444555666)
    PrepareUFStructure(1020304050 777888999)

    file(COPY ./9999999999 DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    PrepareUFStructure(9999999999 111222333)
    PrepareUFStructure(9999999999 444555666)
    PrepareUFStructure(9999999999 777888999)

    file(COPY ./8888888888 DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/8888888888/444555666/data)
    # malformed file to get wrong checksum
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/8888888888/444555666/data)
    file(COPY ivi4-demo-wrong.patch DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/8888888888/444555666/data)
    file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/8888888888/444555666/data/ivi4-demo-wrong.patch
            ${CMAKE_CURRENT_BINARY_DIR}/8888888888/444555666/data/ivi4-demo.patch)

    find_package(CUnit REQUIRED)
    if(CUNIT_FOUND)
        include_directories(${CUNIT_INCLUDE_DIRS})
    endif(CUNIT_FOUND)
endif()

