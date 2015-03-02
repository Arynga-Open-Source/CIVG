# Arynga CarSync(TM)
# 2014 Copyrights by Arynga Inc. All rights reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Proprietary and confidential.

project(basic_imagehandler)
cmake_minimum_required(VERSION 2.8)

if (NOT TARGET basic_imagehandler)
    include_directories(${PROJECT_SOURCE_DIR}/..)
    include_directories(${PROJECT_SOURCE_DIR}/../../utils)

    add_library(basic_imagehandler STATIC
        ../imagehandler.h
        basic_imagehandler.c
    )
endif()
