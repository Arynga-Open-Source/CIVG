# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(rpmanager)
cmake_minimum_required(VERSION 2.8)

if (NOT TARGET rpmanager)
    include_directories(../utils)
    include_directories(../proto)
    include_directories(${GENERATED_FILES_DIR})

    add_library(rpmanager STATIC
        rpmanager.h
        rpmanager_priv.h
        rpmanager.c
    )
    add_dependencies(rpmanager proto)
endif()
