# Arynga CarSync(TM)
# 2014-2015 Copyrights by Arynga Inc. All rights reserved.

project(civg_service_tests)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../modules)

include(../CMakeCFlags.txt)

# set path where generated files will be stored
set(GENERATED_FILES_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_FILES_DIR})

# dirs definitions
set(civg_service_tests_definitions
        CONFIG_DIR=\"${CMAKE_CURRENT_BINARY_DIR}/config\"
        LOGS_DIR=\"${CMAKE_CURRENT_BINARY_DIR}/logs\"
        WORK_DIR=\"${CMAKE_CURRENT_BINARY_DIR}/work\")
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "${civg_service_tests_definitions}")
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/config)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/logs)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/work)

add_subdirectory(../modules/utils ${CMAKE_CURRENT_BINARY_DIR}/utils)
add_subdirectory(../modules/proto ${CMAKE_CURRENT_BINARY_DIR}/proto)
add_subdirectory(../modules/dispatcher/glib_dispatcher ${CMAKE_CURRENT_BINARY_DIR}/dispatcher/glib_dispatcher)
add_subdirectory(../modules/carinfo/dummy_carinfo ${CMAKE_CURRENT_BINARY_DIR}/carinfo/dummy_carinfo)
add_subdirectory(../modules/transport/rmq_transport ${CMAKE_CURRENT_BINARY_DIR}/rmq_transport)
add_subdirectory(../modules/downloader/dumm_downloader ${CMAKE_CURRENT_BINARY_DIR}/dumm_downloader)
add_subdirectory(../modules/rpmanager/basic_imagehandler ${CMAKE_CURRENT_BINARY_DIR}/basic_imagehandler)
add_subdirectory(../modules/rpmanager ${CMAKE_CURRENT_BINARY_DIR}/rpmanager)
add_subdirectory(../modules/core ${CMAKE_CURRENT_BINARY_DIR}/core)

include_directories(../modules/utils/config/tests)
add_subdirectory(../modules/utils/config/tests
                    ${CMAKE_CURRENT_BINARY_DIR}/utils/config/tests)
include_directories(../modules/utils/filetools/tests)
add_subdirectory(../modules/utils/filetools/tests
                    ${CMAKE_CURRENT_BINARY_DIR}/utils/filetools/tests)
include_directories(../modules/dispatcher/glib_dispatcher/tests)
add_subdirectory(../modules/dispatcher/glib_dispatcher/tests
                    ${CMAKE_CURRENT_BINARY_DIR}/dispatcher/glib_dispatcher/tests)
include_directories(../modules/utils/tests)
add_subdirectory(../modules/utils/tests
                    ${CMAKE_CURRENT_BINARY_DIR}/utils/tests)
include_directories(../modules/core/tests)
add_subdirectory(../modules/core/tests
                    ${CMAKE_CURRENT_BINARY_DIR}/core/tests)
include_directories(../modules/proto/tests)
add_subdirectory(../modules/proto/tests
                    ${CMAKE_CURRENT_BINARY_DIR}/proto/tests)
include_directories(../modules/downloader/dumm_downloader/tests)
add_subdirectory(../modules/downloader/dumm_downloader/tests
                    ${CMAKE_CURRENT_BINARY_DIR}/downloader/dumm_downloader/tests)
include_directories(../modules/rpmanager/tests)
add_subdirectory(../modules/rpmanager/tests
                    ${CMAKE_CURRENT_BINARY_DIR}/rpmanager/tests)
include_directories(../modules/rpmanager/basic_imagehandler/tests)
add_subdirectory(../modules/rpmanager/basic_imagehandler/tests
                    ${CMAKE_CURRENT_BINARY_DIR}/rpmanager/basic_imagehandler/tests)
include_directories(../modules/transport/tests)
add_subdirectory(../modules/transport/tests
                    ${CMAKE_CURRENT_BINARY_DIR}/transport/tests)
include_directories(../modules/transport/rmq_transport/tests)
add_subdirectory(../modules/transport/rmq_transport/tests
                    ${CMAKE_CURRENT_BINARY_DIR}/transport/rmq_transport/tests)

add_executable(civg_general_tests
        main_general.c
)
target_link_libraries(civg_general_tests dumm_downloader_tests rmq_transport_tests glib_dispatcher_tests utils_tests utils_filetools_tests utils_config_tests proto_tests rpmanager_tests basic_imagehandler_tests)
target_link_libraries(civg_general_tests core rpmanager basic_imagehandler dummy_carinfo rmq_transport dumm_downloader glib_dispatcher proto utils)

#separate tests executable for core tests, linked with transport,downloader and rmpanager mocks
add_executable(civg_core_tests
        main_core.c
)
target_link_libraries(civg_core_tests core_tests)
target_link_libraries(civg_core_tests core dummy_carinfo proto utils)

#separate tests executable for threaded transport, linked with test_transport
add_executable(civg_threaded_tests
        main_threaded.c
)
target_link_libraries(civg_threaded_tests threaded_transport_tests test_threaded_transport)
target_link_libraries(civg_threaded_tests glib_dispatcher utils)

#separate tests executable for threaded transport, linked with test_transport
add_executable(civg_config_tests
        main_config.c
)
target_link_libraries(civg_config_tests utils_config_tests utils)

file(COPY test_all DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

find_package(Threads REQUIRED)
if(THREADS_FOUND)
    target_link_libraries(civg_general_tests ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(civg_threaded_tests ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(civg_config_tests ${CMAKE_THREAD_LIBS_INIT})
endif(THREADS_FOUND)

find_package(Glib REQUIRED)
if(GLIB_FOUND)
    target_link_libraries(civg_general_tests ${GLIB_LIBRARIES})
    target_link_libraries(civg_threaded_tests ${GLIB_LIBRARIES})
endif(GLIB_FOUND)

find_package(DBus REQUIRED)
if(DBUS_FOUND)
    target_link_libraries(civg_general_tests ${DBUS_LIBRARIES})
endif(DBUS_FOUND)
if(DBUS_GLIB_FOUND)
    target_link_libraries(civg_general_tests ${DBUS_GLIB_LIBRARIES})
endif(DBUS_GLIB_FOUND)

find_package(CUnit REQUIRED)
if(CUNIT_FOUND)
    include_directories(${CUNIT_INCLUDE_DIRS})
    target_link_libraries(civg_general_tests ${CUNIT_LIBRARIES})
    target_link_libraries(civg_core_tests ${CUNIT_LIBRARIES})
    target_link_libraries(civg_threaded_tests ${CUNIT_LIBRARIES})
    target_link_libraries(civg_config_tests ${CUNIT_LIBRARIES})
endif(CUNIT_FOUND)
