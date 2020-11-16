CMAKE_MINIMUM_REQUIRED(VERSION 3.18)
PROJECT(MeshSyncPlugin)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Build/cmake_modules)
set(src_root ${CMAKE_CURRENT_SOURCE_DIR}/Src)

# allow CMake to organize files by folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Custom scripts
include(AddPlugin)

# prerequisites
find_package(Poco REQUIRED COMPONENTS Net)
include(ISPC)

# ----------------------------------------------------------------------------------------------------------------------

if(APPLE)
    set(PLATFORM "Mac")
    set(MAC ON)
elseif(ANDROID)
    set(PLATFORM "Android")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(PLATFORM "Linux")
    set(LINUX ON)

elseif(WIN32) 
    add_definitions(-DmuEnablePPL)

    # Additional Poco Net dependencies on Windows
    list(APPEND Poco_LIBRARIES "ws2_32.lib;Iphlpapi.lib" )           
    
    #Compile with MultiThreaded DLL flag to use dynamically-linked runtime library (Poco)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")    
    
    #Set /fp. We need so that -infinity == infinity == NaN. See Transform::setupDataFlags()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast")
    
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

# Force Unicode character set
add_definitions(-DUNICODE -D_UNICODE)

# Use Poco static lib
add_definitions(-DPOCO_STATIC)

# ----------------------------------------------------------------------------------------------------------------------

# Options
option(ENABLE_ZSTD "Use ZSTD for scene cache compression" ON)
if(ENABLE_ZSTD)
    find_package(ZSTD)
    add_definitions(-DmsEnableZSTD)
endif()

option(BUILD_TESTS "Tests" OFF)

# ----------------------------------------------------------------------------------------------------------------------


#Specify available modules
add_subdirectory(${src_root}/MeshUtils)
add_subdirectory(${src_root}/MeshSync)
add_subdirectory(${src_root}/mscore)

# Tests
if(BUILD_TESTS)
    add_subdirectory(${src_root}/MeshSyncTest)
endif()

