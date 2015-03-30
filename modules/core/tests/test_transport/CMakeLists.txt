# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(test_transport)
cmake_minimum_required(VERSION 2.8)

if (NOT TARGET test_transport)
    include_directories(${PROJECT_SOURCE_DIR}/../../../transport)

    add_library(test_transport STATIC
        test_transport.h
        test_transport.c
    )
endif()
