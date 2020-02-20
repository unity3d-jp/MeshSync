CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT(MeshSync)

# Paths
# set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
#
# find_package(Poco COMPONENTS Net QUIET)
# if(APPLE)
#     find_library(COCOA_LIBRARY Cocoa)
#     mark_as_advanced(COCOA_LIBRARY)
#     set(PLATFORM "Mac")
#     set(MAC ON)
# elseif(ANDROID)
#     set(PLATFORM "Android")
# elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
#     set(PLATFORM "Linux")
#     set(LINUX ON)
# endif()

# include(AddPlugin)
# include(ISPC)
#
# option(ENABLE_ZSTD "Use ZSTD for scene cache compression" ON)
# if(ENABLE_ZSTD)
#     find_package(ZSTD)
#     add_definitions(-DmsEnableZSTD)
# endif()
#
# # Server
# option(BUILD_MSCORE "Build MeshSyncCore" ON)
#
# # Tests
# option(BUILD_TESTS "Tests" OFF)
#
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -fPIC -std=c++14 -Wno-deprecated")
#
# add_subdirectory(MeshUtils)
# add_subdirectory(MeshSync)
#
#
# # core
# if(BUILD_MSCORE)
#     add_subdirectory(mscore)
# endif()
#
# # Tests
# if(BUILD_TESTS)
#     add_subdirectory(Test)
# endif()
#
