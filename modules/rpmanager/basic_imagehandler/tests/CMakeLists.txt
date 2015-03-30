# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(basic_imagehandler_tests)
cmake_minimum_required(VERSION 2.8)

if(NOT TARGET basic_imagehandler_tests)

    include_directories(../../)
    include_directories(../../../utils)

    add_library(basic_imagehandler_tests STATIC
            basic_imagehandler_tests.h
            basic_imagehandler_tests.c
    )

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test1)
    file(COPY test2 DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test2/data)
    file(COPY test3 DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test3/data)
    file(COPY test4 DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test4/data)

    find_package(CUnit REQUIRED)
    if(CUNIT_FOUND)
        include_directories(${CUNIT_INCLUDE_DIRS})
    endif(CUNIT_FOUND)
endif()

