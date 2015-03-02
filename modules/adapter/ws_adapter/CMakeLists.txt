# Arynga CarSync(TM)
# 2014 Copyrights by Arynga Inc. All rights reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Proprietary and confidential.

project(ws_adapter)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -pedantic -Wextra -std=gnu99")

if (NOT TARGET ws_adapter)
    include_directories(${PROJECT_SOURCE_DIR}/..)
    include_directories(${PROJECT_SOURCE_DIR}/../../utils)
    include_directories(${GENERATED_FILES_DIR})
    include_directories(../../3rd_party/cJSON)
    include_directories(../../3rd_party/libwebsockets)


    add_library(ws_adapter STATIC
        ../adapter.h
        ws_adapter.c
        ws_adapter.h
        ../../3rd_party/cJSON/cJSON.h
        ../../3rd_party/cJSON/cJSON.c
        ../../3rd_party/libwebsockets/base64-decode.c
        ../../3rd_party/libwebsockets/extension.c
        ../../3rd_party/libwebsockets/extension-deflate-stream.c
        ../../3rd_party/libwebsockets/extension-deflate-stream.h
        ../../3rd_party/libwebsockets/extension-x-google-mux.c
        ../../3rd_party/libwebsockets/extension-x-google-mux.h
        ../../3rd_party/libwebsockets/handshake.c
        ../../3rd_party/libwebsockets/client-handshake.c
        ../../3rd_party/libwebsockets/libwebsockets.c
        ../../3rd_party/libwebsockets/libwebsockets.h
        ../../3rd_party/libwebsockets/md5.c
        ../../3rd_party/libwebsockets/parsers.c
        ../../3rd_party/libwebsockets/private-libwebsockets.h
        ../../3rd_party/libwebsockets/sha-1.c

    )

    find_package(Glib REQUIRED)
    if(GLIB_FOUND)
        include_directories(${GLIB_INCLUDE_DIRS})
        target_link_libraries(ws_adapter ${GLIB_LIBRARIES})
    endif(GLIB_FOUND)
    find_package( ZLIB REQUIRED )
    if ( ZLIB_FOUND )
        include_directories( ${ZLIB_INCLUDE_DIRS} )
        target_link_libraries( ws_adapter ${ZLIB_LIBRARIES} -lm)
    endif( ZLIB_FOUND )
    target_link_libraries(ws_adapter -lm)
    find_package(Threads REQUIRED)
    if(THREADS_FOUND)
        include_directories(${THREADS_INCLUDE_DIRS})
    endif(THREADS_FOUND)
endif()
