CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT(MeshSyncProj)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Build/cmake_modules)
set(src_root ${CMAKE_SOURCE_DIR}/Src)

# Custom scripts
include(AddPlugin)

# prerequisites
find_package(Poco COMPONENTS Net)
include(ISPC)

if(APPLE)
    set(PLATFORM "Mac")
    set(MAC ON)
elseif(ANDROID)
    set(PLATFORM "Android")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(PLATFORM "Linux")
    set(LINUX ON)
endif()


# ---
# Options
option(ENABLE_ZSTD "Use ZSTD for scene cache compression" ON)
if(ENABLE_ZSTD)
    find_package(ZSTD)
    add_definitions(-DmsEnableZSTD)
endif()

option(BUILD_TESTS "Tests" OFF)

# ---
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -fPIC -std=c++14 -Wno-deprecated")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

#Specify available modules
add_subdirectory(${src_root}/MeshUtils)
add_subdirectory(${src_root}/MeshSync)
add_subdirectory(${src_root}/mscore)

# Tests
if(BUILD_TESTS)
    add_subdirectory(${src_root}/MeshSyncTest)
endif()

