# Arynga CarSync(TM)
# 2014 Copyrights by Arynga Inc. All rights reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Proprietary and confidential.

project(test_threaded_transport)
cmake_minimum_required(VERSION 2.8)

if (NOT TARGET test_threaded_transport)

    include_directories(../../)
    include_directories(../../../utils)

    add_library(test_threaded_transport STATIC
        ../../transport.h
        ../../transport_config.h
        ../../threaded_transport.h
        ../../threaded_transport.c
        ../../threaded_producer.h
        ../../threaded_consumer.h
        test_config.h
        test_config.c
        test_producer.h
        test_producer.c
        test_consumer.h
        test_consumer.c
    )

    find_package(Threads REQUIRED)
    if(THREADS_FOUND)
        include_directories(${THREADS_INCLUDE_DIRS})
    endif(THREADS_FOUND)
endif()
