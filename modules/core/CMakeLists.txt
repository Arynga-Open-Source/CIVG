# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(core)
cmake_minimum_required(VERSION 2.8)

if (NOT TARGET core)
    include_directories(../utils)
    include_directories(../rpmanager)
    include_directories(../transport)
    include_directories(../downloader)
    include_directories(../carinfo)
    include_directories(../proto)
    include_directories(${GENERATED_FILES_DIR})

    add_library(core STATIC
        core.h
        core_priv.h
        core.c
        downloadmgr.h
        downloadmgr_priv.h
        downloadmgr.c
        downloaditem.h
        downloaditem.c
        vbsclient.h
        vbsclient.c
    )
    add_dependencies(core proto)
endif()
