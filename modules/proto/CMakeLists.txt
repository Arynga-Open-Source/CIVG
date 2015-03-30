# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(proto)
cmake_minimum_required(VERSION 2.8)

if(NOT TARGET proto)

    include_directories(../utils)
    include_directories(${GENERATED_FILES_DIR})

    find_package(ProtobufC REQUIRED)
    if(PROTOBUFC_FOUND)
        find_program(PROTO_COMPILER_TOOL NAME protoc-c PATHS /usr/bin /usr/local/bin)
        include_directories(${PROTOBUFC_INCLUDE_DIRS})
    else()
        set(PROTO_COMPILER_TOOL ${CMAKE_CURRENT_BINARY_DIR}/protobuf-c/bin/protoc-c)
        include(ExternalProject)
        ExternalProject_Add(protobuf-c
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../3rd_party/protobuf-c-0.15
            BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/protobuf-c
            CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../3rd_party/protobuf-c-0.15/configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/protobuf-c
            BUILD_COMMAND make
            INSTALL_COMMAND make install && cp -a ${CMAKE_CURRENT_BINARY_DIR}/protobuf-c/include/google ${GENERATED_FILES_DIR})
    endif(PROTOBUFC_FOUND)

    #protobuf generation
    set(PROTO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sapi/proto )
    file(GLOB PROTO_SRC_FILES "${PROTO_SRC_DIR}/*.proto")
    add_custom_command(OUTPUT ${GENERATED_FILES_DIR}/common.pb-c.h
                              ${GENERATED_FILES_DIR}/releasepackage.pb-c.h
                              ${GENERATED_FILES_DIR}/southapi.pb-c.h
                              ${GENERATED_FILES_DIR}/common.pb-c.c
                              ${GENERATED_FILES_DIR}/releasepackage.pb-c.c
                              ${GENERATED_FILES_DIR}/southapi.pb-c.c
        COMMAND ${PROTO_COMPILER_TOOL} --c_out=${GENERATED_FILES_DIR} -I${PROTO_SRC_DIR} ${PROTO_SRC_FILES}
        DEPENDS ${PROTO_SRC_FILES}
    )

    add_library(proto STATIC
        ${GENERATED_FILES_DIR}/common.pb-c.h
        ${GENERATED_FILES_DIR}/common.pb-c.c
        ${GENERATED_FILES_DIR}/releasepackage.pb-c.h
        ${GENERATED_FILES_DIR}/releasepackage.pb-c.c
        ${GENERATED_FILES_DIR}/southapi.pb-c.h
        ${GENERATED_FILES_DIR}/southapi.pb-c.c
        proto.h
        proto.c
    )

    if(PROTOBUFC_FOUND)
        target_link_libraries(proto ${PROTOBUFC_LIBRARIES})
    else()
        add_dependencies(proto protobuf-c)
        target_link_libraries(proto "${CMAKE_CURRENT_BINARY_DIR}/protobuf-c/lib/libprotobuf-c.a")
    endif(PROTOBUFC_FOUND)

endif()
