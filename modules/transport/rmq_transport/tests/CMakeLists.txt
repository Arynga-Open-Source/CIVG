# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(rmq_transport_tests)
cmake_minimum_required(VERSION 2.8)

if(NOT TARGET rmq_transport_tests)

    include_directories(../)
    include_directories(../../)
    include_directories(../../../dispatcher)
    include_directories(../../../dispatcher/glib_dispatcher)
    include_directories(../../../utils)
    include_directories(../../../proto)
    include_directories(${GENERATED_FILES_DIR})

    add_library(rmq_transport_tests STATIC
            rmq_transport_tests.h
            rmq_transport_tests.c
    )

    find_package(Glib REQUIRED)
    if(GLIB_FOUND)
        include_directories(${GLIB_INCLUDE_DIRS})
    endif(GLIB_FOUND)
    find_package(CUnit REQUIRED)
    if(CUNIT_FOUND)
        include_directories(${CUNIT_INCLUDE_DIRS})
    endif(CUNIT_FOUND)
endif()

