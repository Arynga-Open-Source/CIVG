# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(dumm_downloader)
cmake_minimum_required(VERSION 2.8)

if (NOT TARGET dumm_downloader)
    include_directories(${PROJECT_SOURCE_DIR}/..)
    include_directories(${PROJECT_SOURCE_DIR}/../../utils)
    include_directories(${GENERATED_FILES_DIR})

    #dumm_dbus_client.h generation
    find_program(GLIB_DBUS_BINDING_TOOL NAME dbus-binding-tool PATHS /usr/bin /usr/local/bin)
    add_custom_command(OUTPUT ${GENERATED_FILES_DIR}/dumm_dbus_client.h
        COMMAND ${GLIB_DBUS_BINDING_TOOL} --mode=glib-client --output=${GENERATED_FILES_DIR}/dumm_dbus_client.h ${CMAKE_CURRENT_SOURCE_DIR}/dumm/xml/net.dumm.download.xml
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/dumm/xml/net.dumm.download.xml
    )
    #dumm specific object_marshal generation
    find_program(GLIB_GEN_MARSHAL_TOOL NAME glib-genmarshal PATHS /usr/bin /usr/local/bin)
    add_custom_command(OUTPUT ${GENERATED_FILES_DIR}/dumm_object_marshal.h ${GENERATED_FILES_DIR}/dumm_object_marshal.c
        COMMAND ${GLIB_GEN_MARSHAL_TOOL} --header --prefix=dumm_object_marshal ${CMAKE_CURRENT_SOURCE_DIR}/object_marshal.list > ${GENERATED_FILES_DIR}/dumm_object_marshal.h &&
                cat ${CMAKE_CURRENT_SOURCE_DIR}/object_marshal_pragma.c > ${GENERATED_FILES_DIR}/dumm_object_marshal.c &&
                ${GLIB_GEN_MARSHAL_TOOL} --body --prefix=dumm_object_marshal ${CMAKE_CURRENT_SOURCE_DIR}/object_marshal.list >> ${GENERATED_FILES_DIR}/dumm_object_marshal.c
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/object_marshal.list
    )

    add_library(dumm_downloader STATIC
        ${GENERATED_FILES_DIR}/dumm_dbus_client.h
        ${GENERATED_FILES_DIR}/dumm_object_marshal.h
        ${GENERATED_FILES_DIR}/dumm_object_marshal.c
        ../downloader.h
        dumm_downloader.h
        dumm_downloader.c
    )

    find_package(Glib REQUIRED)
    if(GLIB_FOUND)
        include_directories(${GLIB_INCLUDE_DIRS})
    endif(GLIB_FOUND)

    find_package(DBus REQUIRED)
    if(DBUS_FOUND)
        include_directories(${DBUS_INCLUDE_DIRS})
    endif(DBUS_FOUND)
endif()
