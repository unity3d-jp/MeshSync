CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

project(MeshSync)

file(GLOB_RECURSE sources *.cpp *.h )
set(MeshSync_dir "${CMAKE_CURRENT_SOURCE_DIR}")


# setup as a library
add_library(MeshSync STATIC ${sources} ${MUISPC_OUTPUTS})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

#Include, pch
target_precompile_headers(MeshSync PRIVATE "${MeshSync_dir}/pch.h")

add_dependencies(MeshSync MeshUtils)

target_include_directories(MeshSync 
    PUBLIC 
        ${ZSTD_INCLUDE_DIR}    
        "${MeshSync_dir}/Include"
    PRIVATE
        "${MeshSync_dir}"
        "${Poco_INCLUDE_DIRS}"
)

target_link_libraries(MeshSync
    PUBLIC
        MeshUtils
    PRIVATE
        ${ZSTD_LIBRARY}    
        ${Poco_LIBRARIES}        
)
