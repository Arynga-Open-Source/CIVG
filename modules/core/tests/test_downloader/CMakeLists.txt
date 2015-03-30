# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(test_downloader)
cmake_minimum_required(VERSION 2.8)

if (NOT TARGET test_downloader)
    include_directories(../../../downloader)
    include_directories(../../../utils)

    add_library(test_downloader STATIC
        test_downloader.h
        test_downloader.c
    )
endif()
