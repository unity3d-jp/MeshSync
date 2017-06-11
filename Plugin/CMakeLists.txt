CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(MeshSync)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
FIND_PACKAGE(OpenEXR QUIET)
FIND_PACKAGE(Poco COMPONENTS Net QUIET)

SET(ISPC "/usr/local/bin/ispc" CACHE PATH "Path to Intel ISPC")
MARK_AS_ADVANCED(FORCE ISPC)

OPTION(ENABLE_ISPC "Use Intel ISPC to generate SIMDified code. It can significantly boost performance." ON)
OPTION(ENABLE_MAYA "Build MAYA Plugin" OFF)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -Wno-deprecated -DTF_NO_GNU_EXT")

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    OPTION(USDI_ENABLE_OSX_BUNDLE "Build bundle." OFF)
    SET(CMAKE_MACOSX_RPATH ON)

    IF(USDI_ENABLE_OSX_BUNDLE)
        SET(CMAKE_SKIP_RPATH ON)
    ELSE()
        SET(CMAKE_SKIP_RPATH OFF)
    ENDIF()
ENDIF()

SET(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../MeshSync/Assets")
SET(PLUGINS_DIR "${ASSETS_DIR}/UTJ/Plugins/x86_64")

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Poco_INCLUDE_DIRS}
)

# MeshUtilsCore
IF(ENABLE_ISPC)
    if(NOT EXISTS ${ISPC})
        # try to download ISPC
        SET(ISPC_VERSION 1.9.1)
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
        COMMAND ${ISPC} ${CMAKE_CURRENT_SOURCE_DIR}/MeshUtils/MeshUtilsCore.ispc -o ${MUCORE_DIR}/MeshUtilsCore${CMAKE_CXX_OUTPUT_EXTENSION} -h ${MUCORE_DIR}/MeshUtilsCore.h --pic --target=sse2,sse4,avx --arch=x86-64 --opt=fast-masked-vload --opt=fast-math
        COMMAND ${ISPC} ${CMAKE_CURRENT_SOURCE_DIR}/MeshUtils/MeshUtilsCore2.ispc -o ${MUCORE_DIR}/MeshUtilsCore2${CMAKE_CXX_OUTPUT_EXTENSION} -h ${MUCORE_DIR}/MeshUtilsCore2.h --pic --target=sse2,sse4,avx --arch=x86-64 --opt=fast-masked-vload --opt=fast-math
    )
    SET(MUCORE_FILES
        ${MUCORE_DIR}/MeshUtilsCore.h
        ${MUCORE_DIR}/MeshUtilsCore${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore_sse2${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore_sse4${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore_avx${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore2.h
        ${MUCORE_DIR}/MeshUtilsCore2${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore2_sse2${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore2_sse4${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore2_avx${CMAKE_CXX_OUTPUT_EXTENSION}
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


# NormalPainter
FILE(GLOB NP_CXX_FILES NormalPainter/*.cpp)
FILE(GLOB NP_H_FILES NormalPainter/*.h)
IF(USDI_ENABLE_OSX_BUNDLE)
    ADD_LIBRARY(NormalPainter MODULE ${NP_CXX_FILES} ${NP_H_FILES})
ELSE()
    ADD_LIBRARY(NormalPainter SHARED ${NP_CXX_FILES} ${NP_H_FILES})
ENDIF()
ADD_DEPENDENCIES(NormalPainter MeshUtils)
TARGET_INCLUDE_DIRECTORIES(NormalPainter PUBLIC ./NormalPainter)


# MeshSync
FILE(GLOB MS_CXX_FILES MeshSync/*.cpp)
FILE(GLOB MS_H_FILES MeshSync/*.h)
ADD_LIBRARY(MeshSync STATIC ${MS_CXX_FILES} ${MS_H_FILES})
TARGET_INCLUDE_DIRECTORIES(MeshSync PUBLIC ./MeshSync)


# MeshSyncServer
FILE(GLOB MSS_CXX_FILES MeshSyncServer/*.cpp)
FILE(GLOB MSS_H_FILES MeshSyncServer/*.h)
IF(USDI_ENABLE_OSX_BUNDLE)
    ADD_LIBRARY(MeshSyncServer MODULE ${MSS_CXX_FILES} ${MSS_H_FILES})
ELSE()
    ADD_LIBRARY(MeshSyncServer SHARED ${MSS_CXX_FILES} ${MSS_H_FILES})
ENDIF()
ADD_DEPENDENCIES(MeshSyncServer MeshUtils MeshSync)
TARGET_INCLUDE_DIRECTORIES(MeshSyncServer PUBLIC ./MeshSyncServer)


# MeshSyncClientMaya
IF(ENABLE_MAYA)
    FILE(GLOB MSCMAYA_CXX_FILES MeshSyncClientMaya/*.cpp)
    FILE(GLOB MSCMAYA_H_FILES MeshSyncClientMaya/*.h)
    IF(USDI_ENABLE_OSX_BUNDLE)
        ADD_LIBRARY(MeshSyncClientMaya MODULE ${MSCMAYA_CXX_FILES} ${MSCMAYA_H_FILES})
    ELSE()
        ADD_LIBRARY(MeshSyncClientMaya SHARED ${MSCMAYA_CXX_FILES} ${MSCMAYA_H_FILES})
    ENDIF()
    ADD_DEPENDENCIES(MeshSyncClientMaya MeshUtils MeshSync)
    TARGET_INCLUDE_DIRECTORIES(MeshSyncClientMaya PUBLIC ./MeshSyncClientMaya)
ENDIF()

