# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(proto_tests)
cmake_minimum_required(VERSION 2.8)

if(NOT TARGET proto_tests)

    include_directories(../)
    include_directories(../../utils)
    include_directories(${GENERATED_FILES_DIR})

    add_library(proto_tests STATIC
            proto_tests.h
            proto_tests.c
    )
    add_dependencies(proto_tests proto)

    file(COPY UF.meta DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    find_package(CUnit REQUIRED)
    if(CUNIT_FOUND)
        include_directories(${CUNIT_INCLUDE_DIRS})
    endif(CUNIT_FOUND)
endif()

