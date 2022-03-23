CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

project(mscore)
file(GLOB_RECURSE sources *.cpp *.h)
set(mscore_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

if(WIN32) 
    set(plugins_folder "x86_64")
else()
    set(plugins_folder "${CMAKE_SYSTEM_PROCESSOR}")
endif()

set(plugins_dir "${SRC_ROOT}/../../Runtime/Plugins/${plugins_folder}")

add_plugin(mscore SOURCES ${sources} PLUGINS_DIR ${plugins_dir})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

#Include, pch
target_include_directories(mscore PRIVATE 
    ${mscore_dir}
)
target_include_directories(mscore PRIVATE )
target_precompile_headers(mscore PRIVATE "${mscore_dir}/pch.h")

# Library dependencies
target_link_libraries( mscore 
    PUBLIC 
        MeshSync 
)

