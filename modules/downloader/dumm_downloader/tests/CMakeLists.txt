# Arynga CarSync(TM)
# 2014 Copyrights by Arynga Inc. All rights reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Proprietary and confidential.

project(dumm_downloader_tests)
cmake_minimum_required(VERSION 2.8)

if(NOT TARGET dumm_downloader_tests)

    include_directories(../)
    include_directories(../../)
    include_directories(../../../utils)

    add_library(dumm_downloader_tests STATIC
            dumm_downloader_tests.h
            dumm_downloader_tests.c
    )

    set(dumm_downloader_tests_definitions
            ${civg_service_tests_definitions}
            DOWNLOAD_DIR=\"${CMAKE_CURRENT_BINARY_DIR}/work/downloader\")
    set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "${dumm_downloader_tests_definitions}")
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/work/downloader)

    find_package(Glib REQUIRED)
    if(GLIB_FOUND)
        include_directories(${GLIB_INCLUDE_DIRS})
    endif(GLIB_FOUND)
    find_package(DBus REQUIRED)
    if(DBUS_FOUND)
        include_directories(${DBUS_INCLUDE_DIRS})
    endif(DBUS_FOUND)
    find_package(CUnit REQUIRED)
    if(CUNIT_FOUND)
        include_directories(${CUNIT_INCLUDE_DIRS})
    endif(CUNIT_FOUND)
endif()
