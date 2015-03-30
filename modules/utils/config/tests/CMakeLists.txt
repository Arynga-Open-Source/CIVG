# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(utils_config_tests)
cmake_minimum_required(VERSION 2.8)

if(NOT TARGET utils_config_tests)

    include_directories(../)
    include_directories(../../)

    add_library(utils_config_tests STATIC
            config_tests.h
            config_tests.c
    )

    find_package(CUnit REQUIRED)
    if(CUNIT_FOUND)
        include_directories(${CUNIT_INCLUDE_DIRS})
    endif(CUNIT_FOUND)
endif()

