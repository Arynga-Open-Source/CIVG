# Arynga CarSync(TM)
# 2014 Copyrights by Arynga Inc. All rights reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Proprietary and confidential.

project(utils_tests)
cmake_minimum_required(VERSION 2.8)

if(NOT TARGET utils_tests)

    include_directories(../)

    add_library(utils_tests STATIC
            strtools_tests.h
            strtools_tests.c
    )

    find_package(CUnit REQUIRED)
    if(CUNIT_FOUND)
        include_directories(${CUNIT_INCLUDE_DIRS})
    endif(CUNIT_FOUND)
endif()

