# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(civg_service)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)

include(CMakeCFlags.txt)

# add a target to generate documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)


# include dir for version*.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# set path where generated files will be stored
set(GENERATED_FILES_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_FILES_DIR})

set(CONFIG_DIR "var/lib/civg/config")
set(WORK_DIR "var/lib/civg/work")
set(LOGS_DIR "var/log")
set(DEMO_DIR "var/lib/civg/.demo")

# dirs definitions (tests has its own definitions)
set(civg_service_definitions
        CONFIG_DIR=\"${CMAKE_INSTALL_PREFIX}/${CONFIG_DIR}\"
        WORK_DIR=\"${CMAKE_INSTALL_PREFIX}/${WORK_DIR}\"
        LOGS_DIR=\"${CMAKE_INSTALL_PREFIX}/${LOGS_DIR}/civg\"
        DEMO_DIR=\"${CMAKE_INSTALL_PREFIX}/${DEMO_DIR}\" 
)
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "${civg_service_definitions}")

# package specific modules implementations
include_directories(modules/utils)
add_subdirectory(modules/utils)
include_directories(${GENERATED_FILES_DIR})
include_directories(modules/proto)
add_subdirectory(modules/proto)
include_directories(modules/dispatcher)
include_directories(modules/dispatcher/glib_dispatcher)
add_subdirectory(modules/dispatcher/glib_dispatcher)
include_directories(modules/carinfo/)
include_directories(modules/carinfo/dummy_carinfo)
add_subdirectory(modules/carinfo/dummy_carinfo)
include_directories(modules/transport)
include_directories(modules/transport/rvi_transport)
add_subdirectory(modules/transport/rvi_transport)
include_directories(modules/downloader)
include_directories(modules/downloader/dumm_downloader)
add_subdirectory(modules/downloader/dumm_downloader)
include_directories(modules/rpmanager)
add_subdirectory(modules/rpmanager/basic_imagehandler)
add_subdirectory(modules/rpmanager)
include_directories(modules/core)
add_subdirectory(modules/core)
include_directories(modules/adapter)
add_subdirectory(modules/adapter/ws_adapter)

add_executable(civg_service
    main_service.c
    modules/3rd_party/cJSON/cJSON.h
    modules/3rd_party/cJSON/cJSON.c
)
target_link_libraries(civg_service core rpmanager basic_imagehandler dummy_carinfo rvi_transport dumm_downloader glib_dispatcher ws_adapter proto utils -lm)

include_directories(modules/3rd_party/cJSON)
add_executable(civg_config
    main_config.c
    modules/3rd_party/cJSON/cJSON.h
    modules/3rd_party/cJSON/cJSON.c
)
target_link_libraries(civg_config proto utils -lm)
file(COPY config_example.json DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


find_package(Glib REQUIRED)
if(GLIB_FOUND)
    include_directories(${GLIB_INCLUDE_DIRS})
    target_link_libraries(civg_service ${GLIB_LIBRARIES})
endif(GLIB_FOUND)

find_package(DBus REQUIRED)
if(DBUS_FOUND)
    include_directories(${DBUS_INCLUDE_DIRS})
    target_link_libraries(civg_service ${DBUS_LIBRARIES})
endif(DBUS_FOUND)
if(DBUS_GLIB_FOUND)
    target_link_libraries(civg_service ${DBUS_GLIB_LIBRARIES})
endif(DBUS_GLIB_FOUND)

message(STATUS "Installing civg to: " ${CMAKE_INSTALL_PREFIX}/bin)
install(TARGETS civg_service civg_config
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
    RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
)
# make sure logs dir exists
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/civg)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/civg DESTINATION "${CMAKE_INSTALL_PREFIX}/${LOGS_DIR}")
# copy config json example
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config_example.json DESTINATION "${CMAKE_INSTALL_PREFIX}/${CONFIG_DIR}")
# copy script to reset demo and base data on which update is performed
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${DEMO_DIR})
