# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(glib_dispatcher)
cmake_minimum_required(VERSION 2.8)

if (NOT TARGET glib_dispatcher)
    include_directories(${PROJECT_SOURCE_DIR}/..)
    include_directories(${PROJECT_SOURCE_DIR}/../../utils)

    add_library(glib_dispatcher STATIC
        ../dispatcher.h
        glib_dispatcher.h
        glib_dispatcher.c
    )

    find_package(Glib REQUIRED)
    if(GLIB_FOUND)
        include_directories(${GLIB_INCLUDE_DIRS})
    endif(GLIB_FOUND)
endif()
