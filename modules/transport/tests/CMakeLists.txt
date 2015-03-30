# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(threaded_transport_tests)
cmake_minimum_required(VERSION 2.8)

if(NOT TARGET threaded_transport_tests)

    include_directories(../)
    include_directories(../../dispatcher)
    include_directories(../../dispatcher/glib_dispatcher)
    include_directories(../../utils)
    include_directories(test_threaded_transport)
    add_subdirectory(test_threaded_transport)

    add_library(threaded_transport_tests STATIC
            ../msgqueue.h
            ../msgqueue.c
            threaded_transport_tests.h
            threaded_transport_tests.c
            msgqueue_tests.h
            msgqueue_tests.c
    )
    target_link_libraries(threaded_transport_tests test_threaded_transport)

    find_package(Threads REQUIRED)
    if(THREADS_FOUND)
        include_directories(${THREADS_INCLUDE_DIRS})
    endif(THREADS_FOUND)
    find_package(Glib REQUIRED)
    if(GLIB_FOUND)
        include_directories(${GLIB_INCLUDE_DIRS})
    endif(GLIB_FOUND)
    find_package(CUnit REQUIRED)
    if(CUNIT_FOUND)
        include_directories(${CUNIT_INCLUDE_DIRS})
    endif(CUNIT_FOUND)
endif()
