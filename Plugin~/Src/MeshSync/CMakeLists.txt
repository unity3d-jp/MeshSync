CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

project(MeshSync)

file(GLOB_RECURSE sources *.cpp *.h )
set(MeshSync_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(SRC_ROOT   "${CMAKE_CURRENT_SOURCE_DIR}/..")


# setup as a library
add_library(MeshSync STATIC ${sources} ${MUISPC_OUTPUTS})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

#Include, pch
target_precompile_headers(MeshSync PRIVATE "${MeshSync_dir}/pch.h")
target_include_directories(MeshSync PRIVATE
    ${SRC_ROOT}
    ${MeshSync_dir}    
    ${Poco_INCLUDE_DIRS}
)

if(ENABLE_ZSTD)
    target_include_directories(MeshSync PRIVATE ${ZSTD_INCLUDE_DIR})
endif()
