CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(MeshSync)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
FIND_PACKAGE(OpenEXR QUIET)
FIND_PACKAGE(Poco COMPONENTS Net QUIET)

SET(ISPC "/usr/local/bin/ispc" CACHE PATH "Path to Intel ISPC")
MARK_AS_ADVANCED(FORCE ISPC)

OPTION(ENABLE_ISPC "Use Intel ISPC to generate SIMDified code. It can significantly boost performance." ON)
OPTION(BUILD_MAYA_CLIENT "Build MeshSyncClient for Maya" OFF)
OPTION(BUILD_BLENDER_CLIENT "Build MeshSyncClient for Blender" OFF)
OPTION(ENABLE_TESTS "Tests" OFF)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -Wno-deprecated")

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    OPTION(ENABLE_OSX_BUNDLE "Build bundle." OFF)
    SET(CMAKE_MACOSX_RPATH ON)

    IF(ENABLE_OSX_BUNDLE)
        SET(CMAKE_SKIP_RPATH ON)
    ELSE()
        SET(CMAKE_SKIP_RPATH OFF)
    ENDIF()
ENDIF()

SET(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../MeshSync/Assets")
SET(PLUGINS_DIR "${ASSETS_DIR}/UTJ/MeshSync/Plugins/x86_64")

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Poco_INCLUDE_DIRS}
)

# MeshUtilsCore
IF(ENABLE_ISPC)
    if(NOT EXISTS ${ISPC})
        # try to download ISPC
        SET(ISPC_VERSION 1.9.2)
        IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            SET(ISPC_DIR ispc-v${ISPC_VERSION}-linux)
        ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            SET(ISPC_DIR ispc-v${ISPC_VERSION}-osx)
        ENDIF()
        SET(ISPC ${CMAKE_CURRENT_SOURCE_DIR}/External/${ISPC_DIR}/ispc CACHE PATH "" FORCE)

        if(NOT EXISTS ${ISPC})
            SET(ISPC_ARCHIVE ${ISPC_DIR}.tar.gz)
            FILE(DOWNLOAD http://downloads.sourceforge.net/project/ispcmirror/v${ISPC_VERSION}/${ISPC_ARCHIVE} ${CMAKE_CURRENT_BINARY_DIR}/${ISPC_ARCHIVE} SHOW_PROGRESS)
            EXECUTE_PROCESS(
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/External
                COMMAND tar -xzvf ${CMAKE_CURRENT_BINARY_DIR}/${ISPC_ARCHIVE}
            )
        ENDIF()
    ENDIF()

    SET(MUCORE_DIR ${CMAKE_CURRENT_BINARY_DIR}/MeshUtilsCore)
    ADD_CUSTOM_TARGET(MeshUtilsCore ALL
        COMMAND ${ISPC} ${CMAKE_CURRENT_SOURCE_DIR}/MeshUtils/MeshUtilsCore.ispc -o ${MUCORE_DIR}/MeshUtilsCore${CMAKE_CXX_OUTPUT_EXTENSION} -h ${MUCORE_DIR}/MeshUtilsCore.h --pic --target=sse4-i32x4,avx1-i32x8,avx512skx-i32x16 --arch=x86-64 --opt=fast-masked-vload --opt=fast-math
    )
    SET(MUCORE_FILES
        ${MUCORE_DIR}/MeshUtilsCore.h
        ${MUCORE_DIR}/MeshUtilsCore${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore_sse4${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore_avx${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore_avx512skx${CMAKE_CXX_OUTPUT_EXTENSION}
    )

    # create dummy files to make cmake can find it
    FOREACH(F ${MUCORE_FILES})
        FILE(WRITE ${F} "")
    ENDFOREACH(F)
ENDIF()

# MeshUtils
FILE(GLOB MU_CXX_FILES MeshUtils/*.cpp MeshUtils/*.c)
FILE(GLOB MU_H_FILES MeshUtils/*.h)
ADD_LIBRARY(MeshUtils STATIC ${MU_CXX_FILES} ${MU_H_FILES} ${MUCORE_FILES})
TARGET_INCLUDE_DIRECTORIES(MeshUtils PUBLIC ./MeshUtils)
IF(ENABLE_ISPC)
    ADD_DEFINITIONS(-DmuEnableISPC)
    ADD_DEPENDENCIES(MeshUtils MeshUtilsCore)
    TARGET_INCLUDE_DIRECTORIES(MeshUtils PUBLIC ${MUCORE_DIR})
ENDIF()


# MeshSync
FILE(GLOB MS_CXX_FILES MeshSync/*.cpp)
FILE(GLOB MS_H_FILES MeshSync/*.h)
ADD_LIBRARY(MeshSync STATIC ${MS_CXX_FILES} ${MS_H_FILES})
TARGET_INCLUDE_DIRECTORIES(MeshSync PUBLIC ./MeshSync)

# MeshSyncServer
FILE(GLOB MSS_CXX_FILES MeshSyncServer/*.cpp)
FILE(GLOB MSS_H_FILES MeshSyncServer/*.h)
IF(ENABLE_OSX_BUNDLE)
    ADD_LIBRARY(MeshSyncServer MODULE ${MSS_CXX_FILES} ${MSS_H_FILES})
ELSE()
    ADD_LIBRARY(MeshSyncServer SHARED ${MSS_CXX_FILES} ${MSS_H_FILES})
ENDIF()
ADD_DEPENDENCIES(MeshSyncServer MeshUtils MeshSync)
TARGET_INCLUDE_DIRECTORIES(MeshSyncServer PUBLIC ./MeshSyncServer)
TARGET_LINK_LIBRARIES(MeshSyncServer MeshUtils MeshSync)

IF(ENABLE_OSX_BUNDLE)
    SET_TARGET_PROPERTIES(MeshSyncServer PROPERTIES BUNDLE ON)
    SET_TARGET_PROPERTIES(MeshSyncServer PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES(MeshSyncServer PROPERTIES SUFFIX ".bundle")
ENDIF()


# MeshSyncClientMaya
IF(BUILD_MAYA_CLIENT)
    FILE(GLOB MSCMAYA_CXX_FILES MeshSyncClientMaya/*.cpp)
    FILE(GLOB MSCMAYA_H_FILES MeshSyncClientMaya/*.h)
    IF(ENABLE_OSX_BUNDLE)
        ADD_LIBRARY(MeshSyncClientMaya MODULE ${MSCMAYA_CXX_FILES} ${MSCMAYA_H_FILES})
    ELSE()
        ADD_LIBRARY(MeshSyncClientMaya SHARED ${MSCMAYA_CXX_FILES} ${MSCMAYA_H_FILES})
    ENDIF()
    ADD_DEPENDENCIES(MeshSyncClientMaya MeshUtils MeshSync)
    TARGET_INCLUDE_DIRECTORIES(MeshSyncClientMaya PUBLIC ./MeshSyncClientMaya)
    
    IF(ENABLE_OSX_BUNDLE)
        SET_TARGET_PROPERTIES(MeshSyncClientMaya PROPERTIES BUNDLE ON)
        SET_TARGET_PROPERTIES(MeshSyncClientMaya PROPERTIES PREFIX "")
        SET_TARGET_PROPERTIES(MeshSyncClientMaya PROPERTIES SUFFIX ".bundle")
    ENDIF()
ENDIF()

# MeshSyncClientBlender
IF(BUILD_BLENDER_CLIENT)
    FILE(GLOB MSCBLENDER_CXX_FILES MeshSyncClientBlender/*.cpp)
    FILE(GLOB MSCBLENDER_H_FILES MeshSyncClientBlender/*.h)
    IF(ENABLE_OSX_BUNDLE)
        ADD_LIBRARY(MeshSyncClientBlender MODULE ${MSCBLENDER_CXX_FILES} ${MSCBLENDER_H_FILES})
    ELSE()
        ADD_LIBRARY(MeshSyncClientBlender SHARED ${MSCBLENDER_CXX_FILES} ${MSCBLENDER_H_FILES})
    ENDIF()
    ADD_DEPENDENCIES(MeshSyncClientBlender MeshUtils MeshSync)
    TARGET_INCLUDE_DIRECTORIES(MeshSyncClientBlender PUBLIC ./MeshSyncClientBlender)
    
    IF(ENABLE_OSX_BUNDLE)
        SET_TARGET_PROPERTIES(MeshSyncClientBlender PROPERTIES BUNDLE ON)
        SET_TARGET_PROPERTIES(MeshSyncClientBlender PROPERTIES PREFIX "")
        SET_TARGET_PROPERTIES(MeshSyncClientBlender PROPERTIES SUFFIX ".bundle")
    ENDIF()
ENDIF()


IF(ENABLE_OSX_BUNDLE)
    # deploy
    ADD_CUSTOM_TARGET(deploy ALL
        COMMAND rm -rf ${PLUGINS_DIR}/MeshSyncServer.bundle
        COMMAND cp -r MeshSyncServer.bundle ${PLUGINS_DIR}
        DEPENDS MeshSyncServer
    )
ELSE()
    # tests
    IF(ENABLE_TESTS)
        FILE(GLOB Tests_CXX_FILES Test/*.cpp)
        FILE(GLOB Tests_H_FILES Test/*.h)

        ADD_EXECUTABLE(Tests ${Tests_CXX_FILES} ${Tests_H_FILES})
        ADD_DEPENDENCIES(Tests MeshUtils)
        TARGET_LINK_LIBRARIES(Tests MeshUtils)
    ENDIF()

    # deploy
    ADD_CUSTOM_TARGET(deploy ALL
        COMMAND rm -rf ${PLUGINS_DIR}/$<TARGET_FILE_NAME:MeshSyncServer>
        COMMAND cp $<TARGET_FILE:MeshSyncServer> ${PLUGINS_DIR}
        DEPENDS MeshSyncServer
    )
ENDIF()
