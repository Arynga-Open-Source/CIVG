# Arynga CarSync(TM)
# 2014 Copyrights by Arynga Inc. All rights reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Proprietary and confidential.

project(core_tests)
cmake_minimum_required(VERSION 2.8)

if(NOT TARGET core_tests)

    include_directories(../)
    include_directories(../../utils)
    include_directories(../../rpmanager)
    include_directories(../../transport)
    include_directories(../../downloader)
    include_directories(../../carinfo)
    include_directories(../../carinfo/dummy_carinfo)
    include_directories(../../proto)
    include_directories(${GENERATED_FILES_DIR})
    add_subdirectory(test_transport)
    add_subdirectory(test_downloader)
    add_subdirectory(test_rpmanager)

    add_library(core_tests STATIC
            core_tests.h
            core_tests.c
            downloaditem_tests.h
            downloaditem_tests.c
            downloadmgr_tests.h
            downloadmgr_tests.c
            vbsclient_tests.h
            vbsclient_tests.c
    )
    target_link_libraries(core_tests test_transport test_downloader test_rpmanager)

    file(COPY test.uf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    find_package(CUnit REQUIRED)
    if(CUNIT_FOUND)
        include_directories(${CUNIT_INCLUDE_DIRS})
    endif(CUNIT_FOUND)
endif()

