# Arynga CarSync(TM)
# 2014 Copyrights by Arynga Inc. All rights reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Proprietary and confidential.

project(dummy_carinfo)
cmake_minimum_required(VERSION 2.8)

if (NOT TARGET dummy_carinfo)
    include_directories(${PROJECT_SOURCE_DIR}/..)
    include_directories(${PROJECT_SOURCE_DIR}/../../utils)

    add_library(dummy_carinfo STATIC
        ../carinfo.h
        dummy_carinfo_priv.h
        dummy_carinfo.c
    )
endif()
