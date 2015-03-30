# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(utils)
cmake_minimum_required(VERSION 2.8)

if (NOT TARGET utils)

    add_definitions("-D_GNU_SOURCE")

    include_directories(./)
    include_directories(config)

    add_library(utils STATIC
                filetools.h
                filetools/filetools.c
                config.h
                config/config.c
                logger.h
                logger/logger.c
                strtools.h
                strtools.c
                civg_compiler.h
    )

    find_package(Sqlite3 REQUIRED)
    if(SQLITE3_FOUND)
        include_directories(${SQLITE_INCLUDE_DIRS})
        target_link_libraries(utils ${SQLITE3_LIBRARIES})
    endif(SQLITE3_FOUND)

    find_package(Threads REQUIRED)
    if(THREADS_FOUND)
        include_directories(${THREADS_INCLUDE_DIRS})
        target_link_libraries(utils ${CMAKE_THREAD_LIBS_INIT})
    endif(THREADS_FOUND)

    find_package(OpenSSL REQUIRED)
    if(OPENSSL_FOUND)
        include_directories(${OPENSSL_INCLUDE_DIR})
        target_link_libraries(utils ${OPENSSL_LIBRARIES})
    endif(OPENSSL_FOUND)

endif()
