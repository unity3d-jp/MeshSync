CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(MeshSync)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
find_package(OpenEXR QUIET)
find_package(Poco COMPONENTS Net QUIET)
if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    mark_as_advanced(COCOA_LIBRARY)
endif()

include(AddPlugin)
include(ISPC)

OPTION(BUILD_SERVER "Build MeshSyncServer" ON)
OPTION(BUILD_METASEQUOIA4_CLIENT "Build MeshSyncClient for Metasequoia 4" OFF)
OPTION(BUILD_BLENDER279_CLIENT "Build MeshSyncClient for Blender 2.79" OFF)
OPTION(BUILD_BLENDER28_CLIENT "Build MeshSyncClient for Blender 2.8" OFF)
OPTION(BUILD_MAYA2018_CLIENT "Build MeshSyncClient for Maya 2018" OFF)
OPTION(BUILD_MAYA2017_CLIENT "Build MeshSyncClient for Maya 2017" OFF)
OPTION(BUILD_MAYA2016_CLIENT "Build MeshSyncClient for Maya 2016" OFF)
OPTION(BUILD_TESTS "Tests" OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -Wno-deprecated")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()


function(add_subdirectory_link from to)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${to}")
        execute_process(WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" COMMAND ln -s "${from}" "${to}")
    endif()
    add_subdirectory("${to}")
endfunction()

add_subdirectory(MeshUtils)
add_subdirectory(MeshSync)

if(BUILD_SERVER)
    add_subdirectory(MeshSyncServer)
endif()

if(BUILD_METASEQUOIA4_CLIENT)
    add_subdirectory(MeshSyncClientMQ)
endif()

if(BUILD_BLENDER279_CLIENT)
    add_subdirectory_link(MeshSyncClientBlender MeshSyncClientBlender2.79)
endif()
if(BUILD_BLENDER28_CLIENT)
    add_subdirectory_link(MeshSyncClientBlender MeshSyncClientBlender2.8)
endif()

if(BUILD_MAYA2018_CLIENT)
    add_subdirectory_link(MeshSyncClientMaya MeshSyncClientMaya2018)
endif()
if(BUILD_MAYA2017_CLIENT)
    add_subdirectory_link(MeshSyncClientMaya MeshSyncClientMaya2017)
endif()
if(BUILD_MAYA2016_CLIENT)
    add_subdirectory_link(MeshSyncClientMaya MeshSyncClientMaya2016)
endif()

if(BUILD_TESTS)
    add_subdirectory(Test)
endif()

