# Arynga CarSync(TM)
# 2014 Copyrights by Arynga Inc. All rights reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Proprietary and confidential.

project(utils_filetools_tests)
cmake_minimum_required(VERSION 2.8)

if(NOT TARGET utils_filetools_tests)

    include_directories(../../)

    add_library(utils_filetools_tests STATIC
            filetools_tests.h
            filetools_tests.c
    )

    file(COPY test.file DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY test.tar DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    find_package(CUnit REQUIRED)
    if(CUNIT_FOUND)
        include_directories(${CUNIT_INCLUDE_DIRS})
    endif(CUNIT_FOUND)
endif()

