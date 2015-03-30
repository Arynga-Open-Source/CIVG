# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(rvi_transport)
cmake_minimum_required(VERSION 2.8)

if (NOT TARGET rvi_transport)
    include_directories(..)
    include_directories(../../dispatcher)
    include_directories(../../utils)
    include_directories(../../proto)
    include_directories(${GENERATED_FILES_DIR})
    include_directories(../../3rd_party/cJSON)

    add_library(rvi_transport STATIC
        ../transport.h
        ../transport_config.h
        ../threaded_transport.h
        ../threaded_transport.c
        ../threaded_producer.h
        ../threaded_consumer.h
        ../msgqueue.h
        ../msgqueue.c
        rvi_tools.h
        rvi_tools.c
        rvi_config.h
        rvi_config_priv.h
        rvi_config.c
        rvi_producer.c
        rvi_consumer.c
    )
    add_dependencies(rvi_transport proto)

    find_package(LibSoup REQUIRED)
    if(LIBSOUP_FOUND)
        include_directories(${LIBSOUP_INCLUDE_DIRS})
        target_link_libraries(rvi_transport ${LIBSOUP_LIBRARIES})
    endif(LIBSOUP_FOUND)

    find_package(Glib REQUIRED)
    if(GLIB_FOUND)
        include_directories(${GLIB_INCLUDE_DIRS})
        target_link_libraries(rvi_transport ${GLIB_LIBRARIES})
    endif(GLIB_FOUND)

    find_package(Threads REQUIRED)
    if(THREADS_FOUND)
        include_directories(${THREADS_INCLUDE_DIRS})
    endif(THREADS_FOUND)
endif()
