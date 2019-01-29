CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(MeshSync)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
find_package(Poco COMPONENTS Net QUIET)
if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    mark_as_advanced(COCOA_LIBRARY)
    set(PLATFORM "Mac")
    set(MAC ON)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(PLATFORM "Linux")
    set(LINUX ON)
endif()

include(AddPlugin)
include(ISPC)

option(ENABLE_ZSTD "Use ZSTD for scene cache compression" ON)
if(ENABLE_ZSTD)
    find_package(ZSTD)
    add_definitions(-DmsEnableZSTD)
endif()

# Server
option(BUILD_SERVER "Build MeshSyncServer" ON)

# Metasequoia
if(MAC)
    option(BUILD_METASEQUOIA4_CLIENT "Build MeshSyncClient for Metasequoia 4" OFF)
endif()

# Blender
option(BUILD_BLENDER2.79_CLIENT "Build MeshSyncClient for Blender 2.79" OFF)
option(BUILD_BLENDER2.80_CLIENT "Build MeshSyncClient for Blender 2.80" OFF)

# Maya
option(BUILD_MAYA2015_CLIENT "Build MeshSyncClient for Maya 2015" OFF)
option(BUILD_MAYA2016_CLIENT "Build MeshSyncClient for Maya 2016" OFF)
option(BUILD_MAYA2016.5_CLIENT "Build MeshSyncClient for Maya 2016.5" OFF)
option(BUILD_MAYA2017_CLIENT "Build MeshSyncClient for Maya 2017" OFF)
option(BUILD_MAYA2018_CLIENT "Build MeshSyncClient for Maya 2018" OFF)
option(BUILD_MAYA2019_CLIENT "Build MeshSyncClient for Maya 2019" OFF)

# MotionBuilder
if(LINUX)
    option(BUILD_MOTIONBUILDER2015_CLIENT "Build MeshSyncClient for MotionBuilder 2015" OFF)
    option(BUILD_MOTIONBUILDER2016_CLIENT "Build MeshSyncClient for MotionBuilder 2016" OFF)
    option(BUILD_MOTIONBUILDER2017_CLIENT "Build MeshSyncClient for MotionBuilder 2017" OFF)
    option(BUILD_MOTIONBUILDER2018_CLIENT "Build MeshSyncClient for MotionBuilder 2018" OFF)
    option(BUILD_MOTIONBUILDER2019_CLIENT "Build MeshSyncClient for MotionBuilder 2019" OFF)
endif()

# Tests
option(BUILD_TESTS "Tests" OFF)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -fPIC -std=c++14 -Wno-deprecated")

function(add_subdirectory_link to from)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${to}")
        execute_process(WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" COMMAND ln -s "${from}" "${to}")
    endif()
    add_subdirectory("${to}")
endfunction()

add_subdirectory(MeshUtils)
add_subdirectory(MeshSync)


# Server
if(BUILD_SERVER)
    add_subdirectory(MeshSyncServer)
endif()

# Metasequoia
if(BUILD_METASEQUOIA4_CLIENT)
    add_subdirectory(MeshSyncClientMQ)
endif()

# Blender
if(BUILD_BLENDER2.79_CLIENT)
    add_subdirectory_link(MeshSyncClientBlender2.79 MeshSyncClientBlender)
endif()
if(BUILD_BLENDER2.80_CLIENT)
    add_subdirectory_link(MeshSyncClientBlender2.80 MeshSyncClientBlender)
endif()

# Maya
if(BUILD_MAYA2015_CLIENT)
    add_subdirectory_link(MeshSyncClientMaya2015 MeshSyncClientMaya)
endif()
if(BUILD_MAYA2016_CLIENT)
    add_subdirectory_link(MeshSyncClientMaya2016 MeshSyncClientMaya)
endif()
if(BUILD_MAYA2016.5_CLIENT)
    add_subdirectory_link(MeshSyncClientMaya2016.5 MeshSyncClientMaya)
endif()
if(BUILD_MAYA2017_CLIENT)
    add_subdirectory_link(MeshSyncClientMaya2017 MeshSyncClientMaya)
endif()
if(BUILD_MAYA2018_CLIENT)
    add_subdirectory_link(MeshSyncClientMaya2018 MeshSyncClientMaya)
endif()
if(BUILD_MAYA2019_CLIENT)
    add_subdirectory_link(MeshSyncClientMaya2019 MeshSyncClientMaya)
endif()
if(BUILD_MAYALT2018_CLIENT)
    add_subdirectory_link(MeshSyncClientMayaLT2018 MeshSyncClientMaya)
endif()

# MotionBuilder
if(BUILD_MOTIONBUILDER2015_CLIENT)
    add_subdirectory_link(MeshSyncClientMotionBuilder2015 MeshSyncClientMotionBuilder)
endif()
if(BUILD_MOTIONBUILDER2016_CLIENT)
    add_subdirectory_link(MeshSyncClientMotionBuilder2016 MeshSyncClientMotionBuilder)
endif()
if(BUILD_MOTIONBUILDER2017_CLIENT)
    add_subdirectory_link(MeshSyncClientMotionBuilder2017 MeshSyncClientMotionBuilder)
endif()
if(BUILD_MOTIONBUILDER2018_CLIENT)
    add_subdirectory_link(MeshSyncClientMotionBuilder2018 MeshSyncClientMotionBuilder)
endif()
if(BUILD_MOTIONBUILDER2019_CLIENT)
    add_subdirectory_link(MeshSyncClientMotionBuilder2019 MeshSyncClientMotionBuilder)
endif()

# Tests
if(BUILD_TESTS)
    add_subdirectory(Test)
endif()

