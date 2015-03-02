# Arynga CarSync(TM)
# 2014 Copyrights by Arynga Inc. All rights reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Proprietary and confidential.

project(rmq_transport)
cmake_minimum_required(VERSION 2.8)

if (NOT TARGET rmq_transport)
    include_directories(..)
    include_directories(../../dispatcher)
    include_directories(../../utils)
    include_directories(../../proto)
    include_directories(${GENERATED_FILES_DIR})

    add_library(rmq_transport STATIC
        ../transport.h
        ../transport_config.h
        ../threaded_transport.h
        ../threaded_transport.c
        ../threaded_producer.h
        ../threaded_consumer.h
        ../msgqueue.h
        ../msgqueue.c
        rmq_tools.h
        rmq_config.h
        rmq_config_priv.h
        rmq_config.c
        rmq_producer.c
        rmq_consumer.h
        rmq_consumer.c
    )
    add_dependencies(rmq_transport proto librabbitmq)

    include_directories(../../3rd_party/rabbitmq-c-0.5.2/librabbitmq)
    include(ExternalProject)
    ExternalProject_Add(librabbitmq
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/rabbitmq-c-0.5.2
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/rabbitmq
        INSTALL_COMMAND "")
    target_link_libraries(rmq_transport "${CMAKE_CURRENT_BINARY_DIR}/rabbitmq/librabbitmq/librabbitmq.a")

    find_package(Threads REQUIRED)
    if(THREADS_FOUND)
        include_directories(${THREADS_INCLUDE_DIRS})
    endif(THREADS_FOUND)
endif()
