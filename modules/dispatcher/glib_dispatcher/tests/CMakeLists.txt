# Arynga CarSync(TM)
# 2014 Copyrights by Arynga Inc. All rights reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Proprietary and confidential.

project(glib_dispatcher_tests)
cmake_minimum_required(VERSION 2.8)

if(NOT TARGET glib_dispatcher_tests)

    include_directories(../)
    include_directories(../../)
    include_directories(../../../utils)

    add_library(glib_dispatcher_tests STATIC
            glib_dispatcher_tests.h
            glib_dispatcher_tests.c
    )

    find_package(Glib REQUIRED)
    if(GLIB_FOUND)
        include_directories(${GLIB_INCLUDE_DIRS})
    endif(GLIB_FOUND)
    find_package(Threads REQUIRED)
    if(THREADS_FOUND)
        include_directories(${THREADS_INCLUDE_DIRS})
    endif(THREADS_FOUND)
    find_package(CUnit REQUIRED)
    if(CUNIT_FOUND)
        include_directories(${CUNIT_INCLUDE_DIRS})
    endif(CUNIT_FOUND)
endif()
