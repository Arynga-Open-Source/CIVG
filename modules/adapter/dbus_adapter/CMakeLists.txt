# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(dbus_adapter)
cmake_minimum_required(VERSION 2.8)

if (NOT TARGET dbus_adapter)
    include_directories(${PROJECT_SOURCE_DIR}/../../utils)
    include_directories(${GENERATED_FILES_DIR})

    #adapter_dbus_server.h generation
    find_program(GLIB_DBUS_BINDING_TOOL NAME dbus-binding-tool PATHS /usr/bin /usr/local/bin)
    add_custom_command(OUTPUT ${GENERATED_FILES_DIR}/adapter_dbus_server.h
        COMMAND ${GLIB_DBUS_BINDING_TOOL} --mode=glib-server --prefix=civg_adapter --output=${GENERATED_FILES_DIR}/adapter_dbus_server.h ${CMAKE_CURRENT_SOURCE_DIR}/com.arynga.CarSync.CIVGAdapter.xml
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/com.arynga.CarSync.CIVGAdapter.xml
    )
    #dumm specific object_marshal generation
    find_program(GLIB_GEN_MARSHAL_TOOL NAME glib-genmarshal PATHS /usr/bin /usr/local/bin)
    add_custom_command(OUTPUT ${GENERATED_FILES_DIR}/adapter_object_marshal.h ${GENERATED_FILES_DIR}/adapter_object_marshal.c
        COMMAND ${GLIB_GEN_MARSHAL_TOOL} --header --prefix=adapter_object_marshal ${CMAKE_CURRENT_SOURCE_DIR}/object_marshal.list > ${GENERATED_FILES_DIR}/adapter_object_marshal.h &&
                cat ${CMAKE_CURRENT_SOURCE_DIR}/object_marshal_pragma.c > ${GENERATED_FILES_DIR}/adapter_object_marshal.c &&
                ${GLIB_GEN_MARSHAL_TOOL} --body --prefix=adapter_object_marshal ${CMAKE_CURRENT_SOURCE_DIR}/object_marshal.list >> ${GENERATED_FILES_DIR}/adapter_object_marshal.c
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/object_marshal.list
    )

    add_library(dbus_adapter STATIC
        ${GENERATED_FILES_DIR}/adapter_dbus_server.h
        ${GENERATED_FILES_DIR}/adapter_object_marshal.h
        ${GENERATED_FILES_DIR}/adapter_object_marshal.c
        dbus_adapter.h
    )

    find_package(Glib REQUIRED)
    if(GLIB_FOUND)
        include_directories(${GLIB_INCLUDE_DIRS})
        target_link_libraries(dbus_adapter ${GLIB_LIBRARIES})
    endif(GLIB_FOUND)

    find_package(DBus REQUIRED)
    if(DBUS_FOUND)
        include_directories(${DBUS_INCLUDE_DIRS})
        target_link_libraries(dbus_adapter ${DBUS_LIBRARIES})
    endif(DBUS_FOUND)
    if(DBUS_GLIB_FOUND)
        target_link_libraries(dbus_adapter ${DBUS_GLIB_LIBRARIES})
    endif(DBUS_GLIB_FOUND)
endif()
