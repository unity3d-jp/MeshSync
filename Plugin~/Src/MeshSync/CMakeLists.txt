CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

project(MeshSync)

file(GLOB sources *.cpp *.h Utils/*.cpp Utils/*.h SceneCache/*.cpp SceneCache/*.h SceneGraph/*.cpp SceneGraph/*.h)
set(MeshSync_dir "${CMAKE_CURRENT_SOURCE_DIR}")

# setup as a library
add_library(MeshSync STATIC ${sources} ${MUISPC_OUTPUTS})

#Include, pch
target_precompile_headers(MeshSync PRIVATE "${MeshSync_dir}/pch.h")
target_include_directories(MeshSync PRIVATE
    ${MeshSync_dir}    
    ${Poco_INCLUDE_DIRS}
)
